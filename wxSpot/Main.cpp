//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

#include "Main.h"

#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/time.h> 
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/log.h>

#include "SongListCtrl.h"

#include "ProgressIndicator.h"

#include "SoundManager.h"
#include "SpotifyManager.h"
#include "LoginDialogue.h"
#include "SettingsDialogue.h"

#include "glyphicons-174-play.h"
#include "glyphicons-175-pause.h"
#include "glyphicons-171-step-backward.h"
#include "glyphicons-179-step-forward.h"


IMPLEMENT_APP(Main)


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_COMMAND(wxID_ANY, SPOTIFY_WAKE_UP_EVENT, MainFrame::OnSpotifyWakeUpEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_STARTED_PLAYING_EVENT, MainFrame::OnSpotifyStartedPlayingEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_STOPPED_PLAYING_EVENT, MainFrame::OnSpotifyStoppedPlayingEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_END_OF_TRACK_EVENT, MainFrame::OnSpotifyEndOfTrackEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_LOADED_CONTAINER_EVENT, MainFrame::OnSpotifyLoadedContainerEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_PLAYLIST_ADDED_EVENT, MainFrame::OnSpotifyPlaylistAddedEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_PLAYLIST_RENAMED_EVENT, MainFrame::OnSpotifyPlaylistRenamedEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_PLAYLIST_STATE_CHANGED_EVENT, MainFrame::OnSpotifyPlaylistStateChangedEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_SEARCH_RESULTS_EVENT, MainFrame::OnSpotifySearchResultsEvent)
EVT_COMMAND(wxID_ANY, SPOTIFY_LOGGED_IN_EVENT, MainFrame::OnSpotifyLoggedInEvent)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_Settings, MainFrame::OnSettings)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_TIMER(wxID_ANY, MainFrame::OnTimerEvent)

END_EVENT_TABLE()


wxConfig *MainFrame::config = new wxConfig("wxSpot");

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
	: wxFrame((wxFrame *) nullptr, -1, title, pos, size),
	timerStatusUpdate(this, wxID_ANY),
	activeSongIndex(0),
	activePlaylist(nullptr)

{

	wxMenu *menuFile = new wxMenu();
	menuFile->Append(wxID_EXIT);

	wxMenu *optionsFile = new wxMenu();
	optionsFile->Append(ID_Settings, "&Settings...\tCtrl-S");

	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "&File");
	menuBar->Append(optionsFile, "&Settings");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	panel = new wxPanel(this, wxID_ANY);

	horzBox = new wxBoxSizer(wxHORIZONTAL);
	vertBox = new wxBoxSizer(wxVERTICAL);
	bottomHorzBox = new wxBoxSizer(wxHORIZONTAL);

	spotifyManager = new SpotifyManager(this);

	songList = new SongListCtrl(panel, spotifyManager);
	songList->SetClientData(nullptr);

	playlistTree = new wxTreeCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	playlistTree->AddRoot("Playlists");

	playlistTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &event) {
		wxTreeItemId item = event.GetItem();
		std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

		for (unsigned int i = 0; i < playlists->size(); i++) {
			if (playlists->at(i)->getTreeItemId() == item) {
				songList->setPlaylist(playlists->at(i));
				break;
			}
		}

	});


	songList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [=](wxListEvent &event) {
		int index = event.GetIndex();

		Track *track = songList->getTrack(index);
		activePlaylist = (Playlist *)songList->GetClientData();
		spotifyManager->playTrack(track);
		activeSongIndex = index;
	});

	songList->Bind(wxEVT_LIST_COL_CLICK, [=](wxListEvent &event) {
		int col = event.GetColumn();
		if (col == 1) {
			wxMessageBox("Column 1");
		}
	});

	songList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, [=](wxListEvent &event) {
		//wxMenu menu("Play");

		Track *track = songList->getTrack(event.GetIndex());
		activePlaylist = (Playlist *)songList->GetClientData();

		wxMenu *playlists = new wxMenu("Playlists");

		std::vector<SpotifyPlaylist *> *spotifyPlaylists = spotifyManager->getPlaylists();

		for (size_t i = 0; i < spotifyPlaylists->size(); i++) {
			playlists->Append(ID_Menu_Last_Dont_Use + i, spotifyPlaylists->at(i)->getTitle());
		}


		wxMenu popup(track->getTitle());

		popup.AppendSubMenu(playlists, "Add to playlist");
		popup.Append(ID_Menu_Delete_Track, "Delete track");
		popup.Append(ID_Menu_Copy_TrackName, "Copy Track Name");
		popup.Append(ID_Menu_Copy_URI, "Copy Spotify URI");
		popup.Append(ID_Menu_Copy_URL, "Copy Spotify URL");

		int selection = songList->GetPopupMenuSelectionFromUser(popup, event.GetPoint());


		switch (selection) {
		case wxID_NONE:
			break;
		case ID_Menu_Delete_Track:
			activePlaylist->removeTrack(event.GetIndex());
			songList->SetItemCount(activePlaylist->getTracks()->size());
			songList->RefreshItems(0, activePlaylist->getTracks()->size() - 1);
			break;
		case ID_Menu_Copy_TrackName:
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData(new wxTextDataObject(track->getTitle() + " - " + track->getArtist()));
				wxTheClipboard->Close();
			}
			break;
		case ID_Menu_Copy_URI:
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData(new wxTextDataObject(track->getLink()));
				wxTheClipboard->Close();
			}
			break;
		case ID_Menu_Copy_URL:

			// typical link spotify:track:XXXXXXXXXX
			wxStringTokenizer tokenizer(track->getLink(), ":");
			
			if (!tokenizer.HasMoreTokens()) break;

			tokenizer.GetNextToken(); // skip spotify
			wxString type = tokenizer.GetNextToken();
			wxString id = tokenizer.GetNextToken();

			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData(new wxTextDataObject("https://open.spotify.com/" + type + "/" + id));
				wxTheClipboard->Close();
			}
			break;
		}
		if (selection > ID_Menu_Last_Dont_Use) {
			selection -= ID_Menu_Last_Dont_Use;

			wxLogDebug("Trying to add track: %s to Playlist: %s", track->getTitle(), spotifyPlaylists->at(selection)->getTitle());
			spotifyManager->addTrackToPlaylist(track, spotifyPlaylists->at(selection));
			spotifyPlaylists->at(selection)->addTrack(track->getSpTrack());

		}
	});


	searchTextCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	searchTextCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent &event) {
		spotifyManager->search(searchTextCtrl->GetValue());
	});

	searchPlaylistBox = new wxBoxSizer(wxVERTICAL);

	searchPlaylistBox->Add(searchTextCtrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
	searchPlaylistBox->Add(playlistTree, 1, wxGROW);
	horzBox->Add(searchPlaylistBox, 2, wxGROW);

	songList->Layout();

	horzBox->Add(songList, 3, wxGROW);

	vertBox->Add(horzBox, 2, wxGROW);

	wxImage::AddHandler(new wxPNGHandler);
	//wxImage::AddHandler(new wxJPEGHandler);

	//playImage = wxBitmap(wxImage("glyphicons-174-play.png", wxBITMAP_TYPE_PNG));
	//playImage = wxBitmap(pixmap_play, wxBITMAP_TYPE_XPM);
	playImage = wxBITMAP_PNG_FROM_DATA(glyphicons_174_play);
	//pauseImage = wxBitmap(wxImage("glyphicons-175-pause.png", wxBITMAP_TYPE_PNG));
	//pauseImage = wxBitmap(pixmap_pause, wxBITMAP_TYPE_XPM);
	pauseImage = wxBITMAP_PNG_FROM_DATA(glyphicons_175_pause);
	//nextImage = wxBitmap(wxImage("glyphicons-179-step-forward.png", wxBITMAP_TYPE_PNG));
	//nextImage = wxBitmap(pixmap_next, wxBITMAP_TYPE_XPM);
	nextImage = wxBITMAP_PNG_FROM_DATA(glyphicons_179_step_forward);
	//prevImage = wxBitmap(wxImage("glyphicons-171-step-backward.png", wxBITMAP_TYPE_PNG));
	//prevImage = wxBitmap(pixmap_prev, wxBITMAP_TYPE_XPM);
	prevImage = wxBITMAP_PNG_FROM_DATA(glyphicons_171_step_backward);

	buttonPlayPause = new wxBitmapButton(panel, wxID_ANY, playImage);
	buttonPlayPause->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		playPause();
	});

	buttonNext = new wxBitmapButton(panel, wxID_ANY, nextImage);
	buttonNext->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		next();
	});

	buttonPrev = new wxBitmapButton(panel, wxID_ANY, prevImage);
	buttonPrev->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		prev();
	});

	bottomHorzBox->Add(buttonPrev, 0, wxALL, 2);
	bottomHorzBox->Add(buttonPlayPause, 0, wxALL, 2);
	bottomHorzBox->Add(buttonNext, 0, wxALL, 2);

	progressIndicator = new ProgressIndicator(panel);
	progressIndicator->Bind(PI_SCROLL_CHANGED, [=](wxCommandEvent &event) {
		spotifyManager->seek(progressIndicator->GetValue() * spotifyManager->getSongLength());

		// force a process events for faster seeks :)
		spotifyManager->processEvents();
	});


	textCurrentProgressTime = new wxStaticText(panel, wxID_ANY, "0:00");
	textTotalTime = new wxStaticText(panel, wxID_ANY, "0:00");

	
	checkBoxShuffle = new wxCheckBox(panel, wxID_ANY, _("Shuffle"));

	bottomHorzBox->Add(textCurrentProgressTime, 0, wxALL, 2);
	bottomHorzBox->Add(progressIndicator, wxSizerFlags(1).Expand().Border(wxALL, 2));
	bottomHorzBox->Add(textTotalTime, 0, wxALL, 2);
	bottomHorzBox->Add(checkBoxShuffle, 0, wxALL, 2);
	bottomHorzBox->Layout();


	vertBox->Add(bottomHorzBox, 0, wxEXPAND);

	vertBox->Layout();

	panel->SetSizer(vertBox);


	soundManager = new SoundManager(this);

	spotifyManager->setEventHandler(GetEventHandler());

	wxString spotifyCachePath;
	int deviceIndex;

	if (config->Read("Path", &spotifyCachePath) && wxDirExists(spotifyCachePath) &&
		config->Read("DeviceIndex", &deviceIndex) && soundManager->deviceSupported(deviceIndex)) {
		spotifyManager->init(spotifyCachePath);
		soundManager->init(deviceIndex);
	}
	else {
		SettingsDialogue *dialogue = new SettingsDialogue(soundManager);

		if (dialogue->ShowModal() == wxID_OK) {
			config->Read("Path", &spotifyCachePath);
			config->Read("DeviceIndex", &deviceIndex);
			spotifyManager->init(spotifyCachePath);
			soundManager->init(deviceIndex);
		}
		delete dialogue;

	}
	



	showLoginDialog();

}

MainFrame::~MainFrame()
{
	soundManager->stop();
	soundManager->end();

	spotifyManager->end();

	delete spotifyManager;
	delete soundManager;
	delete config;

}

void MainFrame::OnExit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnSettings(wxCommandEvent &event)
{
	std::unique_ptr<SettingsDialogue> dialogue = std::make_unique<SettingsDialogue>(soundManager);
	if (dialogue->ShowModal() == wxID_OK) {

	}
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox("wxSpot 0.8 by Viktor Müntzing", "About", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnSpotifyWakeUpEvent(wxCommandEvent &event)
{
	spotifyManager->processEvents();
}

void MainFrame::OnSpotifyStartedPlayingEvent(wxCommandEvent &event)
{
	buttonPlayPause->SetImageLabel(pauseImage);
	int duration = spotifyManager->getSongLength();

	textTotalTime->SetLabel(wxString::Format("%d:%02d", duration / 60000, (duration / 1000) % 60));
	timerStatusUpdate.Start(1000, false);

	soundManager->play();

	wxTopLevelWindow::SetTitle(spotifyManager->getSongName());
}

void MainFrame::OnSpotifyStoppedPlayingEvent(wxCommandEvent &event)
{
	buttonPlayPause->SetImageLabel(playImage);

	timerStatusUpdate.Stop();
	soundManager->stop();
}

void MainFrame::OnSpotifyEndOfTrackEvent(wxCommandEvent &event)
{
	wxLogDebug("End of track event");
	//next();
}

void MainFrame::OnSpotifyLoadedContainerEvent(wxCommandEvent &event)
{
	std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

	playlistTree->Freeze();
	wxTreeItemId parent = playlistTree->GetRootItem();
	playlistTree->DeleteChildren(parent);

	wxTreeItemId shared = playlistTree->AppendItem(parent, "Shared");
	wxTreeItemId own = playlistTree->AppendItem(parent, "Own");
	for (unsigned int i = 0; i < playlists->size(); i++) {
		wxTreeItemId item;
		if (playlists->at(i)->isShared()) {
			item = playlistTree->AppendItem(shared, playlists->at(i)->getTitle());
		}
		else {
			item = playlistTree->AppendItem(own, playlists->at(i)->getTitle());
		}
		
		playlists->at(i)->setTreeItemId(item);
	}

	playlistTree->ExpandAll();
	playlistTree->Thaw();
}

void MainFrame::OnSpotifyPlaylistAddedEvent(wxCommandEvent &event)
{
	// Maybe add new if we don't have it already
	wxLogDebug("Playlist added event");
}

void MainFrame::OnSpotifyPlaylistRenamedEvent(wxCommandEvent &event)
{
	int index = event.GetInt();

	std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

	wxTreeItemId item = playlists->at(index)->getTreeItemId();

	playlistTree->SetItemText(item, playlists->at(index)->getTitle());
}

void MainFrame::OnSpotifyPlaylistStateChangedEvent(wxCommandEvent &event)
{
	int index = event.GetInt();

	std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

	if (playlists->at(index)->isShared()) {
		//std::cout << " is shared ";
	}
	/*for (int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i).isShared()) {
			std::cout << " is shared ";
		}
	}*/
	wxLogDebug("Playlist changed: %d", index);

}

void MainFrame::OnSpotifySearchResultsEvent(wxCommandEvent &event)
{
	
	Playlist *searchResults = spotifyManager->getSearchResults();
	songList->setPlaylist(searchResults);
}

void MainFrame::OnSpotifyLoggedInEvent(wxCommandEvent &event)
{
	if (loginDialogue != nullptr) {
		loginDialogue->GetEventHandler()->QueueEvent(event.Clone());
	}
}

void MainFrame::OnTimerEvent(wxTimerEvent &event)
{
	unsigned int currTime = audioBuffer.getPlayTime();

	textCurrentProgressTime->SetLabel(wxString::Format("%d:%02d", currTime / 60000, (currTime / 1000) % 60));

	unsigned int duration = spotifyManager->getSongLength();

	progressIndicator->SetValue(((double)currTime / (double)duration));
}

void MainFrame::showLoginDialog()
{
	loginDialogue = new LoginDialogue();

	loginDialogue->setSpotifyManager(spotifyManager);


	if (loginDialogue->ShowModal() == wxID_OK) {
		delete loginDialogue;
		loginDialogue = nullptr;
	}

}

void MainFrame::playPause()
{
	spotifyManager->playPause();
}

bool MainFrame::next()
{
	Playlist *activePlaylist = (Playlist*)songList->GetClientData();
	if (activePlaylist != nullptr) {
		if (checkBoxShuffle->IsChecked()) {
			std::uniform_int_distribution<unsigned int> distribution(0, activePlaylist->getTracks()->size() - 1);
			unsigned int newIndex = distribution(generator);

			if (newIndex == activeSongIndex) {
				activeSongIndex = (activeSongIndex + 1) % activePlaylist->getTracks()->size();
			}
			else {
				activeSongIndex = newIndex;
			}
		}
		else if (activeSongIndex == activePlaylist->getTracks()->size() - 1) {
			timerStatusUpdate.Stop();
			soundManager->stop();
			return false;
		}
		else {
			activeSongIndex++;
		}
		spotifyManager->playTrack(activePlaylist->getTracks()->at(activeSongIndex));
		spotifyManager->processEvents();

	}
	return true;

}

bool MainFrame::prev()
{
	Playlist *activePlaylist = (Playlist*)songList->GetClientData();
	if (activePlaylist != nullptr) {
		if (0 == activeSongIndex) {
			timerStatusUpdate.Stop();
			soundManager->stop();
			return false;
		}
		else {
			activeSongIndex--;
			spotifyManager->playTrack(activePlaylist->getTracks()->at(activeSongIndex));
			spotifyManager->processEvents();

		}
	}

	return true;
}


void MainFrame::bufferDone()
{
	next();
}

Main::Main()
{
}


Main::~Main()
{
}

bool Main::OnInit()
{
	if (!wxApp::OnInit()) {
		return false;
	}

	//_CrtSetBreakAlloc(4223);

	mainFrame = new MainFrame(_("wxSpot"), wxDefaultPosition, wxSize(800, 600));

	mainFrame->Show();

	SetTopWindow(mainFrame);

	return true;
}
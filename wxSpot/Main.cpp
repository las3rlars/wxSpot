#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "Main.h"

#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/time.h> 

#include <random>

#include "SoundManager.h"
#include "SpotifyManager.h"
#include "LoginDialogue.h"
#include "SettingsDialogue.h"

#include "play.xpm"
#include "pause.xpm"
#include "step-forward.xpm"
#include "step-backward.xpm"


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
activeSongIndex(0)

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


	songList = new wxListView(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	songList->AppendColumn(wxString("Track"), wxLIST_FORMAT_LEFT, 280);
	songList->AppendColumn(wxString("Artist"), wxLIST_FORMAT_LEFT, 270);
	songList->SetClientData(nullptr);

	playlistTree = new wxTreeCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	playlistTree->AddRoot("Playlists");

	playlistTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &event) {
		wxTreeItemId item = event.GetItem();
		std::vector<Playlist*> *playlists = spotifyManager->getPlaylists();
		songList->Freeze();
		songList->DeleteAllItems();
		songList->SetClientData(nullptr);

		for (unsigned int i = 0; i < playlists->size(); i++) {
			if (playlists->at(i)->getTreeItemId() == item) {
				songList->SetClientData(playlists->at(i));
				std::vector<Track*> *tracks = playlists->at(i)->getTracks();
				for (unsigned int j = 0; j < tracks->size(); j++) {
					long item = songList->InsertItem(songList->GetItemCount(), tracks->at(j)->getTitle());
					songList->SetItem(item, 1, tracks->at(j)->getArtist());
					songList->SetItemPtrData(item, (wxUIntPtr)tracks->at(j));
				}
				break;
			}
		}

		songList->Thaw();
	});


	songList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [=](wxListEvent &event) {
		int index = event.GetIndex();

		wxListItem item = event.GetItem();
		Track *track = (Track *)songList->GetItemData(item);
		spotifyManager->playTrack(track);
		activeSongIndex = index;

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

	//wxImage::AddHandler(new wxPNGHandler);
	//wxImage::AddHandler(new wxJPEGHandler);

	//playImage = wxBitmap(wxImage("glyphicons-174-play.png", wxBITMAP_TYPE_PNG));
	playImage = wxBitmap(pixmap_play, wxBITMAP_TYPE_XPM);
	//pauseImage = wxBitmap(wxImage("glyphicons-175-pause.png", wxBITMAP_TYPE_PNG));
	pauseImage = wxBitmap(pixmap_pause, wxBITMAP_TYPE_XPM);
	//nextImage = wxBitmap(wxImage("glyphicons-179-step-forward.png", wxBITMAP_TYPE_PNG));
	nextImage = wxBitmap(pixmap_next, wxBITMAP_TYPE_XPM);
	//prevImage = wxBitmap(wxImage("glyphicons-171-step-backward.png", wxBITMAP_TYPE_PNG));
	prevImage = wxBitmap(pixmap_prev, wxBITMAP_TYPE_XPM);

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

	progressSlider = new wxSlider(panel, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS);
	progressSlider->Bind(wxEVT_SCROLL_CHANGED, [=](wxScrollEvent &event) {
		spotifyManager->seek((progressSlider->GetValue() / 100.0) * spotifyManager->getSongLength());
		//wxMessageBox(wxString::Format("trying to scroll to: %f %d %d", (progressSlider->GetValue() / 100.0) * spotifyManager->getSongLength(), progressSlider->GetValue(), spotifyManager->getSongLength()));
	});

	textCurrentProgressTime = new wxStaticText(panel, wxID_ANY, "0:00");
	textTotalTime = new wxStaticText(panel, wxID_ANY, "0:00");

	
	checkBoxShuffle = new wxCheckBox(panel, wxID_ANY, _("Shuffle"));

	bottomHorzBox->Add(textCurrentProgressTime, 0, wxALL, 2);
	bottomHorzBox->Add(progressSlider, wxSizerFlags(1).Expand().Border(wxALL, 2));
	bottomHorzBox->Add(textTotalTime, 0, wxALL, 2);
	bottomHorzBox->Add(checkBoxShuffle, 0, wxALL, 2);
	bottomHorzBox->Layout();


	vertBox->Add(bottomHorzBox, 0, wxEXPAND);

	vertBox->Layout();

	panel->SetSizer(vertBox);

	//SetSizer(vertBox);


	soundManager = new SoundManager(this);

	soundManager->init();

	spotifyManager = new SpotifyManager(this);
	spotifyManager->setEventHandler(GetEventHandler());

	wxString spotifyCachePath;

	if (config->Read("Path", &spotifyCachePath) && wxDirExists(spotifyCachePath)) {
		spotifyManager->init(spotifyCachePath);
	}
	else {
		SettingsDialogue *dialogue = new SettingsDialogue();

		if (dialogue->ShowModal() == wxID_OK) {
			config->Read("Path", &spotifyCachePath);
			spotifyManager->init(spotifyCachePath);
		}
		delete dialogue;

	}
	



	showLoginDialog();

}

MainFrame::~MainFrame()
{
	spotifyManager->end();

	soundManager->stop();
	soundManager->end();

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
	SettingsDialogue *dialogue = new SettingsDialogue();

	if (dialogue->ShowModal() == wxID_OK) {

	}
	delete dialogue;
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox("wxSpot 0.5 by Viktor M�ntzing", "About", wxOK | wxICON_INFORMATION);
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

	//wxTopLevelWindow::SetTitle("Not currently playing");
}

void MainFrame::OnSpotifyEndOfTrackEvent(wxCommandEvent &event)
{
	next();
}

void MainFrame::OnSpotifyLoadedContainerEvent(wxCommandEvent &event)
{
	std::vector<Playlist*> *playlists = spotifyManager->getPlaylists();

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
}

void MainFrame::OnSpotifyPlaylistRenamedEvent(wxCommandEvent &event)
{
	int index = event.GetInt();

	std::vector<Playlist*> *playlists = spotifyManager->getPlaylists();

	wxTreeItemId item = playlists->at(index)->getTreeItemId();

	playlistTree->SetItemText(item, playlists->at(index)->getTitle());
}

void MainFrame::OnSpotifyPlaylistStateChangedEvent(wxCommandEvent &event)
{
	int index = event.GetInt();

	std::vector<Playlist*> *playlists = spotifyManager->getPlaylists();

	if (playlists->at(index)->isShared()) {
		std::cout << " is shared ";
	}
	/*for (int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i).isShared()) {
			std::cout << " is shared ";
		}
	}*/

}

void MainFrame::OnSpotifySearchResultsEvent(wxCommandEvent &event)
{
	
	std::vector<Track*> *tracks = spotifyManager->getSearchResults();

	songList->Freeze();
	songList->DeleteAllItems();

	for (unsigned int j = 0; j < tracks->size(); j++) {
		long item = songList->InsertItem(songList->GetItemCount(), tracks->at(j)->getTitle());
		songList->SetItem(item, 1, tracks->at(j)->getArtist());
		songList->SetItemPtrData(item, (wxUIntPtr)tracks->at(j));
	}
	songList->Thaw();

}

void MainFrame::OnSpotifyLoggedInEvent(wxCommandEvent &event)
{
	if (loginDialogue != nullptr) {
		loginDialogue->GetEventHandler()->QueueEvent(event.Clone());
		//loginDialogue->QueueEvent(NULL);
	}
}

void MainFrame::OnTimerEvent(wxTimerEvent &event)
{
	//unsigned int timeDiff = wxGetUTCTimeMillis().GetValue() - playStartTime.GetValue();
	unsigned int currTime = audioBuffer.getPlayTime();

	textCurrentProgressTime->SetLabel(wxString::Format("%d:%02d", currTime / 60000, (currTime / 1000) % 60));

	unsigned int duration = spotifyManager->getSongLength();

	progressSlider->SetValue(((double)currTime / (double)duration) * 100);
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
			std::default_random_engine generator;
			std::uniform_int_distribution<unsigned int> distribution(0, activePlaylist->getTracks()->size() - 1);
			int newIndex = distribution(generator);

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

		}
	}

	return true;

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

	//_CrtSetBreakAlloc(32784);

	mainFrame = new MainFrame(_("wxSpot"), wxDefaultPosition, wxSize(800, 600));

	mainFrame->Show();

	SetTopWindow(mainFrame);

	return true;
}
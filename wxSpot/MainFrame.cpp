/*#define _CRTDBG_MAP_ALLOC*/
#include <crtdbg.h>

#include "MainFrame.h"

#include <memory>

#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/time.h> 
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/log.h>
#include <wx/dnd.h>
#include <wx/dynlib.h>

#include "SongListCtrl.h"
#include "DnDTrackDataObject.h"
#include "DnDTrackDropTarget.h"
#include "ProgressIndicator.h"
#include "Spectrum.h"

#include "SoundManager.h"
#include "SpotifyManager.h"
#include "LoginDialog.h"
#include "SettingsDialog.h"
#include "Plugin.h"

#include "glyphicons-174-play.h"
#include "glyphicons-174-play-small.h"
#include "glyphicons-175-pause.h"
#include "glyphicons-171-step-backward.h"
#include "glyphicons-179-step-forward.h"


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
EVT_COMMAND(wxID_ANY, SPOTIFY_META_UPDATED_EVENT, MainFrame::OnSpotifyMetaUpdatedEvent)

EVT_COMMAND(wxID_ANY, SPOTIFY_PLAY_NEXT_EVENT, MainFrame::OnSpotifyPlayNextEvent)

EVT_COMMAND(wxID_ANY, PLUGIN_CLOSE_EVENT, MainFrame::OnPluginCloseEvent)
EVT_COMMAND(wxID_ANY, PLUGIN_LOADED_EVENT, MainFrame::OnPluginLoadedEvent)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_Logout, MainFrame::OnLogout)
EVT_MENU(ID_Settings, MainFrame::OnSettings)
EVT_MENU(ID_ShowSpectrum, MainFrame::OnShowSpectrum)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_TIMER(wxID_ANY, MainFrame::OnTimerEvent)

EVT_HOTKEY(ID_HotKey_PlayPause, MainFrame::OnHotKeyPlayPause)
EVT_HOTKEY(ID_HotKey_Prev, MainFrame::OnHotKeyPrev)
EVT_HOTKEY(ID_HotKey_Next, MainFrame::OnHotKeyNext)
EVT_HOTKEY(ID_HotKey_ShowCurrentTrack, MainFrame::OnHotKeyShowCurrentTrack)

END_EVENT_TABLE()

//DEFINE_EVENT_TYPE(PLUGIN_CLOSE_EVENT)

wxDEFINE_EVENT(PLUGIN_CLOSE_EVENT, wxCommandEvent);
wxDEFINE_EVENT(PLUGIN_LOADED_EVENT, wxCommandEvent);



wxConfig *MainFrame::config = new wxConfig("wxSpot");

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
	: wxFrame((wxFrame *) nullptr, -1, title, pos, size),
	loginDialog(nullptr),
	timerStatusUpdate(this, wxID_ANY),
	activeSongIndex(0),
	activePlaylist(nullptr)

{
	loadPlugins();

	wxMenu *menuFile = new wxMenu();
	menuFile->Append(ID_Logout, "Logout");
	menuFile->Append(wxID_EXIT);

	wxMenu *pluginsFile = new wxMenu();
	for (size_t i = 0; i < plugInterfaces.size(); i++) {
		pluginsFile->Append(ID_Plugin + i, plugInterfaces.at(i)->getName());
	}
	pluginsFile->Bind(wxEVT_MENU, [=](wxCommandEvent &event) {
		wxLogDebug("Menu event: %d", event.GetId());
		PluginInterface *plug = plugInterfaces.at(event.GetId() - ID_Plugin);
		//Plugin *plugin = plugin.at(event.GetId() - ID_Plugin);
		//if (plugin->getSupports() & Plugin::SUPPORT_FLAGS::WINDOW) {
			/*plugin->start(this)->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent &event) {
				soundManager->setPCMPlugin(nullptr);
				event.Skip();
			});*/
		plug->start(this);
		/*plugin->getWindow()->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent &event) {
			soundManager->setPCMPlugin(nullptr);
			event.Skip();
		});*/
		
		event.Skip();
	});

	wxMenu *optionsFile = new wxMenu();
	optionsFile->Append(ID_Settings, "&Settings...\tCtrl-S");
	auto showSpectrum = optionsFile->AppendCheckItem(ID_ShowSpectrum, "Show Spectrum");
	showSpectrum->Check(true);

	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "&File");
	menuBar->Append(pluginsFile, "&Plugins");
	menuBar->Append(optionsFile, "&Settings");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	panel = new wxPanel(this, wxID_ANY);
	// TODO not sure if this is needed anymore
	/*panel->Bind(wxEVT_CHAR_HOOK, [=](wxKeyEvent &event) {
		event.Skip();
	});*/



	//auto horzBox = new wxBoxSizer(wxHORIZONTAL);
	auto vertBox = new wxBoxSizer(wxVERTICAL);
	auto bottomHorzBox = new wxBoxSizer(wxHORIZONTAL);

	spotifyManager = new SpotifyManager(this);

	auto windowSplitter = new wxSplitterWindow(panel, wxID_ANY);
	windowSplitter->SetSashGravity(0.3);
	windowSplitter->SetMinimumPaneSize(20);

	auto panel1 = new wxPanel(windowSplitter, wxID_ANY);
	auto panel2 = new wxPanel(windowSplitter, wxID_ANY);

	songList = new SongListCtrl(panel2, spotifyManager);
	songList->SetClientData(nullptr);

	songList->Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent &event) {
		// 0x48 == 'h'

		if (event.CmdDown() && event.GetKeyCode() == 0x48) {
			songList->setPlaylist(activePlaylist);
			songList->showCurrentTrack();
		}
		event.Skip();
	});


	playlistTree = new wxTreeCtrl(panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_EDIT_LABELS);
	playlistTree->AddRoot("Playlists");

	playlistTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &event) {
		wxTreeItemId item = event.GetItem();
		if (spotifyManager->getSearchResults()->getTreeItemId() == item) {
			Playlist *playlist = spotifyManager->getSearchResults();
			if (playlist != activePlaylist) {
				playlist->currentTrack = -1;
			}
			songList->setPlaylist(playlist);
			
			return;
		}
		std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

		for (auto &playlist : *playlists) {
			if (playlist->getTreeItemId() == item) {
				if (playlist != activePlaylist) {
					playlist->currentTrack = -1;
				}
				songList->setPlaylist(playlist);
				break;
			}
		}

	});

	playlistTree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, [=](wxTreeEvent &event) {
		wxTreeItemId item = event.GetItem();

		if (spotifyManager->getSearchResults()->getTreeItemId() == item) {
			// You can't do anything with the Search Playlist
			return;
		}
		else if (playlistTree->ItemHasChildren(item)) {
			wxMenu popup(playlistTree->GetItemText(item));
			popup.Append(ID_Playlist_Add_Playlist, "Add playlist");

			int selection = playlistTree->GetPopupMenuSelectionFromUser(popup, event.GetPoint());

			switch (selection) {
			case wxID_NONE:
				break;
			case ID_Playlist_Add_Playlist:
				wxTreeItemId newPlaylist = playlistTree->AppendItem(item, "New playlist");
				playlistTree->EditLabel(newPlaylist);
				//spotifyManager->createPlaylist("New playlist");
				break;
			}
		}
		else {
			wxMenu popup(playlistTree->GetItemText(item));
			popup.Append(ID_Playlist_Rename, "Rename");
			popup.Append(ID_Playlist_Delete, "Delete");

			int selection = playlistTree->GetPopupMenuSelectionFromUser(popup, event.GetPoint());

			switch (selection) {
			case wxID_NONE:
				break;
			case ID_Playlist_Rename:
				playlistTree->EditLabel(item);
				break;
			case ID_Playlist_Delete:
				std::unique_ptr<wxMessageDialog> dialog = std::make_unique<wxMessageDialog>(panel, wxT("Are you sure you want to delete ") + playlistTree->GetItemText(item), wxT("Delete"), wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);

				if (dialog->ShowModal() == wxID_OK) {
					auto playlists = spotifyManager->getPlaylists();

					for (auto &playlist : *playlists) {
						if (playlist->getTreeItemId() == item) {
							if (activePlaylist == playlist) {
								activePlaylist = nullptr;
							}
							spotifyManager->deletePlaylist(playlist);
							break;
						}
					}
					playlistTree->Delete(item);
				}
			}
		}
	});

	playlistTree->Bind(wxEVT_TREE_END_LABEL_EDIT, [=](wxTreeEvent &event) {
		// Need to detect if we are renaming or creating a new playlist
		wxTreeItemId item = event.GetItem();
		auto playlists = spotifyManager->getPlaylists();
		for (auto &playlist : *playlists) {
			if (playlist->getTreeItemId() == item) {
				spotifyManager->renamePlaylist(playlist, event.GetLabel());
				return;
			}
		}
		
		spotifyManager->createPlaylist(event.GetLabel());
	});

	playlistTree->SetDropTarget(new DnDTrackDropTarget(playlistTree, spotifyManager));

	songList->Bind(wxEVT_LIST_BEGIN_DRAG, [=](wxListEvent &event) {
		
		DnDTrackDataObject::Data data;

		data.tracks = new sp_track*[songList->GetSelectedItemCount()];

		int index = 0;
		long current = -1;
		while ((current = songList->GetNextItem(current, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) {
			data.tracks[index++] = songList->getTrack(current)->getSpTrack();
		}

		data.len = index;

		dnDDataObject.SetData(sizeof(DnDTrackDataObject::Data), &data);
		wxDropSource dropSource(dnDDataObject, playlistTree);
		dropSource.DoDragDrop(wxDrag_CopyOnly);
		delete[] data.tracks;
	});



	songList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [=](wxListEvent &event) {
		int index = event.GetIndex();

		Track *track = songList->getTrack(index);
		
		/*playlistTree->SetItemImage(activePlaylist->getTreeItemId(), 0);
		playlistTree->SetItemImage(activePlaylist->getTreeItemId(), 0, wxTreeItemIcon_Selected);*/
		if (activePlaylist != nullptr) {
			playlistTree->SetItemBold(activePlaylist->getTreeItemId(), false);
		}

		activePlaylist = (Playlist *)songList->GetClientData();
		/*playlistTree->SetItemImage(activePlaylist->getTreeItemId(), 1);
		playlistTree->SetItemImage(activePlaylist->getTreeItemId(), 1, wxTreeItemIcon_Selected);*/
		playlistTree->SetItemBold(activePlaylist->getTreeItemId(), true);
		playTrack(track);
		activeSongIndex = index;
	});

	songList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, [=](wxListEvent &event) {
		Track *track = songList->getTrack(event.GetIndex());
		//activePlaylist->currentTrack = -1;
		auto playlist = (Playlist *)songList->GetClientData();

		wxMenu *playlists = new wxMenu("Playlists");

		std::vector<SpotifyPlaylist *> *spotifyPlaylists = spotifyManager->getPlaylists();

		for (size_t i = 0; i < spotifyPlaylists->size(); i++) {
			playlists->Append(ID_Menu_Last_Dont_Use + i, spotifyPlaylists->at(i)->getTitle());
		}


		wxMenu popup(track->getTitle());

		popup.AppendSubMenu(playlists, "Add to playlist");
		popup.Append(ID_Menu_Open_Artist, "Open artist");
		popup.Append(ID_Menu_Open_Album, "Open album");
		popup.Append(ID_Menu_Delete_Track, "Delete track");
		popup.Append(ID_Menu_Copy_TrackName, "Copy track name");
		popup.Append(ID_Menu_Copy_URI, "Copy Spotify URI");
		popup.Append(ID_Menu_Copy_URL, "Copy Spotify URL");

		int selection = songList->GetPopupMenuSelectionFromUser(popup, event.GetPoint());


		switch (selection) {
		case wxID_NONE:
			break;
		case ID_Menu_Open_Artist:
			spotifyManager->search(track->getArtistLink());
			break;
		case ID_Menu_Open_Album:
			spotifyManager->search(track->getAlbumLink());
			break;
		case ID_Menu_Delete_Track:
			playlist->removeTrack(event.GetIndex());
			songList->SetItemCount(playlist->getTracks()->size());
			songList->RefreshItems(0, playlist->getTracks()->size() - 1);
			break;
		case ID_Menu_Copy_TrackName:
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData(new wxTextDataObject(track->getArtist() + " - " + track->getTitle()));
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


	searchTextCtrl = new wxTextCtrl(panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	searchTextCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent &event) {
		spotifyManager->search(searchTextCtrl->GetValue());
	});

	auto searchPlaylistBox = new wxBoxSizer(wxVERTICAL);

	searchPlaylistBox->Add(searchTextCtrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
	searchPlaylistBox->Add(playlistTree, 1, wxGROW);
	

	//horzBox->Add(searchPlaylistBox, 2, wxGROW);

	songList->Layout();


	panel1->SetSizer(searchPlaylistBox);
	auto panel2Sizer = new wxBoxSizer(wxHORIZONTAL);
	panel2Sizer->Add(songList, 1, wxGROW);
	panel2->SetSizer(panel2Sizer);
	windowSplitter->SplitVertically(panel1, panel2, -750);

	//horzBox->Add(songList, 4, wxGROW);
	vertBox->Add(windowSplitter, 2, wxGROW);
	//vertBox->Add(horzBox, 2, wxGROW);

	wxImage::AddHandler(new wxPNGHandler);
	//wxImage::AddHandler(new wxJPEGHandler);

	playImage = wxBITMAP_PNG_FROM_DATA(glyphicons_174_play);
	pauseImage = wxBITMAP_PNG_FROM_DATA(glyphicons_175_pause);
	nextImage = wxBITMAP_PNG_FROM_DATA(glyphicons_179_step_forward);
	prevImage = wxBITMAP_PNG_FROM_DATA(glyphicons_171_step_backward);
	smallPlayImage = wxBITMAP_PNG_FROM_DATA(glyphicons_174_play_small);

	imageList = new wxImageList(12, 14, false);

	//auto imageList = std::make_shared<wxImageList>(12, 14, false);

	imageList->Add(smallPlayImage);

	songList->SetImageList(imageList, wxIMAGE_LIST_SMALL);
	//playlistTree->SetImageList(imageList);



	buttonPlayPause = new wxBitmapButton(panel, wxID_ANY, playImage, wxDefaultPosition, wxSize(26, 26));
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
	});

	spectrum = new Spectrum(panel);



	textCurrentProgressTime = new wxStaticText(panel, wxID_ANY, "0:00");
	textTotalTime = new wxStaticText(panel, wxID_ANY, "0:00");

	
	checkBoxShuffle = new wxCheckBox(panel, wxID_ANY, _("Shuffle"));

	bottomHorzBox->Add(textCurrentProgressTime, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
	bottomHorzBox->Add(progressIndicator, 1, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 2);
	bottomHorzBox->Add(textTotalTime, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 2);
	bottomHorzBox->Add(checkBoxShuffle, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 2);
	bottomHorzBox->Add(spectrum, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 2);
	bottomHorzBox->Layout();


	vertBox->Add(bottomHorzBox, 0, wxEXPAND);

	vertBox->Layout();

	panel->SetSizer(vertBox);


	soundManager = new SoundManager(this);
	soundManager->setSpectrum(spectrum);

	spotifyManager->setEventHandler(GetEventHandler());

	wxString spotifyCachePath;
	int deviceIndex = 0;

	if (config->Read("Path", &spotifyCachePath) && wxDirExists(spotifyCachePath) &&
		config->Read("DeviceIndex", &deviceIndex) && soundManager->deviceSupported(deviceIndex)) {
		spotifyManager->init(spotifyCachePath);
		soundManager->init(deviceIndex);
	}
	else {
		auto dialog = std::make_unique<SettingsDialog>(soundManager);

		if (dialog->ShowModal() == wxID_OK) {
			config->Read("Path", &spotifyCachePath);
			config->Read("DeviceIndex", &deviceIndex);
			spotifyManager->init(spotifyCachePath);
			soundManager->init(deviceIndex);
		}
	}

#ifdef WIN32

	if (!RegisterHotKey(ID_HotKey_PlayPause, 0, VK_MEDIA_PLAY_PAUSE))
		wxLogError("Could not register play/pause hotkey");
	if (!RegisterHotKey(ID_HotKey_PlayPause, 0, 0xCF))
		wxLogError("Could not register play/pause hotkey");

	RegisterHotKey(ID_HotKey_Prev, 0, VK_MEDIA_PREV_TRACK);
	RegisterHotKey(ID_HotKey_Next, 0, VK_MEDIA_NEXT_TRACK);
#endif
	// TODO - only show if we fail to login
	if (spotifyManager->login() == false) {
		showLoginDialog();
	}

}

MainFrame::~MainFrame()
{
	unLoadPlugins();
	soundManager->stop();
	soundManager->end();

	spotifyManager->end();
	delete imageList;
	delete spotifyManager;
	delete soundManager;
	delete config;

}

#ifdef WIN32
WXLRESULT MainFrame::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (nMsg == WM_APPCOMMAND) {
		playPause();
		return true;
	}
	return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
}
#endif

void MainFrame::loadPlugins()
{
	wxDynamicLibrary dll;

	if (dll.Load(wxGetCwd() + "\\" + wxDynamicLibrary::CanonicalizeName("projectMPlugin"))) {
		wxDYNLIB_FUNCTION(Start_function, start, dll);
		wxDYNLIB_FUNCTION(GetName_function, getName, dll);
		wxDYNLIB_FUNCTION(Stop_function, stop, dll);
		wxDYNLIB_FUNCTION(GetPlugin_function, getPlugin, dll);

		PluginInterface *pluginInterface = new PluginInterface();

		pluginInterface->start = pfnstart;
		pluginInterface->stop = pfnstop;
		pluginInterface->getName = pfngetName;
		pluginInterface->getPlugin = pfngetPlugin;

		if (pfnstart) {
			pluginInterface->dll = dll.Detach();
			wxLogDebug("plugin name: %s", pluginInterface->getName());
			plugInterfaces.push_back(pluginInterface);
		}
	}
}

void MainFrame::unLoadPlugins()
{
	for (auto &plugInterface : plugInterfaces) {
		wxDynamicLibrary::Unload(plugInterface->dll);
		delete plugInterface;
	}
}

void MainFrame::OnExit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnLogout(wxCommandEvent &event)
{
	spotifyManager->logout();
	wxMessageBox("You will be forced to enter your credentials next time you log in");
}

void MainFrame::OnSettings(wxCommandEvent &event)
{
	std::unique_ptr<SettingsDialog> dialogue = std::make_unique<SettingsDialog>(soundManager);
	if (dialogue->ShowModal() == wxID_OK) {

	}
}

void MainFrame::OnShowSpectrum(wxCommandEvent &event)
{
	/*wxMenuItem *test = (wxMenuItem*)event.GetClientData();
	if (test->IsCheck()) {
		wxLogDebug("checked");
	}
	else {
		wxLogDebug("not checked");
	}*/
	if (soundManager->getSpectrum() != nullptr) {
		soundManager->setSpectrum(nullptr);
		spectrum->clear();
	}
	else {
		soundManager->setSpectrum(spectrum);
	}
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox("wxSpot 1.0.3 by Viktor M�ntzing", "About", wxOK | wxICON_INFORMATION);
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
	/*soundManager->stop();
	buttonPlayPause->SetImageLabel(playImage);
	timerStatusUpdate.Stop();*/
}

void MainFrame::OnSpotifyEndOfTrackEvent(wxCommandEvent &event)
{
	audioBuffer.flush();
	//wxLogDebug("%s", __FUNCTIONW__);
	//next();
}

void MainFrame::OnSpotifyLoadedContainerEvent(wxCommandEvent &event)
{
	resetPlaylistTree();
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

	if (index >= playlists->size()) {
		// Old event. we don't care
		return;
	}

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
	if (loginDialog != nullptr) {
		loginDialog->GetEventHandler()->QueueEvent(event.Clone());
	}
}

void MainFrame::OnSpotifyMetaUpdatedEvent(wxCommandEvent &event)
{
	//resetPlaylistTree();
	updatePlaylistTree();
}

void MainFrame::OnSpotifyPlayNextEvent(wxCommandEvent &event)
{
	next();
}

void MainFrame::OnPluginCloseEvent(wxCommandEvent &event)
{
	soundManager->setPCMPlugin(nullptr);
	plugInterfaces[0]->stop();
	wxLogDebug("OnPluginCloseEvent");
}

void MainFrame::OnPluginLoadedEvent(wxCommandEvent &event)
{
	Plugin *plugin = plugInterfaces[0]->getPlugin();
	soundManager->setPCMPlugin(plugin);
	wxLogDebug("OnPluginLoadedEvent");
}


void MainFrame::OnTimerEvent(wxTimerEvent &event)
{
	unsigned int currTime = audioBuffer.getPlayTime();

	textCurrentProgressTime->SetLabel(wxString::Format("%d:%02d", currTime / 60000, (currTime / 1000) % 60));

	unsigned int duration = spotifyManager->getSongLength();

	progressIndicator->SetValue(((double)currTime / (double)duration));
}

void MainFrame::OnHotKeyPlayPause(wxKeyEvent &event)
{
	playPause();
}

void MainFrame::OnHotKeyPrev(wxKeyEvent &event)
{
	prev();
}

void MainFrame::OnHotKeyNext(wxKeyEvent &event)
{
	next();
}

void MainFrame::OnHotKeyShowCurrentTrack(wxKeyEvent &event)
{
	songList->showCurrentTrack();
}

void MainFrame::showLoginDialog()
{
	loginDialog = new LoginDialog();

	loginDialog->setSpotifyManager(spotifyManager);


	if (loginDialog->ShowModal() == wxID_OK) {
		delete loginDialog;
		loginDialog = nullptr;
	}

}

void MainFrame::playPause()
{
	if (spotifyManager->m_isPlaying) {
		soundManager->stop();
		buttonPlayPause->SetImageLabel(playImage);
		timerStatusUpdate.Stop();
	}
	else {
		buttonPlayPause->SetImageLabel(pauseImage);
		timerStatusUpdate.Start(1000, false);
		soundManager->play();
	}
	/*soundManager->stop();
	buttonPlayPause->SetImageLabel(playImage);
	timerStatusUpdate.Stop();*/

	spotifyManager->playPause();
}

bool MainFrame::next()
{
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
		else if (activeSongIndex >= activePlaylist->getTracks()->size() - 1) {
			timerStatusUpdate.Stop();
			soundManager->stop();
			activeSongIndex = activePlaylist->getTracks()->size() - 1;
			return false;
		}
		else {
			activeSongIndex++;
		}
		playTrack(activePlaylist->getTracks()->at(activeSongIndex).get());
	}
	return true;

}

bool MainFrame::prev()
{
	if (activePlaylist != nullptr) {
		if (activeSongIndex == 0) {
			timerStatusUpdate.Stop();
			soundManager->stop();
			return false;
		}
		else {
			activeSongIndex--;
			playTrack(activePlaylist->getTracks()->at(activeSongIndex).get());
		}
	}

	return true;
}


void MainFrame::sendEvent(const wxEventType type, int cargo)
{
	wxThreadEvent evt(type);
	evt.SetInt(cargo);

	QueueEvent(evt.Clone());
}

void MainFrame::highLightTrack(const Track *track)
{
	auto tracks = activePlaylist->getTracks();
	for (unsigned int i = 0; i < tracks->size(); i++) {
		if (track == tracks->at(i).get()) {
			activePlaylist->currentTrack = i;

			if ((Playlist*)songList->GetClientData() == activePlaylist) {
				//songList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				songList->setHighLight(i);
			}

			break;
		}
	}
}

void MainFrame::playTrack(Track *const track)
{
	if (spotifyManager->playTrack(track)) {
		textCurrentProgressTime->SetLabelText("0:00");
		progressIndicator->SetValue(0.0f);
		highLightTrack(track);
		wxCommandEvent temp;
		OnSpotifyStartedPlayingEvent(temp);
	}
}

void MainFrame::resetPlaylistTree()
{
	std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

	playlistTree->Freeze();
	wxTreeItemId parent = playlistTree->GetRootItem();
	playlistTree->DeleteChildren(parent);

	wxTreeItemId searchResults = playlistTree->AppendItem(parent, "Search results");
	spotifyManager->getSearchResults()->setTreeItemId(searchResults);

	shared = playlistTree->AppendItem(parent, "Shared");

	own = playlistTree->AppendItem(parent, "Own");

	for (auto &playlist : *playlists) {
		wxTreeItemId item;
		if (playlist->isShared()) {
			item = playlistTree->AppendItem(shared, playlist->getTitle());
		}
		else {
			item = playlistTree->AppendItem(own, playlist->getTitle());
		}

		playlist->setTreeItemId(item);
	}

	playlistTree->ExpandAll();
	playlistTree->Thaw();
}

void MainFrame::updatePlaylistTree()
{
	std::vector<SpotifyPlaylist*> *playlists = spotifyManager->getPlaylists();

	if (!own.IsOk() || !shared.IsOk())
		return;

	//playlistTree->Freeze();

	wxTreeItemIdValue sharedCookie, ownCookie;
	wxTreeItemId sharedItem = playlistTree->GetFirstChild(shared, sharedCookie);
	wxTreeItemId ownItem = playlistTree->GetFirstChild(own, ownCookie);
	for (auto &playlist : *playlists) {
		wxTreeItemId item;

		if (playlist->isShared()) {
			if (sharedItem.IsOk()) {
				if (!playlist->getTitle().IsSameAs(playlistTree->GetItemText(sharedItem))) {
					playlistTree->SetItemText(sharedItem, playlist->getTitle());
				}
			}
			else {
				item = playlistTree->AppendItem(shared, playlist->getTitle());
				playlist->setTreeItemId(item);
			}
			sharedItem = playlistTree->GetNextChild(sharedItem, sharedCookie);
		}
		else {
			if (ownItem.IsOk()) {
				if (!playlist->getTitle().IsSameAs(playlistTree->GetItemText(ownItem))) {
					playlistTree->SetItemText(ownItem, playlist->getTitle());
				}
			}
			else {
				item = playlistTree->AppendItem(own, playlist->getTitle());
				playlist->setTreeItemId(item);
			}
			ownItem = playlistTree->GetNextChild(ownItem, ownCookie);
		}
		/*wxTreeItemId item;
		if (playlist->isShared()) {
			item = playlistTree->AppendItem(shared, playlist->getTitle());
		}
		else {
			item = playlistTree->AppendItem(own, playlist->getTitle());
		}

		playlist->setTreeItemId(item);*/
	}

	//playlistTree->Thaw();
	
}

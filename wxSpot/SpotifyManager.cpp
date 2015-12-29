#include "SpotifyManager.h"
#include "AppKey.h"

#include "Main.h"
#include "AudioBuffer.h"
//#include "Log.h"
#include <wx/log.h>


wxDEFINE_EVENT(SPOTIFY_WAKE_UP_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_LOGGED_IN_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_STARTED_PLAYING_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_STOPPED_PLAYING_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_END_OF_TRACK_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_LOADED_CONTAINER_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_PLAYLIST_ADDED_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_PLAYLIST_REMOVED_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_PLAYLIST_RENAMED_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_PLAYLIST_STATE_CHANGED_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_SEARCH_RESULTS_EVENT, wxCommandEvent);
wxDEFINE_EVENT(SPOTIFY_PLAY_NEXT_EVENT, wxCommandEvent);



const size_t g_appkey_size = sizeof(g_appkey);

sp_track *g_track;

#define USER_AGENT "wxSpot"


void tryToPlay(sp_session *sess);

static SpotifyManager *GetManagerFromUserdata(void *userData)
{
	SpotifyManager *manager = reinterpret_cast<SpotifyManager*>(userData);
	return manager;
}

static SpotifyManager *GetManagerFromSession(sp_session *sess)
{
	SpotifyManager *manager = reinterpret_cast<SpotifyManager*>(sp_session_userdata(sess));
	return manager;
}

static SpotifyPlaylist *GetPlaylistFromSpPlaylist(SpotifyManager *manager, sp_playlist *pl)
{
	std::vector<SpotifyPlaylist*> *playlists = manager->getPlaylists();
	for (unsigned int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i)->getSpPlaylist() == pl) {
			return playlists->at(i);
		}
	}
	return nullptr;
}

static void SP_CALLCONV callback_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position, void *userData)
{
	SpotifyManager *manager = GetManagerFromUserdata(userData);

	wxLogDebug("%s Tracks added: num: %d position: %d", __FUNCTIONW__, num_tracks, position);

	SpotifyPlaylist *playlist = GetPlaylistFromSpPlaylist(manager, pl);

	if (playlist == nullptr) return;

	for (int i = 0; i < num_tracks; i++) {
		// This causes duplicates when adding tracks to a brand new playlist
		//playlist->addTrack(tracks[i]);
	}
	
}

static void SP_CALLCONV callback_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userData)
{
	wxLogDebug("callback - Tracks removed");
}

static void SP_CALLCONV callback_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userData)
{
	wxLogDebug("callback - Tracks moved");
}

static void SP_CALLCONV callbacks_playlist_renamed(sp_playlist *pl, void *userData)
{
	//wxMessageBox(sp_playlist_name(pl));
	wxLogDebug("callback - Playlist renamed");
	SpotifyManager *manager = GetManagerFromUserdata(userData);

	std::vector<SpotifyPlaylist*> *playlists = manager->getPlaylists();

	for (unsigned int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i)->getSpPlaylist() == pl) {
			manager->sendEvent(SPOTIFY_PLAYLIST_RENAMED_EVENT, i);
			return;
		}
	}

	manager->sendEvent(SPOTIFY_PLAYLIST_RENAMED_EVENT, 0);
}

static void SP_CALLCONV callbacks_playlist_state_changed(sp_playlist *pl, void *userData)
{
	SpotifyManager *manager = GetManagerFromUserdata(userData);

	wxLogDebug("Playlist state changed: %s : collab: %d pending changes: %d loaded: %d", sp_playlist_name(pl), sp_playlist_is_collaborative(pl), sp_playlist_has_pending_changes(pl), sp_playlist_is_loaded(pl));
	std::vector<SpotifyPlaylist*> *playlists = manager->getPlaylists();

	for (unsigned int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i)->getSpPlaylist() == pl) {
			manager->sendEvent(SPOTIFY_PLAYLIST_STATE_CHANGED_EVENT, i);
			return;
		}
	}
}

static void SP_CALLCONV callbacks_playlist_metadata_updated(sp_playlist *pl, void *userData)
{
	SpotifyManager *manager = GetManagerFromUserdata(userData);

	wxLogDebug("Playlist metadata updated: %s", sp_playlist_name(pl));

	std::vector<SpotifyPlaylist*> *playlists = manager->getPlaylists();

	for (unsigned int i = 0; i < playlists->size(); i++) {
		if (playlists->at(i)->getSpPlaylist() == pl) {
			//manager->sendEvent(SPOTIFY_PLAYLIST_STATE_CHANGED_EVENT, i);
			wxLogDebug("Playlist metadata updated: %d", i);

			return;
		}
	}

}

static sp_playlist_callbacks pl_callbacks = {
	&callback_tracks_added,
	&callback_tracks_removed,
	&callback_tracks_moved,
	&callbacks_playlist_renamed,
	&callbacks_playlist_state_changed,
	nullptr,
	&callbacks_playlist_metadata_updated
};

static void SP_CALLCONV callback_playlist_added(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userData)
{
	sp_playlist_add_callbacks(pl, &pl_callbacks, userData);

	SpotifyManager *manager = GetManagerFromUserdata(userData);

	//manager->addPlaylist(Playlist(pl));

	manager->sendEvent(SPOTIFY_PLAYLIST_ADDED_EVENT);

}

static void SP_CALLCONV callback_playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userData)
{
	wxLogDebug("%s", __FUNCTIONW__);
	sp_playlist_remove_callbacks(pl, &pl_callbacks, nullptr);
}

static void SP_CALLCONV callback_container_loaded(sp_playlistcontainer *pc, void *userData)
{
	SpotifyManager *manager = GetManagerFromUserdata(userData);
	int num = sp_playlistcontainer_num_playlists(pc);
	bool dirty = false;
	for (int i = 0; i < num; i++) {

		
		sp_playlist *spPlaylist = sp_playlistcontainer_playlist(pc, i);

		std::vector<SpotifyPlaylist *> *existingPlaylists = manager->getPlaylists();

		bool alreadyHave = false;
		for (size_t i = 0; i < existingPlaylists->size(); i++) {
			if (existingPlaylists->at(i)->getSpPlaylist() == spPlaylist) {
				alreadyHave = true;
				break;
			}
		}

		if (alreadyHave) {
			wxLogDebug("%s Playlist already exists - ignoring", __FUNCTIONW__);
			continue;

		}

		dirty = true;

		//sp_playlist_add_ref(spPlaylist);

		SpotifyPlaylist *playlist = new SpotifyPlaylist(spPlaylist);

		if (sp_playlist_is_loaded(spPlaylist)) {
			int num = sp_playlist_num_tracks(spPlaylist);
			for (int i = 0; i < num; i++) {
				playlist->addTrack(sp_playlist_track(spPlaylist, i));
			}
		}
		else {
			wxLogDebug("%s Playlist not loaded", __FUNCTIONW__);
		}
		wxLogDebug("%s Adding playlist: %s", __FUNCTIONW__, playlist->getTitle());
		manager->addPlaylist(playlist);
	}

	if (dirty) {
		manager->sendEvent(SPOTIFY_LOADED_CONTAINER_EVENT);
	}
	
}

static sp_playlistcontainer_callbacks pc_callbacks = {
	&callback_playlist_added,
	&callback_playlist_removed,
	NULL,
	&callback_container_loaded
};

static void SP_CALLCONV callback_logged_in(sp_session *sess, sp_error error)
{
	if (error != SP_ERROR_OK) {
		wxLogError("Error logging in");
		return;
	}

	sp_session_set_private_session(sess, true);


	SpotifyManager *manager = GetManagerFromSession(sess);
	manager->sendEvent(SPOTIFY_LOGGED_IN_EVENT);

	sp_playlistcontainer *pc = sp_session_playlistcontainer(sess);

	sp_playlistcontainer_add_callbacks(pc, &pc_callbacks, GetManagerFromSession(sess));

}

static void SP_CALLCONV callback_logged_out(sp_session *sess)
{
	wxLogDebug("Logged out callback");
}

static void SP_CALLCONV callback_connection_error(sp_session *sess, sp_error error)
{
	wxLogError("Connection error: %s", sp_error_message(error));
}

static void SP_CALLCONV callback_notify_main_thread(sp_session *sess)
{
	wxLogDebug("callback - Notify main thread");
	SpotifyManager *manager = GetManagerFromSession(sess);
	manager->sendEvent(SPOTIFY_WAKE_UP_EVENT);
}

static void SP_CALLCONV callback_log_message(sp_session *sess, const char *data)
{
	wxLogDebug("Log message: %s", data);
}

static void SP_CALLCONV callback_metadata_updated(sp_session *sess)
{
	//tryToPlay(sess);
	wxLogDebug("Meta updated");
}

static int SP_CALLCONV callback_music_delivery(sp_session *sess, const sp_audioformat *format, const void *frames, int num_frames)
{
	SpotifyManager *manager = GetManagerFromSession(sess);

	if (frames != nullptr) {
		return manager->addSoundData(frames, num_frames * 2);
	}

	return 0;

}

static void SP_CALLCONV callback_play_token_lost(sp_session *sess)
{
	// TODO flush audio

	if (g_track != nullptr) {
		sp_session_player_unload(sess);
		g_track = nullptr;
	}

	wxLogDebug("Play token lost callback");
}

static void SP_CALLCONV callback_playback_start(sp_session *sess)
{
	SpotifyManager *manager = GetManagerFromSession(sess);
	manager->sendEvent(SPOTIFY_STARTED_PLAYING_EVENT);
}

static void SP_CALLCONV callback_playback_stop(sp_session *sess)
{
	SpotifyManager *manager = GetManagerFromSession(sess);
	manager->sendEvent(SPOTIFY_STOPPED_PLAYING_EVENT);
}

static void SP_CALLCONV callback_end_of_track(sp_session *sess)
{
	SpotifyManager *manager = GetManagerFromSession(sess);
	manager->sendEvent(SPOTIFY_END_OF_TRACK_EVENT);
	wxLogDebug("End of track callback");
	manager->m_endOfTrack = true;
}

static void SP_CALLCONV callback_get_audio_buffer_stats(sp_session *sess, sp_audio_buffer_stats *stats)
{
	SpotifyManager *manager = GetManagerFromSession(sess);

	manager->getAudioStatus(&stats->stutter, &stats->samples);
	/*stats->samples = manager->getSampleDiff();
	stats->stutter = manager->getStutter();*/
}

static sp_session_callbacks session_callbacks = {
	&callback_logged_in,
	&callback_logged_out,
	&callback_metadata_updated,
	&callback_connection_error,
	NULL,
	&callback_notify_main_thread,
	&callback_music_delivery,
	&callback_play_token_lost,
	&callback_log_message,
	&callback_end_of_track,
	NULL,
	NULL,
	&callback_playback_start,
	&callback_playback_stop,
	//&callback_get_audio_buffer_stats
	NULL
};

static void SP_CALLCONV callback_search_complete(sp_search *search, void *userData)
{
	SpotifyManager *manager = GetManagerFromUserdata(userData);
	if (sp_search_error(search) == SP_ERROR_OK) {
		Playlist *searchResults = manager->getSearchResults();
		searchResults->clearTracks();
		for (int i = 0; i < sp_search_num_tracks(search); i++) {
			searchResults->addTrack(sp_search_track(search, i));
		}

	}

	manager->sendEvent(SPOTIFY_SEARCH_RESULTS_EVENT);

	//sp_search_release(search);
}

void tryToPlay(sp_session *sess)
{
	wxLogDebug("Trying to play");
	SpotifyManager *manager = GetManagerFromSession(sess);

	if (manager->m_isPlaying) return;
	sp_error err = sp_track_error(g_track);
	if (err != SP_ERROR_OK) {
		wxLogDebug("Error with track");
		return;
	}

	err = sp_session_player_load(sess, g_track);
	if (err != SP_ERROR_OK) {
		wxLogDebug("Error loading track");
		return;
	}


	err = sp_session_player_play(sess, true);
	if (err != SP_ERROR_OK) {
		wxLogError("Error playing track");
		return;
	}

	manager->m_isPlaying = true;

}


SpotifyManager::SpotifyManager(MainFrame *main) : m_pMainFrame(main), m_isPlaying(false), m_pSession(nullptr)
{
	processEventsTimer.Bind(wxEVT_TIMER, [=](wxTimerEvent &event) {
		wxLogDebug("Timeout - calling process events");
		processEvents();
	});
}


SpotifyManager::~SpotifyManager()
{

}

void SpotifyManager::init(wxString cachePath)
{
	sp_session_config config;

	memset(&config, 0, sizeof(config));

	config.api_version = SPOTIFY_API_VERSION;
	config.cache_location = cachePath.mb_str();
	config.settings_location = cachePath.mb_str();
	config.application_key = g_appkey;
	config.application_key_size = g_appkey_size;
	config.user_agent = USER_AGENT;
	config.callbacks = &session_callbacks;

	config.userdata = this;

	sp_error error = sp_session_create(&config, &m_pSession);

	if (error != SP_ERROR_OK) {
		wxLogError("Failed to create session: %s", sp_error_message(error));
		return;
		//std::cerr << "failed to create session" << sp_error_message(error) << std::endl;
	}

	error = sp_session_preferred_bitrate(m_pSession, SP_BITRATE_320k);

	if (error != SP_ERROR_OK) {
		std::cerr << "failed setting high quality streaming" << sp_error_message(error) << std::endl;
	}

	sp_session_set_volume_normalization(m_pSession, false);

}

bool SpotifyManager::login(const wxString username, const wxString password, bool remember)
{
	sp_error error;
	//error = sp_session_login(m_pSession, username.mb_str(), password.mb_str(), true, 0);
	
	if (password == wxEmptyString) {
		if (sp_session_relogin(m_pSession) == SP_ERROR_NO_CREDENTIALS) {
			wxLogDebug("No stored credentials");
			return false;
		}
		return true;
	}
	error = sp_session_login(m_pSession, username.mb_str(), password.mb_str(), remember, 0);

	if (error != SP_ERROR_OK) {
		wxLogDebug("Failed to login: %s", sp_error_message(error));
		return false;
	}

	return true;
}

void SpotifyManager::logout()
{
	sp_session_forget_me(m_pSession);
}

void SpotifyManager::end()
{
	if (g_track != nullptr) {
		sp_session_player_unload(m_pSession);
		g_track = nullptr;
	}
	int temp;
	sp_session_process_events(m_pSession, &temp);

	std::for_each(playlists.begin(), playlists.end(), std::default_delete<SpotifyPlaylist>());

	if (m_pSession != nullptr) {
		sp_session_logout(m_pSession);
		sp_session_release(m_pSession);
		m_pSession = nullptr;

	}
}

void SpotifyManager::setEventHandler(wxEvtHandler *eventHandler)
{
	this->m_eventHandler = eventHandler;
}

void SpotifyManager::processEvents()
{
	int timeout = 0;
	sp_session_process_events(m_pSession, &timeout);
	processEventsTimer.Start(timeout, true);
}

void SpotifyManager::sendEvent(const wxEventType type, int cargo) 
{
	wxCommandEvent evt = wxCommandEvent(type);
	evt.SetInt(cargo);
	m_eventHandler->QueueEvent(evt.Clone());
}

int SpotifyManager::addSoundData(const void *frames, int num_frames)
{
	AudioBuffer *buffer = m_pMainFrame->getAudioBuffer();

	if (m_endOfTrack == false) {
		return buffer->addData((short *)frames, num_frames);
	}
	else {
		//sendEvent(SPOTIFY_PLAY_NEXT_EVENT);
	}


	return 0;
}


void SpotifyManager::addPlaylist(SpotifyPlaylist *playlist)
{
	playlists.push_back(playlist);
}


void SpotifyManager::getAudioStatus(int *stutter, int *sampleDiff)
{
	//wxLogDebug("%s", __FUNCTIONW__);
	AudioBuffer *buffer = m_pMainFrame->getAudioBuffer();
	//buffer->getBufferStatus(stutter, sampleDiff);
}

bool SpotifyManager::playTrack(Track *const track)
{
	wxLogDebug("%s Track: %s", __FUNCTIONW__, track->getTitle());
	if (!isTrackAvailable(track)) {
		sendEvent(SPOTIFY_END_OF_TRACK_EVENT);
		return false;
	}

	if (g_track != nullptr) {
		wxLogDebug("Unloading previous track");
		sp_session_player_unload(m_pSession);
		g_track = nullptr;
		m_isPlaying = false;
	}
	
	m_pMainFrame->getAudioBuffer()->reset();

	g_track = track->getSpTrack();
	tryToPlay(m_pSession);
	if (m_isPlaying) {
		m_endOfTrack = false;
	}

	return true;

}

void SpotifyManager::playPause()
{
	if (g_track != nullptr) {
		sp_error err = sp_session_player_play(m_pSession, !m_isPlaying);
		if (err == SP_ERROR_OK) {
			m_isPlaying = !m_isPlaying;
		}
	}
}

unsigned int SpotifyManager::seek(unsigned int position)
{
	if (g_track != nullptr) {
		sp_error error = sp_session_player_seek(m_pSession, position);
		if (error == SP_ERROR_OK) {
			m_pMainFrame->getAudioBuffer()->setPlayTime(position);
			return position;
		}
		
	}
	return 0;
}

void SpotifyManager::search(wxString searchString)
{
	if (searchString.Lower().StartsWith("spotify:")) {
		sp_link *link = sp_link_create_from_string(searchString.mb_str());
		search(link);
	} else {
		sp_search_create(m_pSession, searchString.mb_str(), 0, 100, 0, 100, 0, 100, 0, 100, SP_SEARCH_STANDARD, callback_search_complete, this);
	}
	
}

void SpotifyManager::search(sp_link *link)
{
	if (link == nullptr) return;

	switch (sp_link_type(link)) {
	case SP_LINKTYPE_TRACK: {
		sp_track *track = sp_link_as_track(link);
		sp_track_add_ref(track);
		if (track != nullptr) {
			searchResults.clearTracks();
			searchResults.addTrack(track);
			sendEvent(SPOTIFY_SEARCH_RESULTS_EVENT);
		}
		break;
	}
	case SP_LINKTYPE_ALBUM:
		sp_albumbrowse_create(m_pSession, sp_link_as_album(link), [](sp_albumbrowse *browse, void *userdata){
			if (sp_albumbrowse_error(browse) != SP_ERROR_OK) {
				return;
			}
			SpotifyManager *manager = GetManagerFromUserdata(userdata);
			Playlist *searchResults = manager->getSearchResults();
			searchResults->clearTracks();

			for (int i = 0; i < sp_albumbrowse_num_tracks(browse); i++) {
				searchResults->addTrack(sp_albumbrowse_track(browse, i));
			}
			//sp_albumbrowse_release(browse);
			manager->sendEvent(SPOTIFY_SEARCH_RESULTS_EVENT);
		}, this);
		break;
	case SP_LINKTYPE_ARTIST:
		sp_artistbrowse_create(m_pSession, sp_link_as_artist(link), SP_ARTISTBROWSE_FULL, [](sp_artistbrowse *browse, void *userdata) {
			if (sp_artistbrowse_error(browse) != SP_ERROR_OK) {
				return;
			}
			SpotifyManager *manager = GetManagerFromUserdata(userdata);
			Playlist *searchResults = manager->getSearchResults();
			searchResults->clearTracks();

			for (int i = 0; i < sp_artistbrowse_num_tracks(browse); i++) {
				searchResults->addTrack(sp_artistbrowse_track(browse, i));
			}
			//sp_artistbrowse_release(browse);
			manager->sendEvent(SPOTIFY_SEARCH_RESULTS_EVENT);
		}, this);
		break;
	case SP_LINKTYPE_PLAYLIST:
		// TODO
		break;
	}
	sp_link_release(link);
}


unsigned int SpotifyManager::getSongLength()
{
	//wxLogDebug("%s", __FUNCTIONW__);
	if (g_track == nullptr) {
		return 0;
	}
	unsigned int length = sp_track_duration(g_track);
	return length;
}

wxString SpotifyManager::getSongName()
{
	wxLogDebug("%s", __FUNCTIONW__);
	if (g_track != nullptr) {
		if (sp_track_is_loaded(g_track)) {
			wxString artists = "";
			for (int i = 0; i < sp_track_num_artists(g_track); i++) {
				sp_artist *art = sp_track_artist(g_track, i);

				artists.append(wxString::FromUTF8(sp_artist_name(art)));
				artists.append(", ");
			}
			artists.Truncate(artists.Length() - 2);

			wxString title = wxString::FromUTF8(sp_track_name(g_track));
			title.Append(" - ");
			title.Append(artists);
			return title;
		}
	}
	return wxString("");

}

bool SpotifyManager::isTrackAvailable(Track *const track)
{
	//wxLogDebug("%s", __FUNCTIONW__);
	sp_track *spTrack = track->getSpTrack();

	if (sp_track_is_loaded(spTrack)) {
		bool available = sp_track_get_availability(m_pSession, spTrack) == SP_TRACK_AVAILABILITY_AVAILABLE;
		return available;
	}

	return false;
}

void SpotifyManager::addTrackToPlaylist(Track *track, SpotifyPlaylist *playlist)
{
	sp_track *spTrack = track->getSpTrack();
	sp_playlist *spPlaylist = playlist->getSpPlaylist();

	sp_playlist_add_tracks(spPlaylist, &spTrack, 1, sp_playlist_num_tracks(spPlaylist), m_pSession);

}

void SpotifyManager::createPlaylist(const wxString name)
{
	sp_playlistcontainer *container = sp_session_playlistcontainer(m_pSession);
	sp_playlistcontainer_add_new_playlist(container, name);
}

void SpotifyManager::deletePlaylist(SpotifyPlaylist *playlist)
{
	sp_playlistcontainer *container = sp_session_playlistcontainer(m_pSession);
	unsigned int pos;
	bool found = false;
	for (pos = 0; pos < playlists.size(); pos++) {
		if (playlists.at(pos) == playlist) {
			sp_playlistcontainer_remove_playlist(container, pos);
			found = true;
			break;
		}
	}
	if (found) {
		playlists.erase(playlists.begin() + pos);
		// Switch to smart pointers instead
		delete playlist;
	}
	
}

void SpotifyManager::renamePlaylist(SpotifyPlaylist *playlist, const wxString newName)
{
	sp_playlist_rename(playlist->getSpPlaylist(), newName);
}
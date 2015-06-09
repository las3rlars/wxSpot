#pragma once


#include <wx/event.h>

wxDECLARE_EVENT(SPOTIFY_WAKE_UP_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_LOGGED_IN_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_STARTED_PLAYING_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_STOPPED_PLAYING_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_END_OF_TRACK_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_LOADED_CONTAINER_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_PLAYLIST_ADDED_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_PLAYLIST_REMOVED_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_PLAYLIST_RENAMED_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_PLAYLIST_STATE_CHANGED_EVENT, wxCommandEvent);
wxDECLARE_EVENT(SPOTIFY_SEARCH_RESULTS_EVENT, wxCommandEvent);

class MainFrame;

#include <vector>
#include <libspotify/api.h>
#include "Playlist.h"



class SpotifyManager
{
public:
	SpotifyManager(MainFrame *mainFrame);
	~SpotifyManager();

	void init(wxString cachePath);
	void end();

	void login(const wxString username, const wxString password);

	void setEventHandler(wxEvtHandler *eventHandler);

	void processEvents();
	void sendEvent(const wxEventType type, int cargo = -1);
	
	
	void addSoundData(const void *data, int num_frames);
	void addPlaylist(Playlist *playlist);
	int getSampleDiff();
	int getStutter();

	bool playTrack(Track *track);
	void playPause();
	unsigned int seek(unsigned int position);

	void search(const wxString searchstring);

	std::vector<Playlist*> *getPlaylists() { return &playlists; }
	std::vector<Track*> *getSearchResults() { return &searchResults; }

	unsigned int getSongLength();
	wxString getSongName();

	bool isTrackAvailable(Track *track);

	bool m_isPlaying;

	sp_session *getSession() { return m_pSession; }

private:
	sp_session *m_pSession;
	MainFrame *m_pMainFrame;
	sp_track *m_spTrack;
	wxEvtHandler *m_eventHandler;

	std::vector<Playlist*> playlists;
	std::vector<Track*> searchResults;

};


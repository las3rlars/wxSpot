#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

#include <wx/config.h>
#include <random>

#include "AudioBuffer.h"

class SpotifyManager;
class SoundManager;

class SongListCtrl;
class wxTreeCtrl;
class Playlist;
class LoginDialog;
class ProgressIndicator;
class Track;

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
	~MainFrame();


	SoundManager *getSoundManager() { return soundManager;  }
	AudioBuffer *getAudioBuffer() { return &audioBuffer;  }
	void bufferDone();

	static wxConfig *config;
private:

	enum {
		ID_HotKey_PlayPause = 1,
		ID_HotKey_Prev,
		ID_HotKey_Next
	};

	enum {
		ID_Logout = 1,
		ID_Settings,
		ID_MilkDrop,
		ID_Menu_Add_To_Playlist,
		ID_Menu_Delete_Track,
		ID_Menu_Copy_TrackName,
		ID_Menu_Copy_URI,
		ID_Menu_Copy_URL,
		ID_Menu_Last_Dont_Use
	};

	void OnExit(wxCommandEvent &event);
	void OnLogout(wxCommandEvent &event);
	void OnSettings(wxCommandEvent &event);
	void OnMilkDrop(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnSpotifyWakeUpEvent(wxCommandEvent &event);
	void OnSpotifyStartedPlayingEvent(wxCommandEvent &event);
	void OnSpotifyStoppedPlayingEvent(wxCommandEvent &event);
	void OnSpotifyEndOfTrackEvent(wxCommandEvent &event);
	void OnSpotifyLoadedContainerEvent(wxCommandEvent &event);
	void OnSpotifyPlaylistAddedEvent(wxCommandEvent &event);
	void OnSpotifyPlaylistRenamedEvent(wxCommandEvent &event);
	void OnSpotifyPlaylistStateChangedEvent(wxCommandEvent &event);
	void OnSpotifySearchResultsEvent(wxCommandEvent &event);
	void OnSpotifyLoggedInEvent(wxCommandEvent &event);

	void OnTimerEvent(wxTimerEvent &event);

	void OnHotKeyPlayPause(wxKeyEvent &event);
	void OnHotKeyPrev(wxKeyEvent &event);
	void OnHotKeyNext(wxKeyEvent &event);

	void showLoginDialog();

	void playPause();
	bool next();
	bool prev();
	void highLightTrack(const Track *track);
	void playTrack(Track *const track);

	wxPanel *panel;

	SpotifyManager *spotifyManager;
	SoundManager *soundManager;
	AudioBuffer audioBuffer;

	wxTextCtrl *searchTextCtrl;
	wxTreeCtrl *playlistTree;
	SongListCtrl *songList;

	wxButton *buttonPlayPause;
	wxButton *buttonNext;
	wxButton *buttonPrev;
	ProgressIndicator *progressIndicator;

	wxStaticText *textCurrentProgressTime;
	wxStaticText *textTotalTime;

	wxCheckBox *checkBoxShuffle;

	wxFrame *milkDropFrame;

	wxTimer timerStatusUpdate;

	wxBitmap playImage;
	wxBitmap pauseImage;
	wxBitmap nextImage;
	wxBitmap prevImage;
	
	LoginDialog *loginDialog;

	Playlist *activePlaylist;
	unsigned int activeSongIndex;

	std::default_random_engine generator;

	DECLARE_EVENT_TABLE()
};

class Main : public wxApp
{
public:
	Main();
	~Main();

	virtual bool OnInit();

	MainFrame *mainFrame;
};



DECLARE_APP(Main)
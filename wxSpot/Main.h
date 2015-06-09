#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

#include <wx/config.h>

#include "AudioBuffer.h"

class SpotifyManager;
class SoundManager;

class wxListView;
class wxTreeCtrl;
class Playlist;
class LoginDialogue;


class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
	~MainFrame();


	SoundManager *getSoundManager() { return soundManager;  }
	AudioBuffer *getAudioBuffer() { return &audioBuffer;  }

	static wxConfig *config;
private:

	enum {
		ID_Settings = 1
	};

	void OnExit(wxCommandEvent &event);
	void OnSettings(wxCommandEvent &event);
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

	void showLoginDialog();

	void playPause();
	bool next();
	bool prev();

	wxPanel *panel;

	wxBoxSizer *horzBox;
	wxBoxSizer *vertBox;
	wxBoxSizer *bottomHorzBox;

	SpotifyManager *spotifyManager;
	SoundManager *soundManager;
	AudioBuffer audioBuffer;

	wxBoxSizer *searchPlaylistBox;

	wxTextCtrl *searchTextCtrl;
	wxTreeCtrl *playlistTree;
	wxListView *songList;

	wxButton *buttonPlayPause;
	wxButton *buttonNext;
	wxButton *buttonPrev;
	wxSlider *progressSlider;

	wxStaticText *textCurrentProgressTime;
	wxStaticText *textTotalTime;

	wxCheckBox *checkBoxShuffle;

	wxTimer timerStatusUpdate;

	wxBitmap playImage;
	wxBitmap pauseImage;
	wxBitmap nextImage;
	wxBitmap prevImage;
	
	LoginDialogue *loginDialogue;

	//Playlist *activePlaylist;
	int activeSongIndex;

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
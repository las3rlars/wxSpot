#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

#include <wx/config.h>
#include <wx/treectrl.h>
#include <random>

#include "AudioBuffer.h"
#include "DnDTrackDataObject.h"
#include "Plugin.h"

class SpotifyManager;
class SoundManager;

class SongListCtrl;
class wxTreeCtrl;
class Playlist;
class LoginDialog;
class ProgressIndicator;
class Spectrum;
class Track;

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
	~MainFrame();


	SoundManager *getSoundManager() { return soundManager;  }
	AudioBuffer *getAudioBuffer() { return &audioBuffer;  }

	void sendEvent(const wxEventType type, int cargo = -1);
#ifdef WIN32
	virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif
	static wxConfig *config;
private:
	enum {
		ID_HotKey_PlayPause = 1,
		ID_HotKey_Prev,
		ID_HotKey_Next,
		ID_HotKey_ShowCurrentTrack
	};

	enum {
		ID_Logout = 1,
		ID_Settings,
		ID_ShowSpectrum,
		ID_MilkDrop,
		ID_Menu_Add_To_Playlist,
		ID_Menu_Open_Artist,
		ID_Menu_Open_Album,
		ID_Menu_Delete_Track,
		ID_Menu_Copy_TrackName,
		ID_Menu_Copy_URI,
		ID_Menu_Copy_URL,
		ID_Menu_Last_Dont_Use,

		ID_Playlist_Add_Playlist,
		ID_Playlist_Rename,
		ID_Playlist_Delete,

		ID_Plugin

	};

	void loadPlugins();
	void unLoadPlugins();

	void OnExit(wxCommandEvent &event);
	void OnLogout(wxCommandEvent &event);
	void OnSettings(wxCommandEvent &event);
	void OnShowSpectrum(wxCommandEvent &event);
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
	void OnSpotifyPlayNextEvent(wxCommandEvent &event);
	void OnSpotifyMetaUpdatedEvent(wxCommandEvent &event);

	void OnPluginCloseEvent(wxCommandEvent &event);
	void OnPluginLoadedEvent(wxCommandEvent &event);

	void OnTimerEvent(wxTimerEvent &event);

	void OnHotKeyPlayPause(wxKeyEvent &event);
	void OnHotKeyPrev(wxKeyEvent &event);
	void OnHotKeyNext(wxKeyEvent &event);
	void OnHotKeyShowCurrentTrack(wxKeyEvent &event);

	void showLoginDialog();

	void playPause();
	bool next();
	bool prev();
	void highLightTrack(const Track *track);
	void playTrack(Track *const track);

	void resetPlaylistTree();
	void updatePlaylistTree();

	wxPanel *panel;

	SpotifyManager *spotifyManager;
	SoundManager *soundManager;
	AudioBuffer audioBuffer;

	wxTextCtrl *searchTextCtrl;
	wxTreeCtrl *playlistTree;
	SongListCtrl *songList;
	wxTreeItemId own;
	wxTreeItemId shared;

	wxButton *buttonPlayPause;
	wxButton *buttonNext;
	wxButton *buttonPrev;
	ProgressIndicator *progressIndicator;
	Spectrum *spectrum;

	wxStaticText *textCurrentProgressTime;
	wxStaticText *textTotalTime;

	wxImageList *imageList;

	wxCheckBox *checkBoxShuffle;

	wxTimer timerStatusUpdate;

	wxBitmap playImage;
	wxBitmap pauseImage;
	wxBitmap nextImage;
	wxBitmap prevImage;
	wxBitmap smallPlayImage;
	
	LoginDialog *loginDialog;

	Playlist *activePlaylist;
	unsigned int activeSongIndex;

	DnDTrackDataObject dnDDataObject;


	std::default_random_engine generator;
	std::vector<PluginInterface*> plugInterfaces;

	DECLARE_EVENT_TABLE()
};


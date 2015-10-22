#pragma once
#include <memory>
#include <wx/listctrl.h>

class Playlist;
class SpotifyManager;
class Track;

class SongListCtrl :
	public wxListCtrl
{
public:
	SongListCtrl(wxWindow *parent, SpotifyManager *spotifyManager);
	~SongListCtrl();

	wxString OnGetItemText(long item, long column) const;
	wxListItemAttr *OnGetItemAttr(long item) const;

	void setPlaylist(Playlist *playlist);
	Track *getTrack(long item);
private:
	SpotifyManager *spotifyManager;
	std::vector<std::unique_ptr<Track>> *tracks;

	wxListItemAttr even_unavailabe;
	wxListItemAttr even_available;
	wxListItemAttr odd_unavailable;
	wxListItemAttr odd_available;
};


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

	void SetSize(const wxRect &rect);
	void SetSize(int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO);

	wxString OnGetItemText(long item, long column) const;
	int OnGetItemImage(long item) const;
	wxListItemAttr *OnGetItemAttr(long item) const;

	void setPlaylist(Playlist *playlist);
	Track *getTrack(long item);

	void setHighLight(long item);
private:
	long highLighted;

	SpotifyManager *spotifyManager;
	std::vector<std::unique_ptr<Track>> *tracks;

	wxListItemAttr even_unavailabe;
	wxListItemAttr even_available;
	wxListItemAttr odd_unavailable;
	wxListItemAttr odd_available;
};


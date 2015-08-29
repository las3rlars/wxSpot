#pragma once

#include <vector>

#include <libspotify/api.h>
#include <wx/string.h>
#include <wx/treectrl.h>

#include "Track.h"


class Playlist
{
public:
	Playlist();
	virtual ~Playlist();

	void setTreeItemId(wxTreeItemId item) { treeItem = item; }
	wxTreeItemId getTreeItemId() { return treeItem; }

	void addTrack(sp_track *track);
	std::vector<Track*> *getTracks();
	void clearTracks();

private:
	wxTreeItemId treeItem;
	
	std::vector<Track*> tracks;
};

class SpotifyPlaylist : public Playlist
{
public:
	SpotifyPlaylist(sp_playlist *playlist);
	~SpotifyPlaylist();

	sp_playlist *getSpPlaylist();

	wxString getTitle() const;
	bool isShared();

private:
	sp_playlist *m_pPlaylist;

};
#pragma once

#include <vector>

#include <libspotify/api.h>
#include <wx/string.h>
#include <wx/treectrl.h>

#include "Track.h"


class Playlist
{
public:
	Playlist(sp_playlist *playlist);
	~Playlist();

	wxString getTitle();
	bool isShared();

	void setTreeItemId(wxTreeItemId item) { treeItem = item; }
	wxTreeItemId getTreeItemId() { return treeItem; }

	void addTrack(sp_track *track);
	std::vector<Track*> *getTracks();

	sp_playlist *getSpPlaylist();
private:
	sp_playlist *m_pPlaylist;
	wxTreeItemId treeItem;
	
	std::vector<Track*> tracks;
};


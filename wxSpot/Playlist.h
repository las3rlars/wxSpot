#pragma once

#include <vector>
#include <memory>

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
	wxTreeItemId getTreeItemId() const { return treeItem; }

	void addTrack(sp_track *track);
	virtual void removeTrack(const int index);
	std::vector<std::unique_ptr<Track>> *getTracks();
	void clearTracks();

protected:
	std::vector<std::unique_ptr<Track>> tracks;
private:
	wxTreeItemId treeItem;

};

class SpotifyPlaylist : public Playlist
{
public:
	SpotifyPlaylist(sp_playlist *playlist);
	~SpotifyPlaylist();

	sp_playlist *getSpPlaylist();

	void removeTrack(const int index) override;

	wxString getTitle() const;
	bool isShared();

private:
	sp_playlist *m_pPlaylist;

};
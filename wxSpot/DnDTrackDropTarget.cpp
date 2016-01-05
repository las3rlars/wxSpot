#include "DnDTrackDropTarget.h"

#include <wx/treectrl.h>
#include <wx/log.h>

#include "DnDTrackDataObject.h"
#include "Playlist.h"
#include "Track.h"
#include "SpotifyManager.h"

DnDTrackDropTarget::DnDTrackDropTarget(wxTreeCtrl *tree, SpotifyManager *spotifyManager) : wxDropTarget(new DnDTrackDataObject)
{
	this->tree = tree;
	this->spotifyManager = spotifyManager;
}


DnDTrackDropTarget::~DnDTrackDropTarget()
{
}

wxDragResult DnDTrackDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult defResult)
{
	if (!GetData()) {
		return wxDragNone;
	}

	wxTreeItemId item = tree->HitTest(wxPoint(x, y));


	if (!item.IsOk() || tree->ItemHasChildren(item)) {
		return wxDragNone;
	}

	DnDTrackDataObject *data = (DnDTrackDataObject*)GetDataObject();
	auto playlists = spotifyManager->getPlaylists();
	
	for (auto &playlist : *playlists) {
		if (playlist->getTreeItemId() == item) {

			for (size_t i = 0; i < data->getNumTracks(); i++) {
				playlist->addTrack(data->getTrack(i));
				spotifyManager->addTrackToPlaylist(data->getTrack(i), playlist);
			}
			break;
		}
	}

	wxLogDebug("%s num tracks: %d", tree->GetItemText(item), data->getNumTracks());
	return defResult;

}
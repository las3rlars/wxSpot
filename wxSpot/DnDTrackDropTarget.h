#pragma once
#include <wx/dnd.h>

class wxTreeCtrl;
class SpotifyManager;

class DnDTrackDropTarget :
	public wxDropTarget
{
public:
	DnDTrackDropTarget(wxTreeCtrl *tree, SpotifyManager *spotifyManager);
	~DnDTrackDropTarget();

	virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult deResult);
private:
	wxTreeCtrl *tree;
	SpotifyManager *spotifyManager;
};


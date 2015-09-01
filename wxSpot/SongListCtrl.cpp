#include "SongListCtrl.h"

#include "Playlist.h"
#include "SpotifyManager.h"

SongListCtrl::SongListCtrl(wxWindow *parent, SpotifyManager *spotifyManager) :
wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VIRTUAL),
	spotifyManager(spotifyManager)
{
	AppendColumn(wxString("Track"), wxLIST_FORMAT_LEFT, 280);
	AppendColumn(wxString("Artist"), wxLIST_FORMAT_LEFT);
	AppendColumn(wxString("Album"), wxLIST_FORMAT_LEFT);

	grey.SetTextColour(*wxLIGHT_GREY);
}


SongListCtrl::~SongListCtrl()
{
}

wxString SongListCtrl::OnGetItemText(long item, long column) const
{
	switch (column) {
	case 0:
		return tracks->at(item)->getTitle();
	case 1:
		return tracks->at(item)->getArtist();
	case 2:
		return tracks->at(item)->getAlbum();
	}

	return wxString("");
}

wxListItemAttr *SongListCtrl::OnGetItemAttr(long item) const
{
	if (spotifyManager->isTrackAvailable(tracks->at(item)) == false) {
		return (wxListItemAttr *)&grey;
	}

	return nullptr;
}

void SongListCtrl::setPlaylist(Playlist *playlist)
{
	Freeze();
	DeleteAllItems();
	SetClientData(playlist);
	tracks = playlist->getTracks();
	SetItemCount(tracks->size());
	Thaw();
}

Track *SongListCtrl::getTrack(long item)
{
	return tracks->at(item);
}
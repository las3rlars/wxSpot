#include "SongListCtrl.h"

#include "Playlist.h"
#include "SpotifyManager.h"

SongListCtrl::SongListCtrl(wxWindow *parent, SpotifyManager *spotifyManager) :
wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES),
	spotifyManager(spotifyManager)
{
	AppendColumn(wxString("Track"), wxLIST_FORMAT_LEFT, 280);
	AppendColumn(wxString("Artist"), wxLIST_FORMAT_LEFT, 170);
	AppendColumn(wxString("Duration"), wxLIST_FORMAT_RIGHT, 60);
	AppendColumn(wxString("Album"), wxLIST_FORMAT_LEFT, 170);

	const wxColour bgColour = GetBackgroundColour();

	// Depending on the background, alternate row color
	// will be 3% more dark or 50% brighter.
	int alpha = bgColour.GetRGB() > 0x808080 ? 97 : 150;

	const wxColor darkenedBg = bgColour.ChangeLightness(alpha);

	odd_unavailable.SetTextColour(*wxLIGHT_GREY);
	even_unavailabe.SetTextColour(*wxLIGHT_GREY);
	even_unavailabe.SetBackgroundColour(darkenedBg);
	even_available.SetBackgroundColour(darkenedBg);


}


SongListCtrl::~SongListCtrl()
{
}

wxString SongListCtrl::OnGetItemText(long item, long column) const
{
	if (tracks->at(item)->isLoaded()) {
		switch (column) {
		case 0:
			return tracks->at(item)->getTitle();
		case 1:
			return tracks->at(item)->getArtist();
		case 2: {
			unsigned int duration = tracks->at(item)->getDuration();
			return wxString::Format("%d:%02d", duration / 60000, (duration / 1000) % 60);
		}
		case 3:
			return tracks->at(item)->getAlbum();
		}
	}

	return wxString("Loading");
}

int SongListCtrl::OnGetItemImage(long item) const
{
	Playlist *playlist = (Playlist*)GetClientData();
	if (item == playlist->currentTrack) {
		return 0;
	}
	return -1;
}

wxListItemAttr *SongListCtrl::OnGetItemAttr(long item) const
{
	if (spotifyManager->isTrackAvailable(tracks->at(item).get()) == false) {
		if (item & 0x1) return (wxListItemAttr*)&even_unavailabe;
		return (wxListItemAttr *)&odd_unavailable;
	}

	if (item & 0x1) return (wxListItemAttr*)&even_available;
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

void SongListCtrl::setHighLight(long item)
{

	long old = highLighted;
	highLighted = item;
	this->RefreshItem(old);
	this->RefreshItem(highLighted);
}

Track *SongListCtrl::getTrack(long item)
{
	return tracks->at(item).get();
}
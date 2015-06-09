#pragma once

#include <libspotify/api.h>
#include <wx/string.h>


class Track
{
public:
	Track(sp_track *track);
	~Track();

	wxString getTitle();
	wxString getAlbum();
	wxString getArtist();

	sp_track *getSpTrack();

private:
	sp_track *m_pTrack;
	wxString title;
};


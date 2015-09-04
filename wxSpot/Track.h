#pragma once

#include <libspotify/api.h>
#include <wx/string.h>


class Track
{
public:
	Track(sp_track *track);
	~Track();

	wxString getLink() const;
	wxString getTitle() const;
	wxString getAlbum() const;
	wxString getArtist() const;
	bool isAvailable() const;

	sp_track *getSpTrack();

private:
	sp_track *m_pTrack;
};


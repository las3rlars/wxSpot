#pragma once

#include <libspotify/api.h>
#include <wx/string.h>


class Track
{
public:
	Track(sp_track *track);
	~Track();

	bool isLoaded();
	wxString getLink() const;
	wxString getTitle() const;
	wxString getAlbum() const;
	wxString getArtist() const;
	unsigned int getDuration() const;

	sp_track *getSpTrack();

private:
	sp_track *m_pTrack;
};


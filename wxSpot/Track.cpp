#include "Track.h"

#include <wx/log.h>
#include <wx/translation.h>

Track::Track(sp_track *track) : m_pTrack(track)
{
	sp_error error = sp_track_add_ref(m_pTrack);
	if (error != SP_ERROR_OK) {
		wxLogDebug("Error increasing ref count");
	}
}


Track::~Track()
{
	sp_track_release(m_pTrack);
}

bool Track::isLoaded()
{
	return sp_track_is_loaded(m_pTrack);
}

wxString Track::getLink() const
{
	char buffer[128];

	if (sp_track_is_loaded(m_pTrack)) {

		sp_error error = sp_track_error(m_pTrack);

		if (error == SP_ERROR_OK) {
			sp_link *link = sp_link_create_from_track(m_pTrack, 0);
			sp_link_as_string(link, buffer, 128);
			sp_link_release(link);

			return wxString::FromUTF8(buffer);
		}
	}

	return wxEmptyString;
}

wxString Track::getTitle() const
{
	sp_error error = sp_track_error(m_pTrack);

	if (error == SP_ERROR_OK) {
		return wxString::FromUTF8(sp_track_name(m_pTrack));
	}
	else if (error == SP_ERROR_IS_LOADING) {
		return wxString("Loading");
	}

	return wxString("Error");

	
}

wxString Track::getAlbum() const
{
	sp_album *album = sp_track_album(m_pTrack);
	if (sp_album_is_loaded(album)) {
		return wxString::FromUTF8(sp_album_name(album));
	}
	return wxString("Loading");
}

sp_link *Track::getAlbumLink()
{
	return sp_link_create_from_album(sp_track_album(m_pTrack));
}

wxString Track::getArtist() const
{
	wxString artists = "";
	for (int i = 0; i < sp_track_num_artists(m_pTrack); i++) {
		sp_artist *art = sp_track_artist(m_pTrack, i);

		artists.append(wxString::FromUTF8(sp_artist_name(art)));
		artists.append(", ");
	}
	artists.Truncate(artists.Length() - 2);
	return artists;
}

sp_link *Track::getArtistLink()
{
	return sp_link_create_from_artist(sp_track_artist(m_pTrack, 0));
}


unsigned int Track::getDuration() const
{
	return sp_track_duration(m_pTrack);
}

sp_track *Track::getSpTrack()
{
	return m_pTrack;
}


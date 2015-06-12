#include "Track.h"

#include <wx/translation.h>

Track::Track(sp_track *track)
{
	m_pTrack = (sp_track*)track;
	sp_track_add_ref(m_pTrack);
}


Track::~Track()
{
	sp_track_release(m_pTrack);
}

wxString Track::getLink()
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

	return wxString("");
}

wxString Track::getTitle()
{
	if (sp_track_is_loaded(m_pTrack)) {

		sp_error error = sp_track_error(m_pTrack);

		if (error == SP_ERROR_OK) {
			return wxString::FromUTF8(sp_track_name(m_pTrack));
		}
	}

	return wxString(_("Loading"));
}

wxString Track::getAlbum()
{
	return wxString("TODO");
}

wxString Track::getArtist()
{
	if (sp_track_is_loaded(m_pTrack)) {

		wxString artists = "";
		for (int i = 0; i < sp_track_num_artists(m_pTrack); i++) {
			sp_artist *art = sp_track_artist(m_pTrack, i);

			artists.append(wxString::FromUTF8(sp_artist_name(art)));
			artists.append(", ");
		}
		artists.Truncate(artists.Length() - 2);
		return artists;
	}

	return wxString(_("Loading"));
}

sp_track *Track::getSpTrack()
{
	return m_pTrack;
}


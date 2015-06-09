#include "Playlist.h"



Playlist::Playlist(sp_playlist *playlist)
{
	m_pPlaylist = playlist;
	sp_playlist_add_ref(playlist);
}



Playlist::~Playlist()
{
	for (unsigned int i = 0; i < tracks.size(); i++) {
		delete tracks.at(i);
	}

	tracks.clear();
	sp_playlist_release(m_pPlaylist);
}

wxString Playlist::getTitle()
{
	if (sp_playlist_is_loaded(m_pPlaylist)) {
		return wxString::FromUTF8(sp_playlist_name(m_pPlaylist));
	}
	return wxString(_("Loading"));
}

bool Playlist::isShared()
{
	return sp_playlist_is_collaborative(m_pPlaylist);
}

sp_playlist *Playlist::getSpPlaylist()
{
	return m_pPlaylist;
}

void Playlist::addTrack(sp_track *track)
{
	tracks.push_back(new Track(track));
}

std::vector<Track*> *Playlist::getTracks()
{
	return &tracks;
}
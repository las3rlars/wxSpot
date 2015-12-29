#include "Playlist.h"

#include <memory>


Playlist::Playlist() : currentTrack(-1)
{
}

Playlist::~Playlist()
{
	clearTracks();
}

void Playlist::clearTracks()
{
	//std::for_each(tracks.begin(), tracks.end(), std::default_delete<Track>());
	tracks.clear();
}

void Playlist::addTrack(sp_track *track)
{
	//tracks.push_back(new Track(track));
	tracks.push_back(std::make_unique<Track>(track));
}

void Playlist::removeTrack(const int index)
{
	//Track *track = tracks.at(index);
	//delete track;
	tracks.erase(tracks.begin() + index);
}

std::vector<std::unique_ptr<Track>> *Playlist::getTracks()
{
	return &tracks;
}

SpotifyPlaylist::SpotifyPlaylist(sp_playlist *playlist) : m_pPlaylist(playlist)
{
	sp_playlist_add_ref(playlist);
}

SpotifyPlaylist::~SpotifyPlaylist()
{
	sp_playlist_release(m_pPlaylist);
}

void SpotifyPlaylist::removeTrack(const int index)
{
	sp_playlist_remove_tracks(m_pPlaylist, &index, 1);
	Playlist::removeTrack(index);
}

wxString SpotifyPlaylist::getTitle() const
{
	if (sp_playlist_is_loaded(m_pPlaylist)) {
		return wxString::FromUTF8(sp_playlist_name(m_pPlaylist));
	}
	return wxString(_("Loading"));
}

bool SpotifyPlaylist::isShared()
{
	return sp_playlist_is_collaborative(m_pPlaylist);
}

sp_playlist *SpotifyPlaylist::getSpPlaylist()
{
	return m_pPlaylist;
}

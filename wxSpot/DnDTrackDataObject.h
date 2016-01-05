#pragma once
#include <wx/dnd.h>

#include "Track.h"

class DnDTrackDataObject :
	public wxDataObjectSimple
{
public:

	struct Data {
		size_t len;
		sp_track **tracks;
	};

	DnDTrackDataObject();
	virtual ~DnDTrackDataObject();

	virtual size_t GetDataSize() const;
	virtual bool GetDataHere(void *buf) const;
	virtual bool SetData(size_t len, const void *buf);
	//std::vector<sp_track*> getTracks() { return tracks; }
	//int getTracks() { return tracks; }
	//sp_track **getTracks() { return tracks;  }
	//size_t getNumTracks() { return (data.len - sizeof(data.len)) / sizeof(data.tracks); }
	size_t getNumTracks() { return data.len; }
	sp_track *getTrack(int i) {
		return data.tracks[i];
	}
private:


	Data data;
	//std::vector<sp_track*> tracks;
	//size_t len;
	//void *tracks;
	//int tracks;
	//sp_track **tracks;
};


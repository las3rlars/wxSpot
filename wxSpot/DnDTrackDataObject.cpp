#include "DnDTrackDataObject.h"
#include <wx/log.h>

DnDTrackDataObject::DnDTrackDataObject() : wxDataObjectSimple("wxSpot/track")
{
}


DnDTrackDataObject::~DnDTrackDataObject()
{
}

size_t DnDTrackDataObject::GetDataSize() const
{
	return sizeof(Data);
}

bool DnDTrackDataObject::GetDataHere(void *buf) const
{
	memcpy(buf, &data, sizeof(Data));
	return true;
}

bool DnDTrackDataObject::SetData(size_t len, const void *buf)
{
	memcpy(&data, buf, len);
	return true;
}
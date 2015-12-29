#pragma once
#include "wx/dataobj.h"

class DnDTrackDataObject :
	public wxDataObject
{
public:
	DnDTrackDataObject();
	~DnDTrackDataObject();
};


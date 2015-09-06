#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class Visualizer : public wxWindow
{
public:
	Visualizer(wxWindow *parent);
	~Visualizer();

	void OnPaint(wxPaintEvent &event);

	void paintNow();

	void update(short int *fftBuffer, unsigned int size, int scale);
private:

	wxWindow *m_pParent;
	void render(wxDC &dc);

	unsigned int width;
	unsigned int height;
	short int *buffer;
	int scale;

	DECLARE_EVENT_TABLE()

};


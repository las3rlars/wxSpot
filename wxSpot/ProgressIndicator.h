#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

wxDECLARE_EVENT(PI_SCROLL_CHANGED, wxCommandEvent);

class ProgressIndicator : public wxWindow
{
public:
	ProgressIndicator(wxWindow *parent);

	~ProgressIndicator();

	
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);

	void paintNow();

	void mouseDown(wxMouseEvent &event);

	void SetValue(float value);
	float GetValue() { return this->value; }

private:
	wxWindow *m_pParent;
	void render(wxDC &dc);

	unsigned int width;
	unsigned int height;
	float value;

	DECLARE_EVENT_TABLE()

};


#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

#include <wx/glcanvas.h>
#include <memory>

class projectM;

class MilkDropVisualizer : public wxGLCanvas
{

public:
	MilkDropVisualizer(wxFrame *parent, int *args);
	~MilkDropVisualizer();
	void updatePCM(short *data);

protected:
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);

private:
	wxGLContext *m_context;
	std::unique_ptr<projectM> m_projectM;
	//projectM *m_projectM;
	DECLARE_EVENT_TABLE()
};


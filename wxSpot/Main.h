#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class MainFrame;
class wxSingleInstanceChecker;

class Main : public wxApp
{
public:
	Main();
	~Main();

	virtual bool OnInit();
	virtual int OnExit();

	MainFrame *mainFrame;
	wxSingleInstanceChecker *singleInstance;
};



DECLARE_APP(Main)
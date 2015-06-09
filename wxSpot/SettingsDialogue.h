#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class wxStaticLine;

class SettingsDialogue : public wxDialog
{
public:
	SettingsDialogue();
	~SettingsDialogue();

private:
	wxStaticText *mPathText;
	wxTextCtrl *mPathTextCtrl;
	wxButton *mSelectDirButton;
	wxButton *mOkButton;
	wxButton *mCancelButton;

	wxStaticLine *mStaticLine;

};


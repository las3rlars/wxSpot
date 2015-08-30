#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class wxStaticLine;
class SoundManager;

class SettingsDialogue : public wxDialog
{
public:
	SettingsDialogue(SoundManager *soundManager);
	~SettingsDialogue();

private:
	SoundManager *m_soundManager;

	wxStaticText *mPathText;
	wxTextCtrl *mPathTextCtrl;
	wxButton *mSelectDirButton;
	wxChoice *mDeviceChoice;

	wxButton *mOkButton;
	wxButton *mCancelButton;

	wxStaticLine *mStaticLine;

};


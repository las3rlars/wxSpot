#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class wxStaticLine;
class wxDirPickerCtrl;
class SoundManager;

class SettingsDialog : public wxDialog
{
public:
	SettingsDialog(SoundManager *soundManager);
	~SettingsDialog();

private:

	wxStaticText* m_spotifyCachePathText;
	wxDirPickerCtrl* m_dirPicker;
	wxStaticText* m_outputDeviceText;
	wxChoice* m_deviceChoice;
	wxStaticLine* m_staticline1;
	wxButton* m_okButton;
	wxButton* m_cancelButton;

	SoundManager *m_soundManager;

};


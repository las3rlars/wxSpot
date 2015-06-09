#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class wxStaticLine;
class SpotifyManager;

class LoginDialogue : public wxDialog
{
public:
	LoginDialogue(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxString& title = _("Login"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxDEFAULT_DIALOG_STYLE);
	~LoginDialogue();

	void setSpotifyManager(SpotifyManager *spotifyManager);

protected:

	enum {
		ID_LOGO_BITMAP = 1000,
		ID_USERNAME_TEXT_CTRL,
		ID_PASSWORD_TEXT_CTRL,
		ID_AUTO_LOGIN_CHECK_BOX,
		ID_LOGIN_BUTTON
	};

	wxStaticText *mStaticText1;
	wxTextCtrl *mUsernameTextCtrl;
	wxStaticText *mStaticText2;
	wxTextCtrl *mPasswordTextCtrl;

	wxStaticLine *mStaticLine;

	wxCheckBox *mAutoLoginCheckBox;
	wxButton *mLoginButton;
	//ServerProxy *proxy;



private:
	SpotifyManager *spotifyManager;

	void OnLoginClicked(wxCommandEvent &event);
	void OnLoggedInEvent(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};


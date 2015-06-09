#include "LoginDialogue.h"

#include <wx/statline.h>
#include "SpotifyManager.h"

LoginDialogue::LoginDialogue(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer *bSizer = new wxBoxSizer(wxVERTICAL);

	mStaticText1 = new wxStaticText(this, wxID_ANY, _("Username"));
	mStaticText1->Wrap(-1);
	bSizer->Add(mStaticText1, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	mUsernameTextCtrl = new wxTextCtrl(this, ID_USERNAME_TEXT_CTRL);
	mUsernameTextCtrl->SetToolTip(_("username"));
	bSizer->Add(mUsernameTextCtrl, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);

	mStaticText2 = new wxStaticText(this, wxID_ANY, _("Password"));
	mStaticText2->Wrap(-1);
	bSizer->Add(mStaticText2, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	mPasswordTextCtrl = new wxTextCtrl(this, ID_PASSWORD_TEXT_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	mPasswordTextCtrl->SetToolTip(_("password"));

	bSizer->Add(mPasswordTextCtrl, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);


	mStaticLine = new wxStaticLine(this, wxID_ANY);
	bSizer->Add(mStaticLine, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer *bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	mAutoLoginCheckBox = new wxCheckBox(this, ID_AUTO_LOGIN_CHECK_BOX, _("Remember me"));

	bSizer2->Add(mAutoLoginCheckBox, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mLoginButton = new wxButton(this, ID_LOGIN_BUTTON, _("Login"));
	mLoginButton->SetDefault();

	bSizer2->Add(mLoginButton, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	bSizer->Add(bSizer2, wxEXPAND, 5);

	this->SetSizer(bSizer);
	this->Layout();
	bSizer->Fit(this);
}


LoginDialogue::~LoginDialogue()
{
}

void LoginDialogue::setSpotifyManager(SpotifyManager *manager)
{
	this->spotifyManager = manager;
}
void LoginDialogue::OnLoginClicked(wxCommandEvent &event)
{
	spotifyManager->login(mUsernameTextCtrl->GetValue(), mPasswordTextCtrl->GetValue());
}

void LoginDialogue::OnLoggedInEvent(wxCommandEvent &event)
{
	EndModal(wxID_OK);
	
}

BEGIN_EVENT_TABLE(LoginDialogue, wxDialog)
EVT_BUTTON(ID_LOGIN_BUTTON, LoginDialogue::OnLoginClicked)
EVT_COMMAND(wxID_ANY, SPOTIFY_LOGGED_IN_EVENT, LoginDialogue::OnLoggedInEvent)

END_EVENT_TABLE()
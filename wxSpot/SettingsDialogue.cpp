#include "SettingsDialogue.h"

#include <wx/statline.h>
#include "Main.h"
#include "SoundManager.h"
#include <wx/config.h>

SettingsDialogue::SettingsDialogue(SoundManager *soundManager) : wxDialog(nullptr, wxID_ANY, "Settings")
{
	this->m_soundManager = soundManager;
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	wxBoxSizer *bSizer = new wxBoxSizer(wxVERTICAL);

	mPathText = new wxStaticText(this, wxID_ANY, _("Cache path"));
	mPathText->Wrap(-1);
	bSizer->Add(mPathText, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxBoxSizer *vSizer = new wxBoxSizer(wxHORIZONTAL);


	mPathTextCtrl = new wxTextCtrl(this, wxID_ANY, MainFrame::config->Read("Path", ""), wxDefaultPosition, wxSize(300, -1));
	vSizer->Add(mPathTextCtrl, 1, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);

	mSelectDirButton = new wxButton(this, wxID_ANY, _("..."));
	mSelectDirButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		wxDirDialog *dirDialog = new wxDirDialog(this, "Select spotify local cache directory");
		
		if (dirDialog->ShowModal() == wxID_OK) {
			mPathTextCtrl->SetLabelText(dirDialog->GetPath());
		}

		delete dirDialog;
	});

	vSizer->Add(mSelectDirButton, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);



	bSizer->Add(vSizer);

	mStaticLine = new wxStaticLine(this, wxID_ANY);
	wxArrayString strings;
	std::vector<Device *> *devices = soundManager->getDevices();

	int deviceSelection = 0;
	int deviceIndex = 0;
	MainFrame::config->Read("DeviceIndex", &deviceIndex, 0);

	for (size_t i = 0; i < devices->size(); i++) {
		strings.Add(devices->at(i)->getName());
		if (devices->at(i)->getIndex() == deviceIndex) {
			deviceSelection = i;
		}
	}

	mDeviceChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, strings);
	mDeviceChoice->SetSelection(deviceSelection);
	bSizer->Add(mDeviceChoice);
	bSizer->Add(mStaticLine, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer *hSizer2 = new wxBoxSizer(wxHORIZONTAL);

	mOkButton = new wxButton(this, wxID_OK, _("OK"));
	mOkButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		MainFrame::config->Write("Path", mPathTextCtrl->GetValue());
		MainFrame::config->Write("DeviceIndex", devices->at(mDeviceChoice->GetSelection())->getIndex());
		EndModal(wxID_OK);
	});

	mCancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));
	mCancelButton->SetDefault();

	hSizer2->Add(mOkButton);
	hSizer2->Add(mCancelButton);

	bSizer->Add(hSizer2, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);


	this->SetSizer(bSizer);
	this->Layout();
	bSizer->Fit(this);


}


SettingsDialogue::~SettingsDialogue()
{
}

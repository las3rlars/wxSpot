#include "SettingsDialog.h"

#include <wx/statline.h>
#include <wx/filepicker.h>
#include "MainFrame.h"
#include "SoundManager.h"
#include <wx/config.h>

SettingsDialog::SettingsDialog(SoundManager *soundManager) : wxDialog(nullptr, wxID_ANY, "Settings", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP)
{
	this->m_soundManager = soundManager;
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	bSizer1->SetMinSize(wxSize(400, 100));
	m_spotifyCachePathText = new wxStaticText(this, wxID_ANY, wxT("Spotify cache path"), wxDefaultPosition, wxDefaultSize, 0);
	m_spotifyCachePathText->Wrap(-1);
	bSizer1->Add(m_spotifyCachePathText, 0, wxALL, 5);

	m_dirPicker = new wxDirPickerCtrl(this, wxID_ANY, MainFrame::config->Read("Path", wxEmptyString), wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE);
	bSizer1->Add(m_dirPicker, 0, wxALL | wxEXPAND, 5);

	m_outputDeviceText = new wxStaticText(this, wxID_ANY, wxT("Output device"), wxDefaultPosition, wxDefaultSize, 0);
	m_outputDeviceText->Wrap(-1);
	bSizer1->Add(m_outputDeviceText, 0, wxALL, 5);

	wxArrayString m_deviceChoiceChoices;

	int deviceChoice = 0;
	int deviceIndex = 0;
	MainFrame::config->Read("DeviceIndex", &deviceIndex, 0);

	auto devices = soundManager->getDevices();
	for (size_t i = 0; i < devices->size(); i++) {
		m_deviceChoiceChoices.Add(devices->at(i)->getName());
		if (devices->at(i)->getIndex() == deviceIndex) {
			deviceChoice = i;
		}
	}

	m_deviceChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_deviceChoiceChoices, 0);
	m_deviceChoice->SetSelection(deviceChoice);
	
	bSizer1->Add(m_deviceChoice, 0, wxALL | wxEXPAND, 5);

	m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);

	m_okButton = new wxButton(this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
	m_okButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent &event) {
		MainFrame::config->Write("Path", m_dirPicker->GetPath());
		MainFrame::config->Write("DeviceIndex", devices->at(m_deviceChoice->GetSelection())->getIndex());
		if (deviceIndex != devices->at(m_deviceChoice->GetSelection())->getIndex()) {
			soundManager->end();
			soundManager->init(devices->at(m_deviceChoice->GetSelection())->getIndex());
		}
		EndModal(wxID_OK);
	});
	bSizer4->Add(m_okButton, 0, wxALL, 5);

	m_cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(m_cancelButton, 0, wxALL, 5);


	bSizer1->Add(bSizer4, 0, wxALIGN_RIGHT, 5);


	this->SetSizer(bSizer1);
	bSizer1->Fit(this);
	this->Layout();
	this->Centre(wxBOTH);
}


SettingsDialog::~SettingsDialog()
{
}

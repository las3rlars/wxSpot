#include "Main.h"

#include <wx/snglinst.h>
#include "MainFrame.h"

IMPLEMENT_APP(Main)


Main::Main()
{
}


Main::~Main()
{
}

bool Main::OnInit()
{
	if (!wxApp::OnInit()) {
		return false;
	}

	singleInstance = new wxSingleInstanceChecker();

	if (singleInstance->IsAnotherRunning()) {
		wxLogError("wxSpot already running, aborting.");
		delete singleInstance;
		singleInstance = nullptr;
		return false;
	}
	//_CrtSetBreakAlloc(9074);

	mainFrame = new MainFrame(_("wxSpot"), wxDefaultPosition, wxSize(1024, 768));

	mainFrame->Show();

	SetTopWindow(mainFrame);

	return true;
}

int Main::OnExit()
{
	delete singleInstance;
	return 0;
}
#include "ProjectMPlugin.h"

#include <wx/window.h>
#include <wx/glcanvas.h>
#include <wx/frame.h>
#include <wx/thread.h>
#include <wx/dynlib.h>

#include <process.h>

#include "MilkDropVisualizer.h"

IMPLEMENT_APP_NO_MAIN(ProjectMPluginApp)

wxDEFINE_EVENT(PLUGIN_CLOSE_EVENT, wxCommandEvent);
wxDEFINE_EVENT(PLUGIN_LOADED_EVENT, wxCommandEvent);

wxCriticalSection gs_wxStartupCS;
HANDLE gs_wxMainThread = nullptr;

static const int CMD_SHOW_WINDOW = wxNewId();
static const int CMD_TERMINATE = wxNewId();


unsigned wxSTDCALL ProjectMPluginAppLauncher(void *event)
{
	const HINSTANCE hInstance = wxDynamicLibrary::MSWGetModuleHandle
		("projectMPlugin", &gs_wxMainThread);

	if (!hInstance)
		return 0;

	wxDISABLE_DEBUG_SUPPORT();

	wxInitializer wxInit;
	if (!wxInit.IsOk())
		return 0;

	HANDLE hEvent = *(static_cast<HANDLE*>(event));

	if (!SetEvent(hEvent))
		return 0;

	wxEntry(hInstance);

	return 1;
}

extern "C"
{

	void start(wxWindow *parent)
	{
		wxCriticalSectionLocker lock(gs_wxStartupCS);
		if (!gs_wxMainThread) {
			HANDLE hEvent = CreateEvent
				(
				NULL,
				FALSE,
				FALSE,
				NULL
				);

			if (!hEvent) {
				return;
			}

			gs_wxMainThread = (HANDLE)_beginthreadex
				(
				NULL,
				0,
				&ProjectMPluginAppLauncher,
				&hEvent,
				0,
				NULL
				);

			if (!gs_wxMainThread) {
				CloseHandle(hEvent);
				return;
			}

			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}


		wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_SHOW_WINDOW);
		event->SetString("projectMPlugin");
		event->SetPayload(parent);
		wxQueueEvent(wxApp::GetInstance(), event);
	}

	void stop()
	{
		wxCriticalSectionLocker lock(gs_wxStartupCS);

		if (!gs_wxMainThread) {
			return;
		}

		wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_TERMINATE);
		wxQueueEvent(wxApp::GetInstance(), event);

		WaitForSingleObject(gs_wxMainThread, INFINITE);
		CloseHandle(gs_wxMainThread);
		gs_wxMainThread = NULL;
	}

	const char *getName()
	{
		return "projectM";
	}

	Plugin *getPlugin()
	{
		return static_cast<ProjectMPluginApp*>(wxApp::GetInstance());
	}
}

ProjectMPluginApp::ProjectMPluginApp()
{
	SetExitOnFrameDelete(false);

	Connect(CMD_SHOW_WINDOW,
		wxEVT_THREAD,
		wxThreadEventHandler(ProjectMPluginApp::OnShowWindow));
	Connect(CMD_TERMINATE,
		wxEVT_THREAD,
		wxThreadEventHandler(ProjectMPluginApp::OnTerminate));
}
void ProjectMPluginApp::OnShowWindow(wxThreadEvent &event)
{
	wxWindow *parent = event.GetPayload<wxWindow*>();
	start(parent);
}

void ProjectMPluginApp::OnTerminate(wxThreadEvent &event)
{
	ExitMainLoop();
}


/*ProjectMPlugin::ProjectMPlugin() : milkDropFrame(nullptr), milkDropVisualizer(nullptr)
{

}*/

wxWindow *ProjectMPluginApp::start(wxWindow *parent)
{
	// Only allow one window at the time
	//if (milkDropFrame != nullptr) return;
	this->parent = parent;
	milkDropFrame = new wxFrame(nullptr, wxID_ANY, wxString("wxSpot - MilkDrop"), wxDefaultPosition, wxSize(512, 288));


	int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 0, 0 };

	milkDropFrame->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent &event) {
		wxThreadEvent *terminateEvent = new wxThreadEvent(PLUGIN_CLOSE_EVENT, wxID_ANY);
		wxQueueEvent(parent, terminateEvent);
		event.Skip();
	});

	milkDropVisualizer = new MilkDropVisualizer(milkDropFrame, args);

	milkDropVisualizer->Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent &event) {
		if (event.GetModifiers() == wxMOD_ALT && event.GetKeyCode() == WXK_RETURN) {
			milkDropFrame->ShowFullScreen(!milkDropFrame->IsFullScreen());
		}
		else if (event.GetKeyCode() == WXK_SPACE) {
			//next();
		}
	});
	SetTopWindow(milkDropFrame);
	milkDropFrame->Show();
	wxThreadEvent *terminateEvent = new wxThreadEvent(PLUGIN_LOADED_EVENT, wxID_ANY);
	wxQueueEvent(parent, terminateEvent);

	return milkDropFrame;

	//plugin->start(NULL);
}

void ProjectMPluginApp::updatePCM(short *data)
{
	if (milkDropVisualizer)
		milkDropVisualizer->updatePCM(data);
}
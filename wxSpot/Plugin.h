#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

#include <wx/dynlib.h>

/*#if defined(__WIN32__)
#define IMPLEMENT_PLUGIN(name) extern "C" __declspec(dllexport) Plugin* CreatePlugin() { return new name();};
#else
#define IMPLEMENT_PLUGIN(name) extern "C" Plugin* CreatePlugin() { return new name();};
#endif
*/
class wxDynamicLibrary;
class MainFrame;

wxDECLARE_EVENT(PLUGIN_CLOSE_EVENT, wxCommandEvent);
wxDECLARE_EVENT(PLUGIN_LOADED_EVENT, wxCommandEvent);

class Plugin : public wxApp
{
public:
	enum SUPPORT_FLAGS {
		PCM_DATA = 1,
		WINDOW = 2
	};

	virtual void updatePCM(short *data) {};

};

extern "C" {
	__declspec(dllexport) void start(wxWindow *parent);
	__declspec(dllexport) void stop();
	__declspec(dllexport) const char *getName();
	__declspec(dllexport) Plugin *getPlugin();
}


#ifndef __PLUGIN_FUNCTION
#define __PLUGIN_FUNCTION
typedef void (*Start_function)(wxWindow *parent);
typedef const char* (*GetName_function)();
typedef void(*Stop_function)();
typedef Plugin* (*GetPlugin_function)();

struct PluginInterface {
	Start_function start;
	Stop_function stop;
	GetName_function getName;
	GetPlugin_function getPlugin;
	wxDllType dll;
	//wxDynamicLibrary *dll;
};
#endif //__PLUGIN_FUNCTION

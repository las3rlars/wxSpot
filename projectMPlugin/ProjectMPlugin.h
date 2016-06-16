#pragma once
#include "../wxSpot/Plugin.h"

class MilkDropVisualizer;
class SoundManager;
class ProjectMPlugin;
class MainFrame;

class ProjectMPluginApp : public Plugin
{
public:
	ProjectMPluginApp();
	//ProjectMPlugin *plugin;

private:
	wxWindow *start(wxWindow *parent);

	void OnShowWindow(wxThreadEvent &event);
	void OnTerminate(wxThreadEvent &event);

	virtual void updatePCM(short *data);

	wxWindow *parent;
	wxFrame *milkDropFrame;
	MilkDropVisualizer *milkDropVisualizer;


};



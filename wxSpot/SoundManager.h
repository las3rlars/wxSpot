#pragma once

#include <wx/string.h>
#include <memory>
#include <vector>
#include "portaudio.h"


class Plugin;
class MainFrame;

class Device
{
public:
	Device(PaDeviceIndex paDeviceIndex);
	~Device();
	wxString getName() const;
	int getIndex() { return m_paDeviceIndex; }
private:
	PaDeviceIndex m_paDeviceIndex;
};

class SoundManager
{
public:
	SoundManager(MainFrame *mainFrame);
	~SoundManager();

	void init(int deviceIndex);
	bool deviceSupported(int deviceIndex);
	void end();
	void play();
	void stop();
	unsigned int getSoundData(void *data, int num_frames);
	void bufferDone();
	
	//void setMilkDropVisualizer(MilkDropVisualizer *visualizer);
	void setPCMPlugin(Plugin *plugin);
	//std::vector<Device *> *getDevices();
	std::vector<std::shared_ptr<Device>> *getDevices();

private:
	PaStreamParameters streamParams;

	MainFrame *m_pMainFrame;
	PaStream *m_pStream;

	std::vector<std::shared_ptr<Device>> devices;

	//MilkDropVisualizer *m_milkDropVisualizer;
	Plugin *plugin;
};


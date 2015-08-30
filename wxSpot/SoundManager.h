#pragma once

#include "Main.h"
#include "portaudio.h"

class Device
{
public:
	Device(PaDeviceIndex paDeviceIndex);
	~Device();
	wxString getName() const;
private:
	PaDeviceIndex m_paDeviceIndex;
};

class SoundManager
{
public:
	SoundManager(MainFrame *mainFrame);
	~SoundManager();

	void init();
	void end();
	void play();
	void stop();
	unsigned int getSoundData(void *data, int num_frames);
	void bufferDone();
	
	std::vector<Device *> *getDevices();

private:
	MainFrame *m_pMainFrame;
	PaStream *m_pStream;

	std::vector<Device *> devices;
};


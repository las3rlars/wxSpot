#pragma once

#include "Main.h"
#include "portaudio.h"

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
	
	std::vector<Device *> *getDevices();

private:
	PaStreamParameters streamParams;

	MainFrame *m_pMainFrame;
	PaStream *m_pStream;

	std::vector<Device *> devices;
};


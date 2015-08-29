#pragma once

#include "Main.h"
#include "portaudio.h"

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
private:
	MainFrame *m_pMainFrame;
	PaStream *m_pStream;
};


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
	void getSoundData(void *data, int num_frames);
private:
	MainFrame *m_pMainFrame;
	PaStream *m_pStream;
};


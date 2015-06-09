#include "SoundManager.h"

#include "AudioBuffer.h"

#include <iostream>

#define SAMPLE_RATE (44100)


static SoundManager *GetManagerFromUserdata(void *userData)
{
	SoundManager *manager = reinterpret_cast<SoundManager*>(userData);
	return manager;
}

static int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	SoundManager *manager = GetManagerFromUserdata(userData);

	manager->getSoundData(output, frameCount);

	return paContinue;
	
}

SoundManager::SoundManager(MainFrame *mainFrame)
{
	m_pMainFrame = mainFrame;
	PaError error = Pa_Initialize();

	if (error != paNoError) {
		std::cerr << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
	}

}


SoundManager::~SoundManager()
{
	PaError error = Pa_Terminate();

	if (error != paNoError) {
		std::cerr << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
	}

}

void SoundManager::init()
{
	PaError error;


	error = Pa_OpenDefaultStream(&m_pStream, 0, 2, paInt16, SAMPLE_RATE, 512, paCallback, this);

	if (error != paNoError) {
		std::cerr << "PortAdio error: " << Pa_GetErrorText(error) << std::endl;
	}
}

void SoundManager::end()
{
	Pa_CloseStream(m_pStream);
}

void SoundManager::play()
{
	Pa_StartStream(m_pStream);
}

void SoundManager::stop()
{
	Pa_StopStream(m_pStream);
}

void SoundManager::getSoundData(void *frames, int num_frames)
{
	AudioBuffer *buffer = m_pMainFrame->getAudioBuffer();

	buffer->readData((int16_t *)frames, num_frames * 2);

}
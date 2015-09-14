#include "SoundManager.h"

#include "AudioBuffer.h"
#include "MilkDropVisualizer.h"

#define SAMPLE_RATE (44100)


static SoundManager *GetManagerFromUserdata(void *userData)
{
	SoundManager *manager = reinterpret_cast<SoundManager*>(userData);
	return manager;
}

static int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	SoundManager *manager = GetManagerFromUserdata(userData);

	unsigned int samples = manager->getSoundData(output, frameCount * 2);

	if (samples < frameCount * 2) {
		manager->bufferDone();
		return paComplete;
	}
	return paContinue;
	
}

SoundManager::SoundManager(MainFrame *mainFrame) : m_milkDropVisualizer(nullptr)
{
	m_pMainFrame = mainFrame;
	PaError error = Pa_Initialize();

	if (error != paNoError) {
		wxLogError("PortAudio Error: %s", Pa_GetErrorText(error));
	}


	streamParams.channelCount = 2;
	streamParams.sampleFormat = paInt16;
	streamParams.suggestedLatency = 0.05;
	streamParams.hostApiSpecificStreamInfo = nullptr;

	PaDeviceIndex deviceCount = Pa_GetDeviceCount();
	PaDeviceIndex defaultDevice = Pa_GetDefaultOutputDevice();
	wxLogDebug("Default output device: %d", defaultDevice);
	for (PaDeviceIndex i = 0; i < deviceCount; i++) {
		if (deviceSupported(i)) {
			devices.push_back(std::make_shared<Device>(i));
		}
	}
}


SoundManager::~SoundManager()
{
	PaError error = Pa_Terminate();

	if (error != paNoError) {
		wxLogError("PortAudio error: %s", Pa_GetErrorText(error));
	}

}

void SoundManager::init(int deviceIndex)
{
	streamParams.device = deviceIndex;

	PaError error;

	//error = Pa_OpenDefaultStream(&m_pStream, 0, 2, paInt16, SAMPLE_RATE, 512, paCallback, this);
	error = Pa_OpenStream(&m_pStream, nullptr, &streamParams, SAMPLE_RATE, 512, paNoFlag, paCallback,this);

	if (error != paNoError) {
		wxLogError("PortAudio Error: %s", Pa_GetErrorText(error));
	}
}

bool SoundManager::deviceSupported(int deviceIndex)
{
	streamParams.device = deviceIndex;

	if (Pa_IsFormatSupported(nullptr, &streamParams, 44100.0) == paFormatIsSupported) {
		return true;
	}

	return false;
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

unsigned int SoundManager::getSoundData(void *frames, int num_frames)
{
	AudioBuffer *buffer = m_pMainFrame->getAudioBuffer();

	int len = buffer->readData((int16_t *)frames, num_frames);
	
	
	//return buffer->readData((int16_t *)frames, num_frames);
	if (m_milkDropVisualizer != nullptr)
		m_milkDropVisualizer->updatePCM((int16_t *)frames);

	return len;

}

void SoundManager::bufferDone()
{
	m_pMainFrame->bufferDone();
}

void SoundManager::setMilkDropVisualizer(MilkDropVisualizer *visualizer)
{
	m_milkDropVisualizer = visualizer;
}

std::vector<std::shared_ptr<Device>> *SoundManager::getDevices()
{
	return &devices;
}

Device::Device(PaDeviceIndex paDeviceIndex)
{
	m_paDeviceIndex = paDeviceIndex;
}

Device::~Device()
{

}

wxString Device::getName() const
{
	const PaDeviceInfo *info = Pa_GetDeviceInfo(m_paDeviceIndex);
	return wxString::FromUTF8(info->name);
}
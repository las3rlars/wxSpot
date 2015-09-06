#include "AudioBuffer.h"

#include <wx/log.h>
#include <algorithm> // std::min
#include "fix_fft.h"
#include "Visualizer.h"

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BUFFER_TIME 1
#define BUFFER_SIZE SAMPLE_RATE * CHANNELS * BUFFER_TIME

AudioBuffer::AudioBuffer()
{
	m_visualizer = nullptr;
	currentBuffer = getBufferFromPool();
	queuedBuffers.push_back(currentBuffer);
	reset();
}


AudioBuffer::~AudioBuffer()
{
}

void AudioBuffer::setVisualizer(Visualizer *visualizer)
{
	m_visualizer = visualizer;
}
std::shared_ptr<int16_t> AudioBuffer::getBufferFromPool()
{
	if (bufferPool.empty()) {
		bufferPool.push_front(std::shared_ptr<int16_t>(new int16_t[BUFFER_SIZE], [](int16_t *p) { delete[] p; }));
	}

	auto front = bufferPool.front();
	bufferPool.pop_front();
	return front;
}

void AudioBuffer::addData(const int16_t *data, const unsigned int samples)
{
	if (samples == 0) {
		readOffset = 0;
		writeOffset = 0;
		stutter = 0;
		writeBufferOffset = 0;
		readBufferOffset = 0;
		queuedBuffers.clear();
		currentBuffer = getBufferFromPool();
		queuedBuffers.push_back(currentBuffer);
		return;
	}

	unsigned int samplesLeft = samples;
	while (samplesLeft > 0) {

		if (writeBufferOffset == BUFFER_SIZE) {
			currentBuffer = getBufferFromPool();

			queuedBuffers.push_back(currentBuffer);
			writeBufferOffset = 0;
		}
		int dataSize = std::min(samplesLeft, (unsigned int)BUFFER_SIZE - writeBufferOffset);

		memcpy(currentBuffer.get() + writeBufferOffset, data + (samples - samplesLeft), dataSize * 2);
		samplesLeft -= dataSize;
		writeOffset += dataSize;
		writeBufferOffset += dataSize;

	}
}

int AudioBuffer::readData(int16_t *dest, const unsigned int samples)
{
	unsigned int samplesLeft = samples;

	if (readOffset + samples >= writeOffset) {
		stutter++;
		readOffset = 0;
	}

	while(samplesLeft > 0) {
		if (readBufferOffset == BUFFER_SIZE) {
			bufferPool.push_front(currentBuffer);
			queuedBuffers.pop_front();
			readBufferOffset = 0;
		}
		if (queuedBuffers.empty())
			return 0;

		auto buffer = queuedBuffers.front();

		unsigned int dataSize = std::min(samplesLeft, BUFFER_SIZE - readBufferOffset);

		memcpy(dest + (samples - samplesLeft), buffer.get() + readBufferOffset, dataSize * 2);
		samplesLeft -= dataSize;
		readBufferOffset += dataSize;

	}
	readOffset += samples;
	currentFrame += samples;
	updateFFT(dest, samples);
	return samples;
}

int AudioBuffer::getSampleDiff()
{
	int temp = writeOffset - readOffset;

	if (temp < 0) return 0;
	return temp;

}

int AudioBuffer::getStutter()
{
	unsigned int temp = stutter;
	stutter = 0;
	return temp;
}

unsigned int AudioBuffer::getPlayTime()
{
	return currentFrame / ((44100 * 2) / 1000);
}

void AudioBuffer::setPlayTime(unsigned int time)
{
	unsigned int newPlayedFrame = ((44100 * 2) / 1000) * time;
	

	if (newPlayedFrame < readOffset) {
		writeOffset = 0;
	}

	currentFrame = newPlayedFrame;
}

void AudioBuffer::reset()
{
	wxLogDebug("Resetting audio buffer with readOffset: %d writeOffset: %d", readOffset, writeOffset);

	readOffset = 0;
	writeOffset = 0;
	stutter = 1;
	currentFrame = 0;
	readBufferOffset = 0;
	writeBufferOffset = 0;
	
	// start fresh!
	while (!queuedBuffers.empty()) {
		bufferPool.push_front(queuedBuffers.front());
		queuedBuffers.pop_front();
	}
	currentBuffer = getBufferFromPool();
	queuedBuffers.push_back(currentBuffer);

}

void AudioBuffer::updateFFT(int16_t *buffer, unsigned int size)
{
	//memcpy(fftBuffer, buffer, size);
	short avg[512];
	int half = size / 2;

	// make mono
	for (int i = 0, j = 0; i < half; i++, j += 2) {
		avg[i] = (buffer[j] + buffer[j + 1]) / 2;
	}

	for (int i = 0; i < half; i++) {
		if (i & 0x01)
			fftBuffer[(half + i) >> 1] = avg[i];
		else
			fftBuffer[i >> 1] = avg[i];
	}

	// log2(512) = 9
	int scale = fix_fftr(fftBuffer, 9, 1);

	m_visualizer->update(fftBuffer, half, scale);
	//wxLogDebug("%+.3f %+.3f %+.3f %+.3f", fftBuffer[255] / 32767.0, fftBuffer[511] / 32767.0, fftBuffer[767] / 32767.0, fftBuffer[1023] / 32767.0);
}
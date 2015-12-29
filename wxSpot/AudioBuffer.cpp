#include "AudioBuffer.h"

#include <wx/log.h>
#include <algorithm> // std::min
//#include <cmath>

#define SAMPLE_RATE 44100
#define CHANNELS 2 // stereo
#define BUFFER_TIME 1 // one second
#define BUFFER_SIZE (SAMPLE_RATE * CHANNELS * BUFFER_TIME) / 2 // make it half a second.

#define MUTEX 1

AudioBuffer::AudioBuffer()
{
	currentBuffer = getBufferFromPool();
	queuedBuffers.push_back(currentBuffer);
	reset();
}


AudioBuffer::~AudioBuffer()
{
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

int AudioBuffer::addData(const int16_t *data, const unsigned int samples)
{
	mutex.lock();
	// If samples is 0 - there has been a seek, Reset stuff
	if (samples == 0) {
		readOffset = 0;
		writeOffset = 0;
		writeBufferOffset = 0;
		readBufferOffset = 0;
		queuedBuffers.clear();
		currentBuffer = getBufferFromPool();
		queuedBuffers.push_back(currentBuffer);
		mutex.unlock();
		return 0;
	}

	if (queuedBuffers.size() == 3) {
		mutex.unlock();
		return 0;
	}
	int samplesLeft = samples;

	while (samplesLeft > 0) {

		if (writeBufferOffset == BUFFER_SIZE) {
			currentBuffer = getBufferFromPool();

			queuedBuffers.push_back(currentBuffer);
			writeBufferOffset = 0;
		}
		int dataSize = std::min(samplesLeft, BUFFER_SIZE - writeBufferOffset);

		memcpy(currentBuffer.get() + writeBufferOffset, data + (samples - samplesLeft), dataSize * 2);
		samplesLeft -= dataSize;
		writeBufferOffset += dataSize;
	}
	writeOffset += samples;

	mutex.unlock();
	return samples;
}

int AudioBuffer::readData(int16_t *dest, const unsigned int samples)
{
	int totSamples = samples;
	int samplesLeft = samples;
	mutex.lock();
	// Are we trying to read more data than we have queued?
	if (queuedBuffers.size() <= 1) {
		memset(dest, 0, samples * sizeof(uint16_t));
		mutex.unlock();
		if (m_flush) {
			m_flush = false;
			return -1;
		}
		return samples;
	}

	while(samplesLeft > 0) {
		if (readBufferOffset == BUFFER_SIZE) {
			auto buffer = queuedBuffers.front();
			bufferPool.push_back(buffer);
			queuedBuffers.pop_front();
			readBufferOffset = 0;
		}

		assert(!queuedBuffers.empty());

		if (queuedBuffers.size() == 0) {
			mutex.unlock();
			return -1;
		}

		auto buffer = queuedBuffers.front();

		//assert(buffer.get() != currentBuffer.get());

		unsigned int dataSize = std::min(samplesLeft, BUFFER_SIZE - readBufferOffset);

		memcpy(dest + (samples - samplesLeft), buffer.get() + readBufferOffset, dataSize * 2);
		samplesLeft -= dataSize;
		readBufferOffset += dataSize;

	}
	readOffset += samples;
	currentFrame += samples;
	mutex.unlock();
	return totSamples;
}

unsigned int AudioBuffer::getPlayTime()
{
	return currentFrame / ((SAMPLE_RATE * CHANNELS) / 1000);
}

void AudioBuffer::setPlayTime(unsigned int time)
{
	unsigned int newPlayedFrame = ((SAMPLE_RATE * CHANNELS) / 1000) * time;

	mutex.lock();

	if (newPlayedFrame < readOffset) {
		readBufferOffset = 0;
		writeBufferOffset = 0;
		writeOffset = 0;
	}

	while (!queuedBuffers.empty() && queuedBuffers.front() != currentBuffer) {
		bufferPool.push_front(queuedBuffers.front());
		queuedBuffers.pop_front();
	}


	currentFrame = newPlayedFrame;

	mutex.unlock();
}

void AudioBuffer::reset()
{
	wxLogDebug("Resetting audio buffer with readOffset: %d writeOffset: %d", readOffset, writeOffset);

	mutex.lock();

	readOffset = 0;
	writeOffset = 0;
	m_flush = false;
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

	mutex.unlock();

}

void AudioBuffer::flush()
{
	mutex.lock();
	m_flush = true;
	mutex.unlock();
}
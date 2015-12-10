#include "AudioBuffer.h"

#include <wx/log.h>
#include <algorithm> // std::min
//#include <cmath>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BUFFER_TIME 1
#define BUFFER_SIZE SAMPLE_RATE * CHANNELS * BUFFER_TIME

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

void AudioBuffer::addData(const int16_t *data, const unsigned int samples)
{
#ifdef MUTEX
	mutex.lock();
#endif
	if (samples == 0) {
		readOffset = 0;
		writeOffset = 0;
		stutter = 0;
		writeBufferOffset = 0;
		readBufferOffset = 0;
		queuedBuffers.clear();
		currentBuffer = getBufferFromPool();
		queuedBuffers.push_back(currentBuffer);
#ifdef MUTEX
		mutex.unlock();
#endif
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
#ifdef MUTEX
	mutex.unlock();
#endif
}

int AudioBuffer::readData(int16_t *dest, const unsigned int samples)
{
	unsigned int samplesLeft = samples;
#ifdef MUTEX
	mutex.lock();
#endif
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
		if (queuedBuffers.empty()) {
#ifdef MUTEX
			mutex.unlock();
#endif
			return 0;
		}

		auto buffer = queuedBuffers.front();

		unsigned int dataSize = std::min(samplesLeft, BUFFER_SIZE - readBufferOffset);

		memcpy(dest + (samples - samplesLeft), buffer.get() + readBufferOffset, dataSize * 2);
		samplesLeft -= dataSize;
		readBufferOffset += dataSize;

	}
	readOffset += samples;
	currentFrame += samples;
#ifdef MUTEX
	mutex.unlock();
#endif
	return samples;
}

void AudioBuffer::getBufferStatus(int *stutter, int *sampleDiff)
{
	mutex.lock();
	*stutter = this->stutter;
	this->stutter = 0;

	int temp = writeOffset - readOffset;
	if (temp < 0)
		*sampleDiff = 0;
	else
		*sampleDiff = temp;
	mutex.unlock();
}

unsigned int AudioBuffer::getPlayTime()
{
	return currentFrame / ((SAMPLE_RATE * CHANNELS) / 1000);
}

void AudioBuffer::setPlayTime(unsigned int time)
{
	unsigned int newPlayedFrame = ((SAMPLE_RATE * CHANNELS) / 1000) * time;
#ifdef MUTEX	
	mutex.lock();
#endif
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
#ifdef MUTEX
	mutex.unlock();
#endif
}

void AudioBuffer::reset()
{
	wxLogDebug("Resetting audio buffer with readOffset: %d writeOffset: %d", readOffset, writeOffset);
#ifdef MUTEX
	mutex.lock();
#endif

	readOffset = 0;
	writeOffset = 0;
	stutter = 0;
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
#ifdef MUTEX
	mutex.unlock();
#endif

}

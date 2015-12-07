#include "AudioBuffer.h"

#include <wx/log.h>
#include <algorithm> // std::min
#include <cmath>
#include <mutex>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BUFFER_TIME 1
#define BUFFER_SIZE SAMPLE_RATE * CHANNELS * BUFFER_TIME

std::mutex mutex;

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
	mutex.lock();
	if (samples == 0) {
		readOffset = 0;
		writeOffset = 0;
		stutter = 0;
		writeBufferOffset = 0;
		readBufferOffset = 0;
		queuedBuffers.clear();
		currentBuffer = getBufferFromPool();
		queuedBuffers.push_back(currentBuffer);
		mutex.unlock();
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
	mutex.unlock();
}

int AudioBuffer::readData(int16_t *dest, const unsigned int samples)
{
	unsigned int samplesLeft = samples;
	mutex.lock();
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
			mutex.unlock();
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
	mutex.unlock();
	return samples;
}

int AudioBuffer::getSampleDiff()
{
	mutex.lock();
	int temp = writeOffset - readOffset;

	if (temp < 0) return 0;
	mutex.unlock();

	return temp;
}

int AudioBuffer::getStutter()
{
	mutex.lock();
	unsigned int temp = stutter;
	stutter = 0;
	mutex.unlock();

	return temp;
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

	mutex.unlock();

}

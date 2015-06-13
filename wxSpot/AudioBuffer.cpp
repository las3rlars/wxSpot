#include "AudioBuffer.h"

#include <mutex>
#include <iterator>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BUFFER_TIME 10
#define BUFFER_SIZE SAMPLE_RATE * CHANNELS * BUFFER_TIME

#define GROW 1
//#define TEST 1


AudioBuffer::AudioBuffer() : playedFrame(0), writtenFrame(0)
{
	buffer.resize(BUFFER_SIZE, 0);
	rewindPos = INT_MAX;
	readOffset = 0;
	writeOffset = 0;
	stutter = 0;
}


AudioBuffer::~AudioBuffer()
{
}

std::mutex g_lock;

void AudioBuffer::addData(const int16_t *data, const unsigned int samples)
{

	if (samples == 0) {
		readOffset = 0;
		writeOffset = 0;
		stutter = 0;
		return;
	}
#ifdef GROW	
	if (writeOffset + samples > buffer.size()) {
		buffer.resize(buffer.size() + BUFFER_SIZE, 0);
	}

	std::copy(data, data + samples, &buffer[writeOffset]);
	writeOffset += samples;
	writtenFrame += samples;
#elif TEST
	g_lock.lock();
	if (0 + samples < readOffset && rewindPos == INT_MAX) {
		rewindPos = writeOffset;
		writeOffset = 0;
	} else if (writeOffset + samples > rewindPos) {
		if (writeOffset + samples > buffer.size()) {
			buffer.resize(buffer.size() + BUFFER_SIZE, 0);
		}


	std::copy(data, data + samples, &buffer[writeOffset]);
	writeOffset += samples;

	writtenFrame += samples;
	g_lock.unlock();
#else
	g_lock.lock();
	//queue.insert(it, 2, 20);
	//queue.insert(it, samples, data);
	std::copy(data, data + samples, std::back_inserter(queue));
	/*for (int i = 0; i < samples; i++) {
		
		queue.push_back(data[i]);
	}*/
	g_lock.unlock();
	writtenFrame += samples;
#endif
	
}

int AudioBuffer::readData(int16_t *dest, const unsigned int samples)
{
#ifdef GROW
	if (readOffset + samples > buffer.size()) {
		stutter++;
		readOffset = 0;
	}

	if (readOffset > writeOffset) {
		stutter++;
		readOffset = 0;
	}

	std::copy(&buffer[readOffset], &buffer[readOffset] + samples, dest);
	readOffset += samples;
	playedFrame += samples;
	return 0;
#elif TEST
	if (writeOffset == 0) {
		return 0;
	}

	/*if (readOffset + samples > buffer.size()) {
		stutter++;
		readOffset = 0;
	}*/

	int diff = 0;

	g_lock.lock();

	if (readOffset == rewindPos) {
		readOffset = 0;
		rewindPos = INT_MAX;
	}

	std::copy(&buffer[readOffset], &buffer[readOffset] + samples - diff, dest);
	readOffset += samples - diff;
	g_lock.unlock();

	playedFrame += samples;
#else
	g_lock.lock();

	//for (int i = samples; i > 0; i--) {
	for (int i = 0; i < samples; i++) {
		dest[i] = queue.front();
		queue.pop_front();
	}
	g_lock.unlock();
#endif
	playedFrame += samples;
	return 0;
}

int AudioBuffer::getSampleDiff()
{
	int temp = writtenFrame - playedFrame;

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
	return playedFrame / (44.1 * 2);
}

void AudioBuffer::setPlayTime(unsigned int time)
{
	playedFrame = (44.1 * 2) * time;
}

void AudioBuffer::reset()
{
	readOffset = 0;
	writeOffset = 0;
	stutter = 0;
	writtenFrame = 0;
	playedFrame = 0;
}
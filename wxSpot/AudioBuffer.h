#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <forward_list>
#include <list>

class Visualizer;

class AudioBuffer
{
public:
	AudioBuffer();
	~AudioBuffer();

	void addData(const int16_t *data, const unsigned int samples);
	int readData(int16_t *dest, const unsigned int samples);
	void getBufferStatus(int *stutter, int *sampleDiff);
	unsigned int getPlayTime();
	void setPlayTime(unsigned int time);
	void reset();

private:
	std::mutex mutex;

	std::shared_ptr<int16_t> getBufferFromPool();
	unsigned short int channels;
	unsigned int readOffset;
	unsigned int writeOffset;
	unsigned int currentFrame;
	unsigned short int stutter;

	unsigned int readBufferOffset;
	unsigned int writeBufferOffset;

	std::shared_ptr<int16_t> currentBuffer;
	std::forward_list<std::shared_ptr<int16_t>> bufferPool;
	std::list<std::shared_ptr<int16_t>> queuedBuffers;
};


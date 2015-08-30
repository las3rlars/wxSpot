#pragma once

#include <cstdint>
#include <vector>
//#include <queue>
#include <deque>

class AudioBuffer
{
public:
	AudioBuffer();
	~AudioBuffer();

	void addData(const int16_t *data, const unsigned int samples);
	int readData(int16_t *dest, const unsigned int samples);
	int getSampleDiff();
	int getStutter();
	unsigned int getPlayTime();
	void setPlayTime(unsigned int time);
	void reset();

	unsigned int channels;
	unsigned int readOffset;
	unsigned int writeOffset;
	unsigned int currentFrame;
	unsigned int stutter;

	std::vector<int16_t> buffer;
};


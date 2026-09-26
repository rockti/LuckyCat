#pragma once

class CompressUtil
{
public:
	CompressUtil(void);
public:
	virtual ~CompressUtil(void);
public:
	static void Zerocompress(unsigned char const* inputBuffer, unsigned long inputNumBytes, unsigned char* outputBuffer, unsigned long& outputNumBytes);
	static void Zerodecompress(unsigned char const* inputBuffer,unsigned long inputNumBytes, unsigned char* outputBuffer, unsigned long& outputNumBytes);
};

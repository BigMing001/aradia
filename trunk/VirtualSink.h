#pragma once
#include "MediaSink.hh"

class VirtualSink : public MediaSink
{
protected:
	unsigned char *mBuffer;
	unsigned int mBufferSize;
	
	VirtualSink(UsageEnvironment& aEnv, unsigned int aBufSize);
	virtual ~VirtualSink();
	virtual Boolean continuePlaying();
	static void afterGettingFrame(void* clientData,
								  unsigned frameSize, unsigned numTruncatedBytes,
								  timeval presentationTime, unsigned durationInMicroseconds);
	
	virtual void afterGettingFrame1(unsigned frameSize,
									struct timeval presentationTime) = 0;
};

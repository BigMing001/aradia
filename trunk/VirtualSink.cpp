#include "stdafx.h"
#include "VirtualSink.h"

VirtualSink::VirtualSink(UsageEnvironment& aEnv, unsigned int aBufSize)
: MediaSink(aEnv)
, mBufferSize(aBufSize)
{
	if (mBufferSize > 0)
	{
		mBuffer = new unsigned char[mBufferSize];
		if (mBuffer == NULL)
		{
			mBufferSize = 0;
		}
	}
}

VirtualSink::~VirtualSink() {
	if(mBuffer) {
		delete []mBuffer;
		mBuffer = NULL;
		mBufferSize = 0;
	}
}

Boolean
VirtualSink::continuePlaying() {
	if (fSource == NULL || mBuffer == NULL) {
		return False;
	}
	fSource->getNextFrame(mBuffer, mBufferSize, afterGettingFrame, this,
						  onSourceClosure, this);
	return True;
}

void
VirtualSink::afterGettingFrame(void* clientData, unsigned frameSize,
							   unsigned /*numTruncatedBytes*/,
							   struct timeval presentationTime,
							   unsigned /*durationInMicroseconds*/) {
	VirtualSink* sink = (VirtualSink*)clientData;
	sink->afterGettingFrame1(frameSize, presentationTime);
	if (sink->continuePlaying() == false)
	{
		sink->onSourceClosure(clientData);
	}
}

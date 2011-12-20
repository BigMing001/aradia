

#include <MediaSink.hh>
#include <H264VideoRTPSink.hh>
#include <liveMedia.hh>
#include <iostream>
#include "VirtualSink.h"
#include "TSWriter.h"
#include <deque>

using namespace std;

#define MAX_SIZE 20000

extern "C"
{
#include <libavformat/avformat.h>·
#include <libavcodec/avcodec.h>
#include "libswscale/swscale.h"
}

class MPEG4VideoSink : public VirtualSink {
public:
	static MPEG4VideoSink * createNew(UsageEnvironment& env, char const* fileName,const char* sProp,CodecID codecID,int bufSize,int aWidth,int aHeight,int fr,CodecID outCodec);
	
protected:
	MPEG4VideoSink(UsageEnvironment& env, const char * out_filename,const char * sProp,CodecID codeciD,int bufSize,int aWidth,int aHeight,int fr,CodecID outCodec);
	// called only by createNew()
	virtual ~MPEG4VideoSink();
	
	
protected:
	/*static void afterGettingFrame(void* clientData, unsigned frameSize,
								  unsigned numTruncatedBytes,
								  struct timeval presentationTime,
								  unsigned durationInMicroseconds);*/
	
	
	virtual void afterGettingFrame1(unsigned frameSize,
									struct timeval presentationTime);
	
	
	
private: 
	virtual Boolean continuePlaying();
	AVCodec *mAVCodec;
	AVCodecContext *mAVCodecContext;
	AVFrame *mAVFrame;
	unsigned int fPos;
	AVFrame * rgb;
	std::deque<unsigned char*> mQueue;
	static void HiloThread(void * arg);
	int w;
	int h;
	uintptr_t mThread;
	PixelFormat formatoPixel;
	const char * mFileName;
    static const DWORD bufferSizeInMiliSec = 2000;
	void AddData(uint8_t* aData, int aSize);
	static void HiloThread2(void * arg);
};



#include <MediaSink.hh>
#include <H264VideoRTPSink.hh>
#include <liveMedia.hh>
#include <iostream>
#include "VirtualSink.h"
#include "TSWriter.h"
#include <deque>



#define MAX_SIZE 20000

extern "C"
{
#include <libavformat/avformat.h>·
#include <libavcodec/avcodec.h>
#include "libswscale/swscale.h"
}

class H264VideoSink : public VirtualSink {
public:
	//static H264VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,int width,int height,CodecID codec);
	//static H264VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID codec);
	static H264VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID codec);
protected:
	//H264VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,int width,int height,CodecID codec);
	//H264VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID codec);
	H264VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID codec);
	// called only by createNew()
	virtual ~H264VideoSink();
	
	
protected:
	/*static void afterGettingFrame(void* clientData, unsigned frameSize,
	 unsigned numTruncatedBytes,
	 struct timeval presentationTime,
	 unsigned durationInMicroseconds);*/
	
	
	virtual void afterGettingFrame1(unsigned frameSize,
									struct timeval presentationTime);
	
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
	int frameRate;
	CodecID outputCodec;
	uintptr_t mThread;
	PixelFormat formatoPixel;
	const char * mFileName;
    static const DWORD bufferSizeInMiliSec = 2000;
	void AddData(uint8_t* aData, int aSize);
	static void HiloThread2(void * arg);
	CRITICAL_SECTION criticalSection;
	struct SwsContext * scale_ctx;
	bool isGotFrame;
};



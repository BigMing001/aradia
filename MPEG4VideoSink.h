

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

class MPEG4VideoSink : public VirtualSink {
public:
	//static MPEG4VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec,int aWidth,int aHeight);
	//static MPEG4VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec);
	static MPEG4VideoSink * createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID outCodec);
protected:
	//MPEG4VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec,int aWidth,int aHeight);
	//MPEG4VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec);
	MPEG4VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID outCodec);
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
	static  void HiloThread(void * arg);
	int w;
	int h;
	int frameRate;
	uintptr_t mThread;
	PixelFormat formatoPixel;
	const char * mFileName;
    static const DWORD bufferSizeInMiliSec = 2000;
	void AddData(uint8_t* aData, int aSize);
	static void HiloThread2(void * arg);
	CodecID outputCodec;
	CRITICAL_SECTION criticalSection;
	bool isGotFrame;
	struct SwsContext * scale_ctx;
};

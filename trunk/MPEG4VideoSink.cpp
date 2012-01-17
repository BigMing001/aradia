
#include "stdafx.h"
#include "MPEG4VideoSink.h"
#include <iostream>
#include "TSWriter.h"
#include <process.h>



static TSWriter * writer;
static int frame_count;

MPEG4VideoSink::MPEG4VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID outCodec)
: VirtualSink(env, bufSize){
	fPos = 0;
	mThread = NULL;
	w = 0;
	h = 0;
	formatoPixel = PIX_FMT_NONE;
	mFileName = filename;
	//frameRate = fr;
	outputCodec = outCodec;
	isGotFrame = false;
	mBufferSize = bufSize;
	writer = new TSWriter();
	mAVCodec = avcodec_find_decoder(codecID);
	if ( mAVCodec != NULL) 
	{
		std::cout << "Decoder finded" << std::endl;
	}else {
		std::cout << "Not finded" << std::endl;
	}
	mAVCodecContext = avcodec_alloc_context();
	if(sProp != NULL){
		
		
		unsigned configLen;
		unsigned char * configData = parseGeneralConfigStr(sProp, configLen);
		AddData(configData,configLen);
		delete[] configData;
		mAVCodecContext->extradata = mBuffer;
		mAVCodecContext->extradata_size = fPos;
	}
	mAVCodecContext->flags = 0;

	
	if (avcodec_open(mAVCodecContext, mAVCodec) < 0) {
		std::cout << "Error opening codec" << std::endl;
	}else
	{
		std::cout << "Codec opened successfull" << std::endl;
	}
	mAVFrame = avcodec_alloc_frame();
	InitializeCriticalSection(&this->criticalSection);
	
}
	
void MPEG4VideoSink::AddData(uint8_t* aData, int aSize){
	memcpy(mBuffer + fPos, aData, aSize);
	fPos += aSize;
}
	
MPEG4VideoSink::~MPEG4VideoSink() {
	if ( mThread ) 
	{
		WaitForSingleObject((HANDLE)mThread, INFINITE);
        mThread = NULL;
	}
	if (mAVFrame)
	{
		avcodec_close(mAVCodecContext);
		
		av_free(mAVFrame);
		mAVFrame = NULL;
	}
	if (mAVCodecContext)
	{
		av_free(mAVCodecContext);
		mAVCodecContext = NULL;
	}
}

MPEG4VideoSink* MPEG4VideoSink::createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,CodecID outCodec) {
	
	return new MPEG4VideoSink(env,filename,sProp,codecID,bufSize,outCodec);
	
}

Boolean MPEG4VideoSink::continuePlaying() {
	if (fSource == NULL) {
		return False;
	}
	fSource->getNextFrame(mBuffer + fPos,
						  mBufferSize - fPos, afterGettingFrame, this,
						  onSourceClosure, this);
	return True;
}








void MPEG4VideoSink::afterGettingFrame1(unsigned frameSize,
								  struct timeval presentationTime) {
	
	int got_frame = 0;
	unsigned int size = frameSize;
	unsigned char *pBuffer = mBuffer + fPos;
	
	uint8_t* data = pBuffer;
	uint8_t startCode4[] = {0x00, 0x00, 0x00, 0x01};
	uint8_t startCode3[] = {0x00, 0x00, 0x01};
	if(size<4){
		return;
	}
	if(memcmp(startCode3, pBuffer, sizeof(startCode3)) == 0){
		data += 3;
	}else if(memcmp(startCode4, pBuffer, sizeof(startCode4)) == 0){
		data += 4;
	}else{
		pBuffer -= 3;
		size += 3;
	}
	AVPacket avpkt;
	avpkt.data = pBuffer;
	avpkt.size = size;
	while (avpkt.size > sizeof(startCode4)) {
		int len = avcodec_decode_video2(mAVCodecContext, mAVFrame, &got_frame, &avpkt);
		if ( got_frame ) 
		{
			isGotFrame = true;
			scale_ctx = sws_getContext(mAVCodecContext->width, mAVCodecContext->height, mAVCodecContext->pix_fmt, mAVCodecContext->width, mAVCodecContext->height, PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			if (scale_ctx)
			{
				rgb = avcodec_alloc_frame();
				if ( !mThread ) 
				{
					mThread = _beginthread(&(MPEG4VideoSink::HiloThread), 0, this);
				}
				AVFrame *rgb = avcodec_alloc_frame();
				int stride = mAVCodecContext->width * 4;
				int numBytes = mAVCodecContext->height * stride;
				unsigned char * buf = new unsigned char[numBytes];
				if ( rgb ) 
				{
					avpicture_fill((AVPicture*)rgb, (uint8_t*)buf,PIX_FMT_RGB32,mAVCodecContext->width,mAVCodecContext->height);

					sws_scale(scale_ctx, mAVFrame->data, mAVFrame->linesize, 0,
							  mAVCodecContext->height, rgb->data,
							  rgb->linesize);


					EnterCriticalSection(&this->criticalSection);
					mQueue.push_back(buf);
					LeaveCriticalSection(&this->criticalSection);

					//writer->write_picture(rgb);
					av_free(rgb);
				}
			}
			sws_freeContext(scale_ctx);
		}
		
		avpkt.size -= len;
		avpkt.data += len;
		
	}
 }

typedef struct RecordData
{
  int mWidth;
  int mHeight;
  PixelFormat formatoPixel;
  CodecID outputCodec;
  int frameRate;
  const char *outputFileName;
  uint8_t * mBuffer;
  bool isGotFrame;
}RecordData;
						
void  MPEG4VideoSink::HiloThread(void *  arg)
{
	MPEG4VideoSink * sink = (MPEG4VideoSink*)arg;
	if ( sink ) 
	{
	   RecordData * data =  new RecordData();
	   data->mWidth = sink->mAVCodecContext->width;
	   data->mHeight = sink->mAVCodecContext->height;
	   data->formatoPixel = PIX_FMT_RGB32;
	   data->outputCodec = sink->outputCodec;
	   int stride = sink->mAVCodecContext->width * 4;
	   int numBytes = sink->mAVCodecContext->height * stride;
	   int autoBufSize =  sizeof(unsigned char) * data->mHeight * stride;
	   data->outputFileName = sink->mFileName;
	   data->isGotFrame = sink->isGotFrame;
	   data->mBuffer =  new uint8_t[autoBufSize];
	   uintptr_t thread = NULL;
	   while ( 1 )
	   {
		   unsigned int queueSize =0 ;
		   EnterCriticalSection(&sink->criticalSection);
		   queueSize = sink->mQueue.size();
		   LeaveCriticalSection(&sink->criticalSection);
		   DWORD sleep_i = sink->bufferSizeInMiliSec / (1 + queueSize);
		   Sleep(sleep_i);
		   if ( queueSize > 0 ) 
		   {
			   EnterCriticalSection(&sink->criticalSection);
			   unsigned char * frame  = sink->mQueue.front();
               sink->mQueue.pop_front();
			   LeaveCriticalSection(&sink->criticalSection);
			   if ( data->mBuffer )
			   {
				   memcpy(data->mBuffer,frame,autoBufSize);
				   delete[] frame;
			   }
		   }
		   if ( !thread ) 
		   {
			   thread = _beginthread(&(MPEG4VideoSink::HiloThread2), 0, data);
		   }
	   }
	   

	}
}

void MPEG4VideoSink::HiloThread2(void * args)
{
	RecordData * data = ( RecordData * ) args;
	if ( data ) 
	{
		   writer->CrearArchivo(data->outputFileName,data->mWidth,data->mHeight,data->formatoPixel,data->outputCodec);
		   std::cout << "Fichero a generar = " << data->outputFileName << std::endl;
		   std::cout << "Codec = " << data->outputCodec <<  std::endl;
		   while ( 1 )
		   {
		   AVFrame * frame = avcodec_alloc_frame();
           avpicture_fill((AVPicture *)frame, data->mBuffer,
                      data->formatoPixel,data->mWidth, data->mHeight);
		   DWORD sleep_i = writer->getTicksPerFrame();
		   if ( writer->hasError() ) 
		   {
			     //Un error se ha producido salimos.
			   exit ( -1  );
		   }else
		   {
		    if ( data->isGotFrame == true ) 
			{
				frame_count++;
				writer->write_picture(frame,frame_count);
			}
		   }
		   Sleep(sleep_i);
		   }
	}
}
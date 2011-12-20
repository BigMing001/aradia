
#include "stdafx.h"
#include "MPEG4VideoSink.h"
#include <iostream>
#include "TSWriter.h"
#include <process.h>



static TSWriter * writer;

MPEG4VideoSink::MPEG4VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec,int aWidth,int aHeight)
: VirtualSink(env, bufSize){
	fPos = 0;
	mThread = NULL;
	w = aWidth;
	h = aHeight;
	formatoPixel = PIX_FMT_RGB32;
	mFileName = filename;
	frameRate = fr;
	outputCodec = outCodec;
	mBufferSize = bufSize;
	writer = new TSWriter();
	mAVCodec = avcodec_find_decoder(codecID);
	if ( mAVCodec != NULL) 
	{
		std::cout << "Decoder finded" << endl;
	}else {
		std::cout << "Not finded" << endl;
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
		std::cout << "Error opening codec" << endl;
	}else
	{
		std::cout << "Codec opened successfull" << endl;
	}
	mAVFrame = avcodec_alloc_frame();
	InitializeCriticalSection(&this->criticalSection);
	
}
	
void MPEG4VideoSink::AddData(uint8_t* aData, int aSize){
	memcpy(mBuffer + fPos, aData, aSize);
	fPos += aSize;
}
	
MPEG4VideoSink::~MPEG4VideoSink() {
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

MPEG4VideoSink* MPEG4VideoSink::createNew(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int fr,CodecID outCodec,int aWidth,int aHeight) {
	
	return new MPEG4VideoSink(env,filename,sProp,codecID,bufSize,fr,outCodec,aWidth,aHeight);
	
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
			
			SwsContext * scale_ctx = sws_getContext(mAVCodecContext->width, mAVCodecContext->height, mAVCodecContext->pix_fmt, w, h, formatoPixel, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			if (scale_ctx)
			{
				rgb = avcodec_alloc_frame();
				if ( !mThread ) 
				{
					mThread = _beginthread(&(MPEG4VideoSink::HiloThread), 0, this);
				}
				AVFrame *rgb = avcodec_alloc_frame();
				int numBytes = avpicture_get_size(PIX_FMT_RGB32,w,h);
				unsigned char * buf = new unsigned char[numBytes];
				if ( rgb ) 
				{
					avpicture_fill((AVPicture*)rgb, (uint8_t*)buf,formatoPixel,w,h);

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
  CodecID outputc;
  int frameRate;
  const char *outputFileName;
  uint8_t * mBuffer;
}RecordData;
						
void  MPEG4VideoSink::HiloThread(void *  arg)
{
	MPEG4VideoSink * sink = (MPEG4VideoSink*)arg;
	if ( sink ) 
	{
	   RecordData * data =  new RecordData();
	   data->mWidth = sink->w;
	   data->mHeight = sink->h;
	   data->formatoPixel = sink->formatoPixel;
	   data->frameRate  = sink->frameRate;
	   data->outputc = sink->outputCodec;
	   int size = avpicture_get_size(data->formatoPixel,data->mWidth,data->mHeight);
	   data->outputFileName = sink->mFileName;
	   data->mBuffer =  new uint8_t[size];
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
				   memcpy(data->mBuffer,frame,size);
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
		   writer->CrearArchivo(data->outputFileName,data->mWidth,data->mHeight,data->frameRate,data->formatoPixel,data->outputc);
		   while ( 1 )
		   {
		   AVFrame * frame = avcodec_alloc_frame();
           avpicture_fill((AVPicture *)frame, data->mBuffer,
                      data->formatoPixel,data->mWidth, data->mHeight);
		   DWORD sleep_i = writer->getTicksPerFrame();
		   if ( writer->hasError() ) 
		   {
			std::cout << "Error creating file" << endl;
		   }else
		   {
			writer->write_picture(frame);
		   }
		   Sleep(sleep_i);
		   }
	}
}
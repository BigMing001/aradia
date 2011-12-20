
#include "stdafx.h"
#include "H264VideoSink.h"
#include <iostream>
#include "TSWriter.h"
#include <process.h>



static TSWriter * writer;

H264VideoSink::H264VideoSink(UsageEnvironment& env,char const * filename,const char  * sProp,CodecID codecID,int bufSize,int aWidth,int aHeight,int fr,CodecID codec)
: VirtualSink(env, bufSize){
	fPos = 0;
	w = aWidth;
	mThread = NULL;
	mFileName = filename;
	h = aHeight;
	formatoPixel = PIX_FMT_RGB32;
	mBufferSize = bufSize;
	writer = new TSWriter();
	writer->CrearArchivo(filename,w,h,fr,PIX_FMT_RGB32,codec);
	mAVCodec = avcodec_find_decoder(codecID);
	if ( mAVCodec != NULL) 
	{
		std::cout << "Decoder finded" << endl;
	}else {
		std::cout << "Not finded" << endl;
	}
	mAVCodecContext = avcodec_alloc_context();
	uint8_t startCode[] = {0x00, 0x00,0x01};
	if(sProp != NULL){
		unsigned numSPropRecords;
		SPropRecord* sPropRecords = parseSPropParameterSets(sProp, numSPropRecords);
		for (unsigned i = 0; i < numSPropRecords; ++i) {
			AddData(startCode, sizeof(startCode));
			AddData(sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
		}
		AddData(startCode, sizeof(startCode));
		mAVCodecContext->extradata = mBuffer;
		mAVCodecContext->extradata_size = fPos;
	}
	AddData(startCode, sizeof(startCode));
	mAVCodecContext->flags = 0;
	
	if (avcodec_open(mAVCodecContext, mAVCodec) < 0) {
		std::cout << "Error opening codec" << endl;
	}
	
	
	if ( mAVCodecContext->codec_id == CODEC_ID_H264 ) 
	{
		mAVCodecContext->flags2 |= CODEC_FLAG2_CHUNKS;
	}
	mAVFrame = avcodec_alloc_frame();
	
}
	
void H264VideoSink::AddData(uint8_t* aData, int aSize){
	memcpy(mBuffer + fPos, aData, aSize);
	fPos += aSize;
}
	
H264VideoSink::~H264VideoSink() {
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

H264VideoSink* H264VideoSink::createNew(UsageEnvironment& env, char const* fileName,const char * sProp,CodecID codecID,int aBufSize,int aWidth,int aHeight,int fr,CodecID codec) {
	
	return new H264VideoSink(env,fileName,sProp,codecID,aBufSize,aWidth,aHeight,fr,codec);
	
}

Boolean H264VideoSink::continuePlaying() {
	if (fSource == NULL) {
		return False;
	}
	fSource->getNextFrame(mBuffer + fPos,
						  mBufferSize - fPos, afterGettingFrame, this,
						  onSourceClosure, this);
	return True;
}








void H264VideoSink::afterGettingFrame1(unsigned frameSize,
								  struct timeval presentationTime) {
	
	int got_frame = 0;
	unsigned int size = frameSize;
	unsigned char *pBuffer = mBuffer + fPos;
	
	uint8_t* data = (uint8_t*)pBuffer;
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
		if (len < 0) {
			std::cout << "Error decoding frame" << endl;
			return;
		}
		if ( got_frame ) 
		{
			//SwsContext *  scale_ctx = sws_getContext(mAVCodecContext->coded_width,
			//mAVCodecContext->coded_height, mAVCodecContext->pix_fmt,
			//w,h, PIX_FMT_RGB32, SWS_FAST_BILINEAR,NULL,NULL,NULL);
			
			SwsContext * scale_ctx = sws_getContext(mAVCodecContext->width, mAVCodecContext->height, mAVCodecContext->pix_fmt, w, h, PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			if (scale_ctx)
			{
				rgb = avcodec_alloc_frame();
				if ( !mThread ) 
				{
					mThread = _beginthread(&(H264VideoSink::HiloThread), 0, this);
				}
				AVFrame *rgb = avcodec_alloc_frame();
				int numBytes = avpicture_get_size(PIX_FMT_RGB32,w,h);
				unsigned char * buf = new unsigned char[numBytes];
				if ( rgb ) 
				{
					avpicture_fill((AVPicture*)rgb, (uint8_t*)buf, PIX_FMT_RGB32,w,h);

					sws_scale(scale_ctx, mAVFrame->data, mAVFrame->linesize, 0,
							  mAVCodecContext->height, rgb->data,
							  rgb->linesize);

					mQueue.push_back(buf);
					//writer->write_picture(rgb);
				}
				av_free(rgb);
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
  const char *outputFileName;
  uint8_t * mBuffer;
}RecordData;
								 
void H264VideoSink::HiloThread(void * arg)
{
	H264VideoSink * sink = (H264VideoSink*)arg;
	if ( sink ) 
	{
	   RecordData * data =  new RecordData();
	   data->mWidth = sink->w;
	   data->mHeight = sink->h;
	   data->formatoPixel = sink->formatoPixel;
	   int size = avpicture_get_size(data->formatoPixel,data->mWidth,data->mHeight);
	   data->outputFileName = sink->mFileName;
	   data->mBuffer =  new uint8_t[size];
	   uintptr_t thread = NULL;
	   
	   while ( 1 )
	   {
		   unsigned int queueSize =0 ;
		   queueSize = sink->mQueue.size();
		   DWORD sleep_i = sink->bufferSizeInMiliSec / (1 + queueSize);
		   Sleep(sleep_i);
		   if ( queueSize > 0 ) 
		   {
			   unsigned char * frame  = sink->mQueue.front();
               sink->mQueue.pop_front();
			   if ( data->mBuffer )
			   {
				   memcpy(data->mBuffer,frame,size);
			   }
		   }
		   if ( !thread ) 
		   {
			   thread = _beginthread(&(H264VideoSink::HiloThread2), 0, data);
		   }
	   }
	   

	}
}

void H264VideoSink::HiloThread2(void * args)
{
	RecordData * data = ( RecordData * ) args;
	if ( data ) 
	{
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
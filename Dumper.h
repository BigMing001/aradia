/*
 *  Dumper.h
 *  RTSPDumper
 *
 *  Created by Moises on 19/11/11.
 *  Copyright 2011 Vidium. All rights reserved.
 *
 */

#include "liveMedia.hh"  
#include "BasicUsageEnvironment.hh"  
#include "GroupsockHelper.hh" 

#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#define MAXIMUM_QUALITY_FRAME_RATE 30
#define HIGHT_QUALITY_FRAME_RATE 25

using namespace std;

class RTSPDumper
{
public:
	
	void setupScheduler();
	void setupRtspClient(const char * uri);
	void dumpFile();
	void setFrameRate(int fr);
    int getFrameRate();
	void setOutputFileName ( const char  * fFilename );
	const char * getOutputFileName();
	void setWidth(int w);
	void setHeight(int h);
	int getHeight();
	int getWidth();
	CodecID getOutputCodec();
	void setOutputCodec(CodecID outputCodec);
	
private:
	
	TaskScheduler * task;
	UsageEnvironment * env;
	RTSPClient * client;
	MediaSubsession * sub;
	MediaSession * sesion;
	int frame_rate;
	int w;
	int h;
	CodecID outCodec;
	const char * outputFileName;
	
};

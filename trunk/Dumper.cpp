/*
 *  Dumper.cpp
 *  RTSPDumper
 *
 *  Created by Moises on 19/11/11.
 *  Copyright 2011 Vidium. All rights reserved.
 *
 */
#include "stdafx.h"
#include "Dumper.h"
#include "MPEG4VideoSink.h"
#include "H264VideoSink.h"

void RTSPDumper::setupScheduler()
{
	this->task = BasicTaskScheduler::createNew();
	this->env = BasicUsageEnvironment::createNew(*this->task);
}

void RTSPDumper::setupRtspClient(const char * uri)
{
	this->client = RTSPClient::createNew(*this->env);
	if ( this->client != NULL )
	{
		char * opt = this->client->sendOptionsCmd(uri);
		std::cout << opt << endl;
		delete[] opt;
		char * bufcmd = this->client->describeURL(uri);
		std::cout << bufcmd << endl;
		this->sesion = MediaSession::createNew(*this->env,bufcmd);
		delete[] bufcmd;
	}
}

void RTSPDumper::setFrameRate(int fr)
{
	this->frame_rate = fr;
}

int RTSPDumper::getFrameRate()
{
	return ( this->frame_rate );
}

int  RTSPDumper::getWidth()
{
	return ( this->w );
}

int RTSPDumper::getHeight()
{
	return ( this->h );
}

void RTSPDumper::setWidth(int width)
{
	this->w = width;
}

void RTSPDumper::setHeight(int height)
{
	this->h = height;
}

void RTSPDumper::setOutputFileName(const char * outf)
{
	this->outputFileName = outf;
}

void RTSPDumper::setOutputCodec(CodecID codec)
{
	this->outCodec = codec;
}

CodecID RTSPDumper::getOutputCodec()
{
	return ( this->outCodec );
}

const  char * RTSPDumper::getOutputFileName()
{
	return ( this->outputFileName );
}

void RTSPDumper::dumpFile()
{
	unsigned int buf_size = 2000 * 2000;
	MediaSession * sesionNew = this->sesion;
	RTSPClient * cliente = this->client;
	//H264VideoFileSink * sink;
	H264VideoSink * sink;
	MPEG4VideoSink * mpegVideoSink;
	if ( sesionNew != NULL )
	{
		MediaSubsessionIterator itor(*sesionNew);
		MediaSubsession * sub = itor.next();
		while ( sub != NULL )
		{
			if ( sub->initiate(0) ) 
			{
				
				cliente->setupMediaSubsession(*sub);
				if ( strcmp(sub->codecName(),"H264") == 0  && strcmp(sub->mediumName(),"video") == 0)
				{
				    
					std::cout << "Flujo H264 Detectado...dumping.." << endl;
					RTPSource * sourc =  sub->rtpSource();
					//sink = H264VideoFileSink::createNew(*this->env, uri, sub->fmtp_spropparametersets(),1000000, false);
					sink = H264VideoSink::createNew(*this->env, this->getOutputFileName(), sub->fmtp_spropparametersets(),CODEC_ID_H264,buf_size,this->getWidth(),this->getHeight(),this->getFrameRate(),this->getOutputCodec());
					sourc->setPacketReorderingThresholdTime(1000000);
					sub->sink = sink;
					sub->sink->startPlaying(*sourc, 0, 0);
				    
				}else if ( !strcmp(sub->codecName(), "MPEG4") || !strcmp(sub->codecName(), "MP4V-ES") && strcmp(sub->mediumName(),"video") == 0)
				{
					std::cout << "Flujo MPEG4 Detectado...dumping.." << endl;
					RTPSource * sourc =  sub->rtpSource();
					//sink = H264VideoFileSink::createNew(*this->env, uri, sub->fmtp_spropparametersets(),1000000, false);
					mpegVideoSink = MPEG4VideoSink::createNew(*this->env, this->getOutputFileName(), sub->fmtp_config(),CODEC_ID_MPEG4,buf_size,this->getWidth(),this->getHeight(),this->getFrameRate(),this->getOutputCodec());
					sourc->setPacketReorderingThresholdTime(1000000);
					sub->sink = mpegVideoSink;
					sub->sink->startPlaying(*sourc, 0, 0);
				}
			}
			sub = itor.next();
		}
		cliente->playMediaSession(*this->sesion);
		this->env->taskScheduler().doEventLoop();
		
		if(this->sesion != NULL){
			MediaSubsessionIterator itor(*this->sesion);
			MediaSubsession * sub = itor.next();
			while(sub != NULL) {
				if (sub->sink)
				{
					sub->sink->stopPlaying();
					MediaSink::close(sub->sink);
				}
				sub = itor.next();
			}
			cliente->teardownMediaSession(*this->sesion);
			MediaSession::close(this->sesion);
		}
		RTSPClient::close(cliente);
		this->env->reclaim();
		delete this->task;
	}
}


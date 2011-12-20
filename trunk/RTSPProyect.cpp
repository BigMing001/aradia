#include "stdafx.h"
#include <iostream>
#include "Dumper.h"
#include <process.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}





/*
 Posibles direcciones.
 */

//dump->setupRtspClient("rtsp://192.168.1.34:1935/vod/sample.mp4");
//dump->setupRtspClient("rtsp://root:video@elrincondelfer.no-ip.info:554/mpeg4/media.amp");
//dump->setupRtspClient("rtsp://132.64.128.106:7070/econ17/Kenneth_Arrow.mov");
//dump->setupRtspClient("rtsp://infogijon4.axiscam.net:554/mpeg4/media.amp");

#pragma comment(lib,"wsock32.lib")

using namespace std;



void setupCodecs()
{
	avcodec_register_all();
	av_register_all();
}
int main (int argc, char * const argv[]) {
	setupCodecs();
	if ( argc < 2 ) 
	{
		std::cout << "Error debes insertar dos argumentos<direccion rtsp><fichero de salida.ts>" << endl;
	}else
	{
		RTSPDumper * dumper = new RTSPDumper();
		std::cout << "Argumentos introducidos:" << endl;
		std::cout << argv[1] << endl;
		std::cout << argv[2] << endl;
		const  char * url = argv[1];
		const  char * outputFile = argv[2];
		dumper->setupScheduler();
	    dumper->setupRtspClient(url);
	    dumper->setFrameRate(MAXIMUM_QUALITY_FRAME_RATE); // == 30 frames per second.
	    dumper->setWidth(640);
	    dumper->setHeight(420);
	    dumper->setOutputCodec(CODEC_ID_MPEG2VIDEO);
	    dumper->setOutputFileName(outputFile);
	    dumper->dumpFile();

	}
}

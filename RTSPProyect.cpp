#include "stdafx.h"
#include <iostream>
#include "Dumper.h"
#include <process.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}




using namespace std;

#pragma comment(lib,"wsock32.lib")


void setupCodecs()
{
	avcodec_register_all();
	av_register_all();
}
int main (int argc, char * const argv[]) {
	setupCodecs();
	if ( argc < 2 ) 
	{
		std::cout << "Error debes insertar dos argumentos<direccion rtsp><ancho><alto><fichero_salida.ts>" << endl;
	}else
	{
		RTSPDumper * dumper = new RTSPDumper();
		std::cout << "Argumentos introducidos:" << endl;
		std::cout << argv[1] << endl;
		std::cout << argv[2] << endl;
		const  char * url = argv[1];
		const  char * ancho = argv[2];
		const char * alto = argv[3];
		const char * outputFile = argv[4];
		int width = atoi(ancho);
		int height = atoi(alto);
		if   ( ( width > 0  ) && ( height > 0 )  ) 
		{
			dumper->setupScheduler();
	        dumper->setupRtspClient(url);
	        dumper->setFrameRate(MAXIMUM_QUALITY_FRAME_RATE); // == 30 frames per second.
	        dumper->setWidth(width);
	        dumper->setHeight(height);
	        dumper->setOutputCodec(CODEC_ID_MPEG1VIDEO);
	        dumper->setOutputFileName(outputFile);
	        dumper->dumpFile();
		}else
		{
			std::cout << "Error debes insertar un ancho por alto valido" << endl;
		}

	}
}


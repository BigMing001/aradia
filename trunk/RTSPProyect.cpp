
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
int main (int argc, char * const argv[])
	{
		setupCodecs();
        if ( argc < 2 ) 
		{
			std::cout << "Error debe introducir almenos dos datos<direccion rtsp><direccion del fichero de salida acabado en .ts>" << std::endl;
		}else
		{
		        RTSPDumper * dumper = new RTSPDumper();
                dumper->setupScheduler();
                dumper->setupRtspClient(argv[1]);
                //dumper->setWidth(width);
                //dumper->setHeight(height);
                dumper->setOutputCodec(CODEC_ID_H264);
                dumper->setOutputFileName(argv[2]);
                dumper->dumpFile();
		}
}
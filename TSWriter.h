#pragma once

extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

#include <string>

class TSWriter {
public:
	void CrearArchivo(const std::string& aFilePath, int aWidth, int aHeight,
			 int aFrameRate, PixelFormat aPixFmt,CodecID outCodec);
	~TSWriter();
	
	bool write_picture(AVFrame *aPicture);
	
	bool hasError() { return mHasError; }
	std::string getError() { return mErrorMsg; }
	
	double getPTS() {
		if (mVideoStream) {
			return (double)mVideoStream->pts.val * mVideoStream->time_base.num / mVideoStream->time_base.den;
		}
		return -1.00;
	}
	int getTicksPerFrame() {
		return mVideoStream->codec->ticks_per_frame;
	}
private:
	AVStream* setup_video_stream();
	bool open_video();
	AVFrame *alloc_picture(PixelFormat pix_fmt, int width, int height);
	void close_video();
	
	//errors
	bool mHasError;
	std::string mErrorMsg;
	//inner data
	std::string mFilePath;
	int mBitRate, mWidth, mHeight, mFrameRate;
	PixelFormat mPixFmt;
	AVOutputFormat *mOutFormat;
	AVFormatContext *mFormatCtx;
	AVStream *mVideoStream;
	
	static const PixelFormat s_CodecPixFormat;
	
	struct PictureData {
		uint8_t *mOutBuf;
		int mOutBufSize;
		AVFrame *mTmpPicture;
	}mData;
};


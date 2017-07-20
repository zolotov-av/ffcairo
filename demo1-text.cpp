/****************************************************************************

Демо-программа №1

Демонстрирует возможности модификации изображения при перекодировании

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcmuxer.h>
#include <ffcairo/ffcdemuxer.h>
#include <ffcairo/scale.h>

class MyInputStream: public FFCVideoInput
{
public:
	int frameNo;
	ptr<FFCImage> pic;
	ptr<FFCScale> sin;
	ptr<FFCScale> sout;
	ptr<FFCMuxer> muxer;
	ptr<FFCVideoOutput> vo;
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame();
	
	void ModifyFrame(FFCImage *pic, int iFrame, int64_t ts);
};

void MyInputStream::handleFrame()
{
	frameNo ++;
	
	// Save the frame to disk
	if( 1 /* frameNo < 10 */ )
	{
		//printf("handleFrame #%d\n", frameNo);
		
		// avFrame->pts не работает???
		int64_t ts = av_rescale(frameNo-1, avStream->time_base.num, avStream->time_base.den);
		sin->scale(pic->avFrame, avFrame);
		ModifyFrame(pic.getObject(), frameNo, ts);
		sout->scale(vo->avFrame, pic->avFrame);
		if ( ! vo->encode() )
		{
			printf("frame[%d] encode failed\n", frameNo);
			exit(-1);
		}
		
		int got_packet = 0;
		AVPacket pkt;
		pkt.data = NULL;
		pkt.size = 0;
		av_init_packet(&pkt);
		while ( 1 )
		{
			if ( ! vo->recv_packet(&pkt, got_packet) )
			{
				printf("recv_packet() failed\n");
				exit(-1);
			}
			
			if ( got_packet )
			{
				//vo->rescale_ts(&pkt);
				
				/* Write the compressed frame to the media file. */
				//log_packet(muxer->avFormat, &pkt);
				muxer->writeFrame(&pkt);
			}
			else
			{
				break;
			}
		}
		
		// сохраняем фрейм в файл
		/*
		char szFilename[48];
		sprintf(szFilename, "out/frame%03d.png", frameNo);
		pic->savePNG(szFilename);
		*/
		
		int its = ts;
		if ( frameNo % 50 == 0 ) printf("#%i, ts=%d\n", frameNo, its);
	}
}

void MyInputStream::ModifyFrame(FFCImage *pic, int iFrame, int64_t ts)
{
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	int min = ts / 60;
	int sec = ts % 60;
	
	char sFrameId[48];
	//sprintf(sFrameId, "Frame: %03d", iFrame);
	sprintf(sFrameId, "Time %02d:%02d", min, sec);
	
	double hbox = pic->height * 0.2;
	double shift = pic->height * 0.1;
	
	cairo_set_source_rgba (cr, 0x5a /255.0, 0xe8/255.0, 0xf9/255.0, 96/255.0);
	cairo_rectangle (cr, shift, pic->height - hbox - shift, pic->width - 2*shift, hbox);
	cairo_fill (cr);
	
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, hbox * 0.8);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	
	cairo_move_to (cr, shift*1.5, pic->height - shift*1.5);
	cairo_show_text (cr, sFrameId);
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
}

int main(int argc, char *argv[])
{
	const char *iname = argc > 1 ? argv[1] : "test.avi";
	const char *oname = argc > 2 ? argv[2] : "out.avi";
	printf("input filename = %s\n", iname);
	printf("output filename = %s\n", oname);
	
	// INIT
	av_register_all();
	avformat_network_init();
	
	ptr<FFCDemuxer> demux = new FFCDemuxer();
	
	// открыть файл
	if ( ! demux->open(iname) )
	{
		printf("fail to open file\n");
		return -1;
	}
	printf("demux->probesize = %"PRId64"\n", demux->avFormat->probesize);
	printf("demux->fps_probe_size = %d\n", demux->avFormat->fps_probe_size);
	
	// найти видео-поток
	int video_stream = demux->findVideoStream();
	if ( video_stream < 0 )
	{
		printf("fail to find video stream\n");
		return -1;
	}
	printf("video stream #%d\n", video_stream);
	
	// присоединить обработчик потока
	ptr<MyInputStream> videoStream = new MyInputStream;
	demux->bindStream(video_stream, videoStream);
	
	int width = videoStream->avStream->codecpar->width;
	int height = videoStream->avStream->codecpar->height;
	printf("video size: %dx%d\n", width, height);
	
	FFCVideoOptions opts;
	opts.width = width;
	opts.height = height;
	opts.pix_fmt = AV_PIX_FMT_YUV420P;
	opts.bit_rate = 2000000;
	opts.time_base = videoStream->avStream->time_base;
	printf("time_base = {%d, %d}\n", opts.time_base.num, opts.time_base.den);
	opts.gop_size = 12;
	
	ptr<FFCMuxer> muxer = new FFCMuxer();
	
	// открыть декодер видео
	if ( ! videoStream->openDecoder() )
	{
		printf("stream[%d] openDecoder() failed\n", video_stream);
		return -1;
	}
	
	videoStream->muxer = muxer;
	videoStream->pic = FFCImage::createImage(width, height);
	if ( videoStream->pic.getObject() == NULL )
	{
		printf("fail to create FFCImage\n");
		return -1;
	}
	videoStream->sin = new FFCScale();
	videoStream->sin->init_scale(videoStream->pic->avFrame, videoStream->avFrame);
	videoStream->frameNo = 0;
	
	if ( ! muxer->createFile(oname) )
	{
		printf("failed to createFile(%s)\n", oname);
		return -1;
	}
	
	// add video stream
	AVCodecID video_codec = muxer->defaultVideoCodec();
	if ( video_codec == AV_CODEC_ID_NONE)
	{
		printf("video_codec = NONE\n");
		return -1;
	}
	
	opts.codec_id = video_codec;
	
	ptr<FFCVideoOutput> vo = muxer->createVideoStream(&opts);
	videoStream->vo = vo;
	
	if ( ! vo->openEncoder(&opts) )
	{
		return -1;
	}
	
	av_dump_format(muxer->avFormat, 0, oname, 1);
	ptr<FFCScale> scale = new FFCScale();
	videoStream->sout = scale;
	scale->init_scale(vo->avFrame, videoStream->pic->avFrame);
	
	if ( ! muxer->openFile(oname) )
	{
		printf("fail to openFile(%s)\n", oname);
		return -1;
	}
	
	while ( demux->readFrame() ) ;
	
	muxer->closeFile();
	
	return 0;
}

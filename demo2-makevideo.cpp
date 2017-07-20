/****************************************************************************

Демо-программа №2

Демонстрирует возможности создания видео из отдельных кадров

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcmuxer.h>
#include <ffcairo/scale.h>
#include <math.h>

void DrawPic(ptr<FFCImage> pic, int iFrame)
{
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	int width = pic->width;
	int height = pic->height;
	
	int cx = width / 2;
	int cy = height / 2;
	
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
	
	double r0 = (width > height ? height : width) / 2.0;
	double r1 = r0 * 0.75;
	double r2 = r0 * 0.65;
	
	cairo_set_line_width(cr, r0 * 0.04);
	
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_arc(cr, cx, cy, r1, 0, 2 * M_PI);
	cairo_stroke (cr);
	
	double x, y;
	const double k = M_PI * (2.0 / 250);
	const double f = - M_PI / 2.0;
	x = r2 * cos(iFrame * k + f) + cx;
	y = r2 * sin(iFrame * k + f) + cy;
	cairo_move_to(cr, cx, cy);
	cairo_line_to(cr, x, y);
	cairo_stroke (cr);
	
	char sFrameId[48];
	int t = iFrame / 25;
	int sec = t % 60;
	int min = t / 60;
	sprintf(sFrameId, "Time: %02d:%02d", min, sec);
	
	double hbox = pic->height * 0.1;
	double shift = pic->height * 0.05;
	
	cairo_set_source_rgba (cr, 0x5a /255.0, 0xe8/255.0, 0xf9/255.0, 96/255.0);
	cairo_rectangle (cr, shift, pic->height - hbox - shift, pic->width - 2*shift, hbox);
	cairo_fill (cr);
	
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, hbox * 0.8);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	
	cairo_move_to (cr, shift*1.5, pic->height - shift*1.5);
	cairo_show_text (cr, sFrameId);
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
}

int main(int argc, char *argv[])
{
	const char *fname = argc > 1 ? argv[1] : "out.avi";
	printf("output filename = %s\n", fname);
	
	const int width = 1280;
	const int height = 720;
		
	// INIT
	av_register_all();
	
	FFCVideoOptions opts;
	opts.width = width;
	opts.height = height;
	opts.pix_fmt = AV_PIX_FMT_YUV420P;
	opts.bit_rate = 2000000;
	opts.time_base = (AVRational){ 1, 25 };
	opts.gop_size = 12;
	
	ptr<FFCImage> pic = FFCImage::createImage(width, height);
	if ( pic.getObject() == NULL )
	{
		printf("fail to create FFCImage\n");
		return -1;
	}
	
	ptr<FFCMuxer> muxer = new FFCMuxer();
	
	if ( ! muxer->createFile(fname) )
	{
		printf("failed to createFile(%s)\n", fname);
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
	
	if ( ! vo->openEncoder(&opts) )
	{
		return -1;
	}
	
	av_dump_format(muxer->avFormat, 0, fname, 1);
	
	ptr<FFCScale> scale = new FFCScale();
	scale->init_scale(vo->avFrame, pic->avFrame);
	
	if ( ! muxer->openFile(fname) )
	{
		printf("fail to openFile(%s)\n", fname);
		return -1;
	}
	
	int frameNo = 0;
	while ( 0 )
	{
		frameNo++;
		if ( frameNo > (25 * 30) ) break;
		
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);
		int got_packet = 0;
		
		DrawPic(pic, frameNo);
		
		scale->scale(vo->avFrame, pic->avFrame);
		
		if ( ! vo->encode() )
		{
			printf("frame[%d] encode failed\n", frameNo);
			break;
		}
		
		while ( 1 )
		{
			if ( ! vo->recv_packet(&pkt, got_packet) )
			{
				printf("recv_packet() failed\n");
				return -1;
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
	}
	
	muxer->closeFile();
	
	return 0;
}

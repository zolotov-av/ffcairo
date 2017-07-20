/****************************************************************************

Тестовый исходник для экспериментов

****************************************************************************/


#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>

void check(int r, const char *msg)
{
	if ( r != 0 )
	{
		printf("error: %s\n", msg);
		exit(1);
	}
}

void ModifyFrame(FFCImage *pic, int iFrame)
{
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	char sFrameId[48];
	sprintf(sFrameId, "Frame: %03d", iFrame);
		
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 14);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

	cairo_move_to (cr, 10.0, 135.0);
	cairo_show_text (cr, sFrameId);
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
}

int main(int argc, char *argv[])
{
	// INIT
	av_register_all();
	avformat_network_init();
	
	AVFormatContext *pFormatCtx = NULL;
	
	// Open video file
	check( avformat_open_input(&pFormatCtx, argv[1], NULL, 0), "avformat_open_input fault" );
	
	if ( pFormatCtx == NULL )
	{
		printf("pFormatCtx is NULL\n");
	}
	
	// Retrieve stream information
	if( avformat_find_stream_info(pFormatCtx, NULL) < 0 )
	{
		printf("avformat_find_stream_info failed\n");
		return -1; // Couldn't find stream information
	}
	
	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);
	
	int i;
	AVCodecContext *pCodecCtxOrig = NULL;
	AVCodecContext *pCodecCtx = NULL;
	
	// Find the first video stream
	int videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if( pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO ) {
			videoStream=i;
			break;
		}
	
	if(videoStream==-1) {
		printf("video stream not found\n");
		return -1; // Didn't find a video stream
	}
	
	printf("video stream #%d\n", videoStream);
	
	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	if ( pCodecCtx == NULL ) {
		printf("pCodecCtx is NULL\n");
	}
	
	AVCodec *pCodec = NULL;
	
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if( pCodec == NULL ) {
		printf("Unsupported codec!\n");
		return -1; // Codec not found
	}
	
	pCodecCtxOrig = pCodecCtx;
	
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if( avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0 ) {
		printf("Couldn't copy codec context");
		return -1; // Error copying codec context
	}
	
	AVDictionary *opts = NULL;
	//av_dict_set(&opts, "refcounted_frames", "1", 0);
	
	// Open codec
	if( avcodec_open2(pCodecCtx, pCodec, 0) < 0 ) {
		printf("Could not open codec\n");
		return -1; // Could not open codec
	}
	
	AVFrame *pFrame = NULL;
	
	// Allocate video frame
	pFrame = av_frame_alloc();
	
	printf("video size: %dx%d\n", pCodecCtx->width, pCodecCtx->height);
	
	FFCImage *pic = new FFCImage(pCodecCtx->width, pCodecCtx->height);
	
	struct SwsContext *sws_ctx = NULL;
	int frameFinished;
	AVPacket packet;
	
	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_BGRA, SWS_BILINEAR,
		NULL, NULL, NULL);
	
	i=0;
	while( av_read_frame(pFormatCtx, &packet) >= 0 )
	{
		// Is this a packet from the video stream?
		if( packet.stream_index == videoStream )
		{
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			
			// Did we get a video frame?
			if(frameFinished) {
			// Convert the image from its native format to RGB
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
				pFrame->linesize, 0, pCodecCtx->height,
				pic->avFrame->data, pic->avFrame->linesize);
			
				// Save the frame to disk
				if(++i<10)
				{
					ModifyFrame(pic, i);
					
					// сохраняем фрейм в файл
					char szFilename[48];
					sprintf(szFilename, "out/frame%03d.png", i);
					pic->savePNG(szFilename);
					
					if ( i % 50 == 0 ) printf("#%i\n", i);
				}
				else
				{
					av_free_packet(&packet);
					break;
				}
			}
		}
		
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}
	
	return 0;
}

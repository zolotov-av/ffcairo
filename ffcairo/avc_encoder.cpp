
#include <ffcairo/avc_encoder.h>

/**
* Конструктор
*/
AVCEncoder::AVCEncoder(): avEncoder(NULL), avFrame(NULL)
{
}

/**
* Деструктор
*
* Автоматически закрывает кодек и высобождает ресурсы, если кодек еще
* не был закрыт
*/
AVCEncoder::~AVCEncoder()
{
	closeEncoder();
}

/**
* Открыть кодек
*/
bool AVCEncoder::openVideoEncoder(const FFCVideoOptions *opts)
{
	AVCodec *codec = avcodec_find_encoder(opts->codec_id);
	if ( ! codec )
	{
		printf("codec not found\n");
		return false;
	}
	
	avEncoder = avcodec_alloc_context3(codec);
	avEncoder->width     = opts->width;
	avEncoder->height    = opts->height;
	avEncoder->pix_fmt   = opts->pix_fmt;
	avEncoder->bit_rate  = opts->bit_rate;
	avEncoder->time_base = opts->time_base;
	avEncoder->gop_size  = opts->gop_size;
	
	// TODO не знаю что за фигня, так было в примере
	if ( avEncoder->codec_id == AV_CODEC_ID_MPEG2VIDEO )
	{
		/* just for testing, we also add B frames */
		avEncoder->max_b_frames = 2;
	}
	
	// TODO не знаю что за фигня, так было в примере
	if ( avEncoder->codec_id == AV_CODEC_ID_MPEG1VIDEO )
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		 * This does not happen with normal video, it just happens here as
		 * the motion of the chroma plane does not match the luma plane. */
		avEncoder->mb_decision = 2;
	}
	
	int ret = avcodec_open2(avEncoder, codec, NULL);
	if ( ret < 0 )
	{
		printf("avcodec_open2() failed\n");
		return false;
	}
	
	avFrame = av_frame_alloc();
	if ( !avFrame )
	{
		printf("av_frame_alloc() failed\n");
		return false;
	}
	
	avFrame->width  = opts->width;
	avFrame->height = opts->height;
	avFrame->format = opts->pix_fmt;
	
	// выделить буфер под фрейм, av_frame_get_buffer() самый правильный
	// способ выделить reference-counted буфер
	// TODO второй параметр align - надо разобраться что лучше ставить
	ret = av_frame_get_buffer(avFrame, 1);
	if ( ret < 0 )
	{
		printf("av_frame_get_buffer() failed\n");
		return false;
	}
	
	return true;
}

/**
* Закрыть кодек и высвободить ресурсы
*/
void AVCEncoder::closeEncoder()
{
	// освободить фрейм, указатель будет установлен в NULL
	av_frame_free(&avFrame);
	
	// освободить кодек, указатель будет установлен в NULL
	avcodec_free_context(&avEncoder);
}

/**
* Кодировать кадр хранящийся в avFrame
*/
bool AVCEncoder::encode()
{
	return encode(avFrame);
}

/**
* Кодировать кадр
*/
bool AVCEncoder::encode(AVFrame *frame)
{
	int ret = avcodec_send_frame(avEncoder, frame);
	
	if ( ret == AVERROR(EAGAIN) )
	{
		printf("avcodec_send_frame() == EAGAIN\n");
		return false;
	}
	
	if ( ret < 0 )
	{
		printf("avcodec_send_frame() failed\n");
		return false;
	}
	
	return true;
}

/**
* Завершить кодирование
*/
bool AVCEncoder::sendEof()
{
	return encode(NULL);
}

/**
* Прочитать пакет
*/
bool AVCEncoder::readPacket(AVPacket *pkt, int *got_packet)
{
	int ret = avcodec_receive_packet(avEncoder, pkt);
	if ( ret == 0 )
	{
		*got_packet = 1;
		return true;
	}
	
	*got_packet = 0;
	
	if ( ret == AVERROR(EAGAIN) )
	{
		//printf("avcodec_receive_packet() == EAGAIN\n");
		return true;
	}
	
	printf("avcodec_receive_packet() failed\n");
	return false;
}

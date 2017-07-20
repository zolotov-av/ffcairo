
#include <ffcairo/avc_decoder.h>

/**
* Конструктор
*/
AVCDecoder::AVCDecoder(): avDecoder(NULL), avFrame(NULL)
{
}

/**
* Деструктор
*
* Автоматически закрывает кодек и высобождает ресурсы, если кодек еще
* не был закрыт
*/
AVCDecoder::~AVCDecoder()
{
	closeDecoder();
}

/**
* Открыть кодек
*/
bool AVCDecoder::openDecoder(AVCodecID codec_id, const AVCodecParameters *par)
{
	AVCodec *codec = avcodec_find_decoder(codec_id);
	if( codec == NULL )
	{
		printf("codec not found\n");
		return false; // Codec not found
	}
	
	avDecoder = avcodec_alloc_context3(codec);
	if ( !avDecoder )
	{
		printf("avcodec_alloc_context3() failed\n");
		return false;
	}
	
	int ret;
	
	if ( par )
	{
		ret = avcodec_parameters_to_context(avDecoder, par);
		if ( ret < 0 )
		{
			printf("avcodec_parameters_to_context() failed\n");
			return false;
		}
	}
	
	ret = avcodec_open2(avDecoder, codec, 0);
	if( ret < 0 )
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
	
	// TODO надо ли? и как быть с аудио? полагаю что не надо, нужен иной API
	//avFrame->width  = avDecoder->width;
	//avFrame->height = avDecoder->height;
	//avFrame->format = avDecoder->pix_fmt;
	
	return true;
}

/**
* Закрыть кодек и высвободить ресурсы
*/
void AVCDecoder::closeDecoder()
{
	// освободить фрейм, указатель будет установлен в NULL
	av_frame_free(&avFrame);
	
	// освободить кодек, указатель будет установлен в NULL
	avcodec_free_context(&avDecoder);
}

/**
* Декодировать пакет
*/
bool AVCDecoder::decode(AVPacket *pkt)
{
	int ret = avcodec_send_packet(avDecoder, pkt);
	
	if ( ret == AVERROR(EAGAIN) )
	{
		printf("avcodec_send_packet() == EAGAIN\n");
		return false;
	}
	
	if ( ret < 0 )
	{
		printf("avcodec_send_packet() failed\n");
		return false;
	}
	
	return true;
}

/**
* Завершить кодирование
*/
bool AVCDecoder::sendEof()
{
	return decode(NULL);
}

/**
* Прочитать пакет
*/
bool AVCDecoder::readFrame(int *got_frame)
{
	return readFrame(avFrame, got_frame);
}

/**
* Прочитать пакет
*/
bool AVCDecoder::readFrame(AVFrame *frame, int *got_frame)
{
	int ret = avcodec_receive_frame(avDecoder, frame);
	
	if ( ret == 0 )
	{
		*got_frame = 1;
		return true;
	}
	
	*got_frame = 0;
	
	if ( ret == AVERROR(EAGAIN) )
	{
		//printf("avcodec_receive_frame() == EAGAIN\n");
		return true;
	}
	
	printf("avcodec_receive_frame() failed\n");
	return false;
}

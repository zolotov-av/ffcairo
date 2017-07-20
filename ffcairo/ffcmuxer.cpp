
#include <ffcairo/ffcmuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCVideoOutput::FFCVideoOutput(AVStream *st): avStream(st)
{
}

/**
* Деструктор
*/
FFCVideoOutput::~FFCVideoOutput()
{
}

/**
* Открыть кодек
*/
bool FFCVideoOutput::openEncoder(const FFCVideoOptions *opts)
{
	if ( openVideoEncoder(opts) )
	{
		avStream->time_base = opts->time_base;
		
		int ret = avcodec_parameters_from_context(avStream->codecpar, avEncoder);
		if ( ret < 0 )
		{
			printf("Failed to copy encoder parameters to output stream\n");
			return false;
		}
		
		return true;
	}
	
	return false;
}

/**
* Получить пакет
*/
bool FFCVideoOutput::recv_packet(AVPacket *pkt, int &got_packet)
{
	return readPacket(pkt, &got_packet);
}

/**
* Конструктор
*/
FFCMuxer::FFCMuxer()
{
}

/**
* Деструктор
*/
FFCMuxer::~FFCMuxer()
{
}

/**
* Создать файл
*
* На самом деле эта функция не создает файла, а только подготавливает
* контекст, который надо будет еще донастроить, в частности создать
* потоки и настроить кодеки, после чего вызвать функцию openFile()
* которая начнет запись файла.
*/
bool FFCMuxer::createFile(const char *fname)
{
	avformat_alloc_output_context2(&avFormat, NULL, NULL, fname);
	if ( !avFormat )
	{
		printf("avformat_alloc_output_context2() failed\n");
		return false;
	}
	
	return true;
}

/**
* Создать контекст
*/
bool FFCMuxer::createContext(const char *fmt)
{
	avformat_alloc_output_context2(&avFormat, NULL, fmt, NULL);
	if ( !avFormat )
	{
		printf("avformat_alloc_output_context2() failed\n");
		return false;
	}
	
	return true;
}

/**
* Вернуть кодек по умолчанию для аудио
*/
AVCodecID FFCMuxer::defaultAudioCodec()
{
	return avFormat->oformat->audio_codec;
}

/**
* Вернуть кодек по умолчанию для видео
*/
AVCodecID FFCMuxer::defaultVideoCodec()
{
	return avFormat->oformat->video_codec;
}

/**
* Вернуть кодек по умолчанию для субтитров
*/
AVCodecID FFCMuxer::defaultSubtitleCodec()
{
	return avFormat->oformat->subtitle_codec;
}

/**
* Создать поток
*/
AVStream* FFCMuxer::createStream()
{
	AVStream *avStream = avformat_new_stream(avFormat, NULL);
	if ( ! avStream )
	{
		printf("avformat_new_stream() failed\n");
		return NULL;
	}
	
	printf("stream[%d] created\n", avStream->index);
	return avStream;
}

/**
* Создать поток
*/
AVStream* FFCMuxer::createStream(AVCodecID codec_id)
{
	AVCodec *codec = avcodec_find_encoder(codec_id);
	if ( !codec )
	{
		printf("encoder not found\n");
		return NULL;
	}
	
	AVStream *avStream = avformat_new_stream(avFormat, codec);
	if ( ! avStream )
	{
		printf("avformat_new_stream() failed\n");
		return NULL;
	}
	
	printf("stream[%d] created\n", avStream->index);
	return avStream;
}

/**
* Создать копию потока
*/
AVStream* FFCMuxer::createStreamCopy(AVStream *in_stream)
{
	AVCodecParameters *in_codecpar = in_stream->codecpar;
	
	AVStream *out_stream = avformat_new_stream(avFormat, NULL);
	if ( ! out_stream )
	{
		printf("avformat_new_stream() failed\n");
		return NULL;
	}
	
	int ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
	if (ret < 0)
	{
		printf("avcodec_parameters_copy() failed\n");
	}
	
	out_stream->codecpar->codec_tag = 0;
	
	return out_stream;
}

/**
* Создать видео-поток
*/
FFCVideoOutput* FFCMuxer::createVideoStream(const FFCVideoOptions *opts)
{
	AVStream *avStream = createStream(opts->codec_id);
	if ( !avStream )
	{
		printf("fail to create video stream\n");
		return NULL;
	}
	
	FFCVideoOutput *vo = new FFCVideoOutput(avStream);
	
	return vo;
}

/**
* Открыть файл
*
* Открывает файл и записывает заголовки, перед вызовом должен
* быть создан контекст, потоки, настроены кодеки.
*/
bool FFCMuxer::openFile(const char *fname)
{
	int ret;
	
	/* open the output file, if needed */
	if (!(avFormat->oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&avFormat->pb, fname, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			fprintf(stderr, "Could not open '%s'\n", fname);
			return false;
		}
	}
	
	ret = avformat_write_header(avFormat, NULL);
	if ( ret < 0 )
	{
		fprintf(stderr, "avformat_write_header() failed\n");
		return false;
	}
	
	return true;
}

/**
* Открыть файл
*
* Открывает файл (через AVIO) и записывает заголовки, перед вызовом должен
* быть создан контекст, потоки, настроены кодеки.
*/
bool FFCMuxer::openAVIO()
{
	int ret;
	
	/* open the output file, if needed */
	/*
	if (!(avFormat->oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&avFormat->pb, fname, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			fprintf(stderr, "Could not open '%s'\n", fname);
			return false;
		}
	}
	*/
	
	ret = avformat_write_header(avFormat, NULL);
	if ( ret < 0 )
	{
		fprintf(stderr, "avformat_write_header() failed\n");
		return false;
	}
	
	return true;
}

/**
* Записать пакет
*/
bool FFCMuxer::writeFrame(AVPacket *pkt)
{
	int ret = av_interleaved_write_frame(avFormat, pkt);
	return ret == 0;
}

/**
* Закрыть файл
*
* Записывает финальные данные и закрывает файл.
*/
bool FFCMuxer::closeFile()
{
	/* Write the trailer, if any. The trailer must be written before you
	 * close the CodecContexts open when you wrote the header; otherwise
	 * av_write_trailer() may try to use memory that was freed on
	 * av_codec_close(). */
	av_write_trailer(avFormat);
	
	avformat_free_context(avFormat);
}

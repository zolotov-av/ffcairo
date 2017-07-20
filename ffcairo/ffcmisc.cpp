
#include <ffcairo/ffcmisc.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCStreamCopy::FFCStreamCopy()
{
}

/**
* Деструктор
*/
FFCStreamCopy::~FFCStreamCopy()
{
}

/**
* Обработчик присоединения
*
* Автоматически вызывается когда поток присоединяется к демультиплексору
*/
void FFCStreamCopy::handleAttach(AVStream *st)
{
}

/**
* Обработчик отсоединения
*
* Автоматически вызывается когда поток отсоединяется от демультиплексора
*/
void FFCStreamCopy::handleDetach()
{
}

/**
* Обработчик пакета
*/
void FFCStreamCopy::handlePacket(AVPacket *packet)
{
	
}

/**
* Создать выходной поток в мультиплексоре и настроить его для копирования
*/
AVStream* FFCStreamCopy::createStreamCopy()
{
	/*
	AVStream *oStream = muxer->createStream();
	
	// копируем параметры кодека
	AVCodecParameters *in_codecpar = avStream->codecpar;
	int ret = avcodec_parameters_copy(oStream->codecpar, in_codecpar);
	if (ret < 0) {
		fprintf(stderr, "Failed to copy codec parameters\n");
		return oStream;
	}
	oStream->codecpar->codec_tag = 0;
	
	av_dump_format(muxer->avFormat, oStream->index, NULL, 1);
	*/
}

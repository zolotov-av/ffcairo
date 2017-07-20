
#include <ffcairo/ffcdemuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCVideoInput::FFCVideoInput(): avStream(NULL)
{
}

/**
* Деструктор
*/
FFCVideoInput::~FFCVideoInput()
{
}

/**
* Обработчик присоединения
*
* Автоматически вызывается когда поток присоединяется к демультиплексору
*/
void FFCVideoInput::handleAttach(AVStream *st)
{
	printf("handleAttach()\n");
	avStream = st;
}

/**
* Обработчик отсоединения
*
* Автоматически вызывается когда поток отсоединяется от демультиплексора
*/
void FFCVideoInput::handleDetach()
{
	printf("handleDetach()\n");
	
	closeDecoder();
	
	avStream = NULL;
}

/**
* Открыть кодек
*/
bool FFCVideoInput::openDecoder()
{
	AVCodecID codec_id = avStream->codecpar->codec_id;
	const AVCodecParameters *par = avStream->codecpar;
	
	if ( AVCDecoder::openDecoder(codec_id, par) )
	{
		avFrame->width  = avDecoder->width;
		avFrame->height = avDecoder->height;
		avFrame->format = avDecoder->pix_fmt;
		
		return true;
	}
	
	return false;
}

/**
* Обработчик пакета
*/
void FFCVideoInput::handlePacket(AVPacket *packet)
{
	if ( decode(packet) )
	{
		int got_frame;
		do
		{
			if ( ! readFrame(&got_frame) ) return;
			if ( got_frame ) handleFrame();
		}
		while ( got_frame );
	}
}

/**
* Конструктор
*/
FFCDemuxer::FFCDemuxer(): avFormat(NULL), stream_count(0), streams(NULL)
{
}

/**
* Деструктор
*/
FFCDemuxer::~FFCDemuxer()
{
	if ( streams ) delete [] streams;
}

/**
* Найти видео-поток
*
* Возвращает ID потока или -1 если не найден
*/
int FFCDemuxer::findVideoStream()
{
	int videoStream = -1;
	printf("stream's count: %d\n", stream_count);
	for(int i=0; i<stream_count; i++)
	{
		if( avFormat->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO )
		{
			videoStream = i;
			break;
		}
	}
	
	if(videoStream==-1)
	{
		fprintf(stderr, "video stream not found\n");
		return -1;
	}
	
	return videoStream;
}

/**
* Создать контекст
*/
bool FFCDemuxer::createContext()
{
	avFormat = avformat_alloc_context();
	if ( !avFormat )
	{
		printf("avformat_alloc_context() failed\n");
		return false;
	}
	
	return true;
}

/**
* Открыть поток
*
* Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
*/
bool FFCDemuxer::open(const char *uri)
{
	/*
	 * TODO something...
	 * Здесь avFormat должен быть NULL, а что если он не-NULL?
	 * непонятно, можно ли повторно вызывать avformat_open_input()
	 * или надо пересоздавать контекст...
	 */
	
	// Открываем файл/URL
	if ( avformat_open_input(&avFormat, uri, NULL, 0) != 0 )
	{
		fprintf(stderr, "avformat_open_input(%s) fault\n", uri);
		return false;
	}
	
	// Извлечь информацию о потоке
	if( avformat_find_stream_info(avFormat, NULL) < 0 )
	{
		fprintf(stderr, "avformat_find_stream_info(%s) failed\n", uri);
		return false;
	}
	
	// Вывести информацию о потоке на стандартный вывод (необязательно)
	av_dump_format(avFormat, 0, uri, 0);
	
	stream_count = avFormat->nb_streams;
	streams = new ptr<FFCInputStream>[stream_count];
	for(int i = 0; i < stream_count; i++) streams[i] = NULL;
	
	printf("Поток[%s] успешно открыт\n", uri);
	
	return true;
}

/**
* Открыть поток через AVIO
*
* NOTE асинхронная (неблокирующая) работа при открытии не поддерживается
* самим FFMPEG, так что прежде чем вызывать openAVIO() нужно быть
* уверенным что уже поступило достаточно данных чтобы прочитать заголовки
* файла и расчитать частоту кадров. Более того, должны поступить не только
* заголовки, но и несколько первых фреймов, т.к. FFMPEG еще читает
* какое-то количество фреймов чтобы расчитать FPS. Будем надеятся в
* следующих версиях они сделают полноценную поддержку неблокирующего
* режима, по крайней мере в avio.h заявлено что такая работа ведется,
* см. описание AVIO_FLAG_NONBLOCK:
*   Warning: non-blocking protocols is work-in-progress; this flag may be
*   silently ignored.
*/
int FFCDemuxer::openAVIO()
{
	// открываем "файл"
	int ret = avformat_open_input(&avFormat, NULL, NULL, NULL);
	if ( ret < 0 )
	{
		printf("avformat_open_input() fault\n");
		return ret;
	}
	
	// Извлечь информацию о потоке
	ret = avformat_find_stream_info(avFormat, NULL);
	if( ret < 0 )
	{
		printf("avformat_find_stream_info() failed\n");
		return ret;
	}
	
	stream_count = avFormat->nb_streams;
	streams = new ptr<FFCInputStream>[stream_count];
	for(int i = 0; i < stream_count; i++) streams[i] = NULL;
	
	// Вывести информацию о потоке на стандартный вывод (необязательно)
	av_dump_format(avFormat, 0, NULL, 0);
	
	return 0;
}

/**
* Попытаться прочитать заголовки потока
* 
* используется только совместно с openAVIO() т.к. в асинхронном режиме
* возможно что данные заголовков еще не поступили и надо ждать
*/
int FFCDemuxer::findStreamInfo()
{
	// Извлечь информацию о потоке
	int ret = avformat_find_stream_info(avFormat, NULL);
	
	if ( ret == AVERROR(EAGAIN) )
	{
		fprintf(stderr, "avformat_find_stream_info() EAGAIN\n");
		return ret;
	}
	
	if( ret < 0 )
	{
		fprintf(stderr, "avformat_find_stream_info() failed\n");
		return ret;
	}
	
	// Вывести информацию о потоке на стандартный вывод (необязательно)
	av_dump_format(avFormat, 0, 0, 0);
	
	stream_count = avFormat->nb_streams;
	streams = new ptr<FFCInputStream>[stream_count];
	for(int i = 0; i < stream_count; i++) streams[i] = NULL;
	
	printf("Поток успешно открыт\n");
	
	return 0;
}

/**
* Присоединить приемник потока
*/
void FFCDemuxer::bindStream(int i, ptr<FFCInputStream> st)
{
	if ( i >= 0 && i < stream_count )
	{
		if ( streams[i].getObject() )
		{
			streams[i]->handleDetach();
		}
		
		streams[i] = st;
		
		if ( st.getObject() )
		{
			st->handleAttach(avFormat->streams[i]);
		}
	}
}

/**
* Обработать фрейм
*/
bool FFCDemuxer::readFrame()
{
	AVPacket packet;
	packet.data = NULL;
	packet.size = 0;
	av_init_packet(&packet);
	
	int r = av_read_frame(avFormat, &packet);
	if ( r < 0 )
	{
		if ( r == AVERROR(EAGAIN) )
		{
			//printf("av_read_frame() == EAGAIN\n");
			return false;
			
		}
		// конец файла или ошибка
		printf("av_read_frame() failed\n");
		return false;
	}
	
	static int i =0;
	i++;
	//printf("got_packet #%d\n", i);
	
	int id = packet.stream_index;
	if ( id >= 0 && id < stream_count )
	{
		if ( streams[id].getObject() )
		{
			streams[id]->handlePacket(&packet);
		}
	}
	
	// Free the packet that was allocated by av_read_frame
	av_packet_unref(&packet);
	
	return true;
}

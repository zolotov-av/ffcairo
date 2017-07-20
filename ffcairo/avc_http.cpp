
#include <ffcairo/avc_http.h>
#include <ffcairo/avc_scene.h>

#include <ctype.h>
#include <string>

/**
* Конструктор
*/
AVCHttp::AVCHttp(int afd, AVCEngine *e): AsyncStream(afd), engine(e)
{
	http_state = READ_METHOD;
	line.clear();
	done = false;
	peer_down = false;
	first_pts = AV_NOPTS_VALUE;
	skip_to_keyframe = true;
}

/**
* Деструктор
*/
AVCHttp::~AVCHttp()
{
	printf("AVCHttp::~AVCHttp()\n");
	close();
}

/**
* Обработчик прочитанных данных
*/
void AVCHttp::onRead(const char *data, size_t len)
{
	const char *p = data;
	const char *lim = data + len;
	
	while ( http_state == READ_METHOD || http_state == READ_HEADERS )
	{
		while ( *data != '\n' && data < lim ) data ++;
		line += std::string(p, data);
		if ( data >= lim ) return;
		data++;
		p = data;
		if ( http_state == READ_METHOD )
		{
			method = line;
			printf("method: %s\n", method.c_str());
			http_state = READ_HEADERS;
			line.clear();
		}
		else
		{
			if ( line == "" || line == "\r" )
			{
				http_state = READ_BODY;
			}
			else
			{
				printf("header: %s\n", line.c_str());
				line.clear();
			}
		}
	}
	
	if ( http_state == READ_BODY )
	{
		printf("read body\n");
		if ( initStreaming() )
		{
			http_state = STREAMING;
		}
		else
		{
			http_state = FAILED_STATE;
		}
	}
}

/**
* Обработчик события опустошения выходного буфера
*
* Вызывается после того как все данные были записаны в файл/сокет
*/
void AVCHttp::onEmpty()
{
	if ( done )
	{
		printf("onEmtpy done, leave\n");
		engine->scene->removeHttpClient(this);
		leaveDaemon();
	}
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCHttp::onPeerDown()
{
	peer_down = true;
	printf("AVCHttp::onPeerDown()\n");
	engine->scene->removeHttpClient(this);
	leaveDaemon();
}


void AVCHttp::write(const std::string &s)
{
	put(s.c_str(), s.size());
}

/**
* Инициализация стриминга
*/
bool AVCHttp::initStreaming()
{
	std::string error;
#if 1
	do
	{
		muxer = new FFCMuxer();
		
		if ( ! muxer->createContext("avi") )
		{
			error = "failed to createContext()";
			break;
		}
		
		uint8_t *avio_ctx_buffer = NULL;
		size_t avio_ctx_buffer_size = 4096;
		
		avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
		if (!avio_ctx_buffer)
		{
			error = "av_malloc failed";
			break;
		}
	
		avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, this/*user pointer*/, NULL, &write_packet, NULL);
		if ( !avio_ctx )
		{
			error = "avio_alloc_context() failed";
			break;
		}
		
		muxer->avFormat->pb = avio_ctx;
		
		AVStream *in_stream = engine->scene->vo->avStream;
		
		in_time_base = in_stream->time_base;
		
		vo = muxer->createStreamCopy(in_stream);
		
		av_dump_format(muxer->avFormat, 0, 0, 1);
		
		write("HTTP/1.0 200 OK\r\n");
		write("Content-type: video/x-flv\r\n");
		write("\r\n");
		
		if ( ! muxer->openAVIO() )
		{
			error = "fail to openFile()\n";
			break;
		}
		
		return true;
	}
	while ( 0 );
#endif
	write("HTTP/1.0 200 OK\r\n");
	write("Content-type: text/plain\r\n");
	write("\r\n");
	if ( error != "" )
	{
		write("<error>");
		write(error);
		write("</error>");
	}
	else
	{
		write("<ok>Hello world</ok>");
	}
	done = true;
}

/**
* Обработчик записи пакета в поток
*/
int AVCHttp::write_packet(void *opaque, uint8_t *buf, int buf_size)
{
	AVCHttp *s = (AVCHttp*)opaque;
	if ( s->put((const char *)buf, buf_size) ) return buf_size;
	else return 0;
}

/**
* Отправить пакет в стрим
*/
int AVCHttp::sendPacket(AVPacket *pkt)
{
	if ( done )
	{
		printf("AVCHttp::sendPacket() after done\n");
		return AVERROR_UNKNOWN;
	}
	
	if ( peer_down )
	{
		printf("AVCHttp::sendPacket() after peer_down\n");
		return AVERROR_UNKNOWN;
	}
	
	if ( http_state != STREAMING )
	{
		printf("AVCHttp::sendPacket() in wrong state\n");
		return AVERROR_UNKNOWN;
	}
	
	if ( skip_to_keyframe )
	{
		if ( pkt->flags & AV_PKT_FLAG_KEY )
		{
			/*
			printf("got first key frame\n");
			printf("dts: %"PRId64"\n", pkt->dts);
			printf("pts: %"PRId64"\n", pkt->pts);
			printf("pos: %"PRId64"\n", pkt->pos);
			printf("in_stream.time_base = { %d, %d }\n", engine->scene->vo->avStream->time_base.num, engine->scene->vo->avStream->time_base.den);
			printf("out_stream.time_base = { %d, %d }\n", vo->time_base.num, vo->time_base.den);
			*/
			
			skip_to_keyframe = false;
			first_pts = pkt->dts;
			//printf("first_pts: %"PRId64"\n", first_pts);
		}
		else
		{
			// drop packet
			//printf("drop not key frame\n");
			return 0;
		}
	}
	
	pkt->pos = -1;
	
	if ( pkt->dts != AV_NOPTS_VALUE )
	{
		pkt->dts = av_rescale_q(pkt->dts - first_pts, engine->scene->vo->avStream->time_base, vo->time_base);
	}
	
	if ( pkt->pts != AV_NOPTS_VALUE )
	{
		pkt->pts = av_rescale_q(pkt->pts - first_pts, engine->scene->vo->avStream->time_base, vo->time_base);
	}
	
	int ret = av_write_frame(muxer->avFormat, pkt);
	if ( ret < 0 )
	{
		printf("av_write_frame() failed\n");
		return ret;
	}
	
	return 0;
}

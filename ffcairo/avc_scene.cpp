
#include <ffcairo/avc_scene.h>
#include <ffcairo/avc_http.h>
#include <ffcairo/avc_channel.h>
#include <ffcairo/avc_time.h>
#include <ffcairo/ffcimage.h>

/**
* Конструктор
*/
AVCScene::AVCScene(): http_count(0), feed(NULL)
{
	memset(&opts, 0, sizeof(opts));
}

/**
* Деструктор
*/
AVCScene::~AVCScene()
{
	printf("AVCScene::~AVCScene()\n");
	if ( avio_ctx )
	{
		unsigned char *buf = avio_ctx->buffer;
		if ( buf ) av_free(buf);
		av_free(avio_ctx);
	}
}

/**
* Обработчик записи пакета в поток
*/
int AVCScene::write_packet(void *opaque, uint8_t *buf, int buf_size)
{
	//printf("AVCScene::write_packet(%d)\n", buf_size);
	// возможно тут будет отправка контента на сервер стриминга (twitch,
	// youtube, icecast или что-то типа того), но пока данные просто
	// дропаются, а фактический вывод осуществляется через AVCHttp
	// через копирование и ремультиплексирование
	return buf_size;
}

/**
* Отправить фрейм в кодировщик
*/
int AVCScene::sendFrame()
{
	AVPacket pkt = { 0 };
	pkt.data = NULL;
	pkt.size = 0;
	av_init_packet(&pkt);
	
	int got_packet = 0;
	
	if ( ! scale->scale(vo->avFrame, pic->avFrame) )
	{
		printf("scale() failed\n");
	}
	
	vo->avFrame->pts = av_rescale_q(curr_pts - start_pts, ms_time_base, vo->avStream->time_base);
	//printf("avFrame->pts: %d\n", (int)vo->avFrame->pts);
	
	if ( ! vo->encode() )
	{
		printf("frame[%d] encode failed\n", iFrame);
		failStreaming();
		return AVERROR_UNKNOWN;
	}
	
	while ( 1 )
	{
		if ( ! vo->recv_packet(&pkt, got_packet) )
		{
			printf("recv_packet() failed\n");
			failStreaming();
			return AVERROR_UNKNOWN;
		}
		//printf("recv_packet ok, got_packet = %s\n", (got_packet ? "yes" : "no"));
		
		if ( got_packet )
		{
			sendPacket(&pkt);
			av_packet_unref(&pkt);
			got_packet = 0;
		}
		else
		{
			break;
		}
	}
	return 0;
}

/**
* Отправить пакет в стрим
*/
int AVCScene::sendPacket(AVPacket *pkt)
{
	int64_t dts = pkt->dts;
	int64_t pts = pkt->pts;
	int64_t pos = pkt->pos;
	
	// отправляем пакет в основной стрим (в write_packet)
	// (в будущем) на twitch, youtube, icecast или что-то типа того
	int ret = av_write_frame(avFormat, pkt);
	if ( ret < 0 )
	{
		printf("av_write_frame() failed\n");
		return ret;
	}
	
	// отправляем пакет HTTP-клиентам
	for(int i = 0; i < MAX_HTTP_CLIENTS; i++)
	{
		if ( http_clients[i].getObject() )
		{
			pkt->dts = dts;
			pkt->pts = pts;
			pkt->pos = pos;
			http_clients[i]->sendPacket(pkt);
		}
	}
	
	return 0;
}

/**
* Создать контекст с AVIO
*/
int AVCScene::createAVIO(const char *format)
{
	int ret = avformat_alloc_output_context2(&avFormat, NULL, format, NULL);
	if ( ret < 0 )
	{
		printf("avformat_alloc_output_context2() failed\n");
		return ret;
	}
	if ( ! avFormat )
	{
		printf("avFormat=NULL\n");
		return AVERROR_UNKNOWN;
	}
	
	uint8_t *avio_ctx_buffer = NULL;
	size_t avio_ctx_buffer_size = 4096;
	
	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer)
	{
		printf("av_malloc failed\n");
		return AVERROR_UNKNOWN;
	}

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, this/*user pointer*/, NULL, &write_packet, NULL);
	if ( !avio_ctx )
	{
		printf("avio_alloc_context() failed\n");
		return AVERROR_UNKNOWN;
	}
	
	avFormat->pb = avio_ctx;
	
	return 0;
}

/**
* Инициализация стриминга
*/
int AVCScene::initStreaming(int width, int height, int64_t bit_rate)
{
	opts.width = width;
	opts.height = height;
	opts.pix_fmt = AV_PIX_FMT_YUV420P;
	opts.bit_rate = bit_rate;
	opts.time_base = (AVRational){ 1, 25 };
	opts.gop_size = 12;
	
	pic = FFCImage::createImage(width, height);
	if ( pic.getObject() == NULL )
	{
		printf("fail to create FFCImage\n");
		return AVERROR_UNKNOWN;
	}
	
	int ret = createAVIO("avi");
	if ( ret < 0 )
	{
		printf("failed to createAVI(avi)\n");
		return ret;
	}
	
	// add video stream
	AVCodecID video_codec = defaultVideoCodec();
	if ( video_codec == AV_CODEC_ID_NONE)
	{
		printf("error: video_codec = NONE\n");
		return AVERROR_UNKNOWN;
	}
	
	opts.codec_id = video_codec;
	
	vo = createVideoStream(&opts);
	
	if ( ! vo->openEncoder(&opts) )
	{
		printf("openEncoder() failed\n");
		return AVERROR_UNKNOWN;
	}
	
	av_dump_format(avFormat, 0, 0, 1);
	
	scale = new FFCScale();
	if ( ! scale->init_scale(vo->avFrame, pic->avFrame) )
	{
		printf("init_scale() failed\n");
		return AVERROR_UNKNOWN;
	}
	
	if ( ! openAVIO() )
	{
		printf("fail to openAVIO()\n");
		return AVERROR_UNKNOWN;
	}
	
	iFrame = 0;
	int64_t ms = ms_time();
	start_pts = ms + 40; // планируемое время первого фрейма
	next_pts = ms + 38; // позволим таймеру срабатывать чуть раньше
	
	return 0;
}

/**
* Завершить стриминг
*
* Временная функция для прерывания стрима в случае непредвиденных ошибок
*/
void AVCScene::failStreaming()
{
}

/**
* Выпустить новый фрейм и отправить его в стриминг
*/
void AVCScene::emitFrame()
{
	// по началу таймер запускается раз в секунду, и соответственно
	// он должен сгенерировать сразу 25 кадров, соответственно большая часть
	// кадров формируется задним числом. В будущем попробуем увеличить частоту
	// таймера и генерировать все карды в реальном времени
	
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	const int width = pic->width;
	const int height = pic->height;
	
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
	
	char sTime[48];
	char sFrameId[80];
	int t = iFrame / 25;
	int sec = t % 60;
	int min = t / 60;
	
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime(sTime, sizeof(sTime),"Time %H:%M:%S",timeinfo);
	sprintf(sFrameId, "%s   HTTP clients: %d", sTime, http_count);
	
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
	
	if ( feed )
	{
		double shift = pic->width * 0.03;
		double cam_x = pic->width - shift - feed->pic->width;
		double cam_y = shift;
		cairo_set_source_surface (cr, feed->pic->surface, cam_x, cam_y);
		cairo_paint (cr);
	}
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
	
	sendFrame();
}

/**
* Таймер
*
* Таймер должен запускаться не менее раз в 40мс
*/
void AVCScene::onTimer(const timeval *tv)
{
	int64_t ms = ms_time();
	if ( ms >= next_pts )
	{
		curr_pts = ms;
		next_pts += 40;
		iFrame++;
		emitFrame();
	}
}

/**
* Клиента HTTP
*/
bool AVCScene::addHttpClient(AVCHttp *client)
{
	for(int i = 0; i < MAX_HTTP_CLIENTS; i++)
	{
		if ( http_clients[i].getObject() == NULL )
		{
			http_clients[i] = client;
			http_count++;
			printf("new HTTP client, new count %d of %d\n", http_count, MAX_HTTP_CLIENTS);
			return true;
		}
	}
	
	printf("addHttpClient() failed\n");
	return false;
}

/**
* Удалить клиента HTTP
*/
void AVCScene::removeHttpClient(AVCHttp *client)
{
	for(int i = 0; i < MAX_HTTP_CLIENTS; i++)
	{
		if ( http_clients[i].getObject() == client )
		{
			http_clients[i] = NULL;
			http_count--;
			printf("removed HTTP client, new count %d of %d\n", http_count, MAX_HTTP_CLIENTS);
		}
	}
}

/**
* Заменить фидера
*
* Устанавливает фидера, если фидер уже был установлен,то заменяет его,
* а старого кикает с сервера
*
* Временная функция
*/
void AVCScene::replaceFeed(AVCChannel *ch)
{
	printf("AVCScene::replaceFeed()\n");
	if ( feed ) feed->handleKick();
	feed = ch;
}

/**
* Удалить фидера
*
* Если текущий фидер равен указанному, то удалить фидер, если не
* совпадает, то ничего не делать
*
* Временная функция
*/
void AVCScene::removeFeed(AVCChannel *ch)
{
	if ( ch && feed == ch )
	{
		feed = NULL;
		ch->handleKick();
	}
}

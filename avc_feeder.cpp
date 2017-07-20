/****************************************************************************

Клиент (фидер) для сервера видеоконференций

Фидер это клиент который отправляет на сервер свой видеопоток для участия
в видеоконференции, но при этом не является полноценным клиентом. Этот клиент
нам нужен на раннем этапе для тестирования и отладки, т.к. полноценной
инфраструктуры у нас пока нет.

****************************************************************************/

#include <nanosoft/logger.h>
#include <nanosoft/debug.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/asyncdns.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/avc_listen.h>
#include <ffcairo/ffctypes.h>
#include <ffcairo/avc_http.h>
#include <ffcairo/avc_feedagent.h>

#include <stdio.h>

struct opts_t
{
	bool fork;
};

opts_t opts;

/**
 * Парсер опций
 */
void parse_options(int argc, char** argv)
{
	// init default options
	opts.fork = false;
	
	for(int i = 1; i < argc; i++)
	{
		if ( strcmp(argv[i], "-f") == 0 )
		{
			opts.fork = true;
			continue;
		}
		
		fprintf(stderr, "unknown option [%s]\n", argv[i]);
	}
}

/**
 * Форкнуть демона
 */
void forkDaemon()
{
	printf("fork daemon\n");
	
	pid_t pid = fork();
	
	// Если полученный родительским процессом pid меньше 0, значит, форкнуться не удалось
	if ( pid < 0 )
	{
		logger.error("fork failure: %s", strerror(errno));
		return;
	}
	
	if ( pid == 0 )
	{
		// потомок
		return;
	}
	else
	{
		// родитель
		logger.information("daemon forked, PID: %u", pid);
		exit(0);
	}
}

AVCFeedAgent *agent;

void onTimer(const timeval &tv, NetDaemon* daemon)
{
	// нам нужен таймер 40мс
	// TODO на самом деле нам нужно принципиальное другое планирование времени
	// надо чтобы обработчик запускался по определенным временным меткам,
	// а для этого придется влезать в NetDaemon
	int ts = tv.tv_sec;
	int ms = tv.tv_usec / 1000;
	int ticktime = ((ts % 1000) * 1000 + ms) / 100;
	
	static int prevtime = 0;
	static int iFrame = 0;
	if ( ticktime != prevtime )
	{
		prevtime = ticktime;
		
		iFrame++;
		agent->onTimer(tv);
	}
	
	static int old_ts = 0;
	static int old_ifr = 0;
	if ( ts != old_ts )
	{
		old_ts = ts;
		printf("fps: %d\n", iFrame - old_ifr);
		old_ifr = iFrame;
	}
}

int main(int argc, char** argv)
{
	const char *fname = argc > 1 ? argv[1] : "/dev/video0";
	printf("device filename = %s\n", fname);
	
	logger.open("avc_feeder.log");
	logger.information("avc_feeder start");
	
	// загружаем опции отладки из переменных окружения
	DEBUG::load_from_env();
	
	parse_options(argc, argv);
	
	if ( opts.fork ) forkDaemon();
	
	// INIT
	av_register_all();
	
	NetDaemon daemon(100, 1024);
	
	// нам нужные быстрые таймеры, при FPS=25 нужно запускать таймер
	// каждые 40мс
	daemon.setSleepTime(10);
	
	ptr<AVCFeedAgent> avc_agent = agent = new AVCFeedAgent(&daemon);
	
	int ret = avc_agent->openWebcam(fname);
	if ( ret < 0 )
	{
		printf("openWebcam() failed\n");
		return -1;
	}
	
	if ( ! avc_agent->openVideoStream() )
	{
		printf("failed to open input video stream\n");
		return -1;
	}
	
	ret = avc_agent->openFeed(avc_agent->opts.width, avc_agent->opts.height, 2000000);
	if ( ret < 0 )
	{
		printf("failed to open output video stream\n");
		return -1;
	}
	
	avc_agent->createSocket();
	
	adns = new AsyncDNS(&daemon);
	
	daemon.addObject(adns);
	daemon.addObject(avc_agent);
	
	avc_agent->connectTo("127.0.0.1", 8001);
	//avc_agent->enableObject();
	
	daemon.setGlobalTimer(onTimer, &daemon);
	
	
	avc_payload_t p;
	p.type = AVC_SIMPLE;
	p.channel = 1;
	const char *s = "Hello world";
	int plen = strlen(s) + 5;
	strcpy(p.buf, s);
	avc_set_packet_len(&p, plen);
	printf("plen=%d, avc_packet_len()=%d\n", plen, avc_packet_len(&p));
	avc_agent->sendPacket(&p);
	
	daemon.run();
	
	logger.information("avc_feeder exit");
	printf("avc_feeder exit\n");
	return 0;
}

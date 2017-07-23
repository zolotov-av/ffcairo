
#include <ffcairo/avc_listen.h>

#include <ffcairo/avc_channel.h>
#include <ffcairo/avc_scene.h>
#include <ffcairo/avc_http.h>

/**
* Конструктор
*/
AVCListen::AVCListen(AVCEngine *e, AVCProtocol p): engine(e), proto(p)
{
}

/**
* Принять входящее соединение
*/
void AVCListen::onAccept()
{
	int sock = accept();
	
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;
	
	len = sizeof addr;
	getpeername(sock, (struct sockaddr *) &addr, &len);
	
	struct sockaddr_in *s = (struct sockaddr_in *) &addr;
	port = ntohs(s->sin_port);
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	
	//logger.information("new connection from %s", ipstr);
	printf("new connection from %s\n", ipstr);
	
	if ( proto == AVC_CHANNEL )
	{
		ptr<AVCChannel> ch = new AVCChannel(sock, engine);
		engine->daemon->addObject(ch);
		ch->init();
		return;
	}
	
	if ( proto == AVC_HTTP )
	{
		ptr<AVCHttp> ch = new AVCHttp(sock, engine);
		if ( engine->scene->addHttpClient(ch.getObject()) )
		{
			engine->daemon->addObject(ch);
			return;
		}
	}
	
	::close(sock);
}

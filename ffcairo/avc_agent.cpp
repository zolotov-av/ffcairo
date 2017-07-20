
#include <ffcairo/avc_agent.h>
#include <nanosoft/logger.h>

/**
* Конструктор
*/
AVCAgent::AVCAgent(NetDaemon *d): AVCStream(-1), daemon(d)
{
}

/**
* Деструктор
*/
AVCAgent::~AVCAgent()
{
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCAgent::onPeerDown()
{
	// TODO daemon->exit();
	exit(1);
}

/**
* Обработчик пакета
*/
void AVCAgent::onPacket(const avc_packet_t *pkt)
{
}

/**
* Создать сокет
*
* Временная функция...
*/
bool AVCAgent::createSocket()
{
	// создадим новый сокет
	int sock = ::socket(PF_INET, SOCK_STREAM, 0);
	if ( sock == -1 )
	{
		// пичаль, попробуем в другой раз
		logger.unexpected("AVCAgent::createSocket() cannot create socket");
		return false;
	}
	
	setFd( sock );
	
	return true;
}

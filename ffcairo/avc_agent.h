#ifndef AVC_AGENT_H
#define AVC_AGENT_H

#include <ffcairo/avc_stream.h>

/**
 * Канал (сетевой) сервера видеоконференций
 *
 * Данный класс описывают клиентскую часть канала
 */
class AVCAgent: public AVCStream
{
public:
	
	/**
	 * Ссылка на демона
	 */
	NetDaemon *daemon;
	
	/**
	 * Конструктор
	 */
	AVCAgent(NetDaemon *d);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCAgent();
	
protected:
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
	/**
	 * Обработчик пакета
	 */
	virtual void onPacket(const AVCPacket *pkt);
	
public:
	
	/**
	 * Создать сокет
	 *
	 * Временная функция...
	 */
	bool createSocket();
};

#endif // AVC_AGENT_H
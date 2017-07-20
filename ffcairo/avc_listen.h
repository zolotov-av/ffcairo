#ifndef AVC_LISTEN_H
#define AVC_LISTEN_H

#include <nanosoft/asyncserver.h>
#include <ffcairo/avc_engine.h>

enum AVCProtocol {
	AVC_CHANNEL,
	AVC_HTTP
};

class AVCListen: public AsyncServer
{
public:
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	/**
	 * Протокол
	 */
	AVCProtocol proto;
	
	/**
	 * Конструктор
	 */
	AVCListen(AVCEngine *e, AVCProtocol p = AVC_CHANNEL);
	
protected:
	
	/**
	* Принять входящее соединение
	*/
	virtual void onAccept();
};

#endif // AVC_LISTEN_H

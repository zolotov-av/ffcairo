#ifndef AVC_ENGINE_H
#define AVC_ENGINE_H

#include <nanosoft/netdaemon.h>

class AVCScene;
class AVCHttp;

class AVCEngine
{
public:
	/**
	 * Ссылка на движок NetDaemon
	 */
	NetDaemon *daemon;
	
	/**
	 * Сцена
	 */
	ptr<AVCScene> scene;
	
	/**
	 * временная ссылка на http-поток, по хорошему надо список
	 */
	ptr<AVCHttp> http2;
	
	/**
	 * Конструктор
	 */
	AVCEngine(NetDaemon *d);
	
	/**
	 * Деструктор
	 */
	~AVCEngine();
};

#endif // AVC_ENGINE_H

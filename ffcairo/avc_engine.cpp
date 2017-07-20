
#include <ffcairo/avc_engine.h>

#include <ffcairo/avc_scene.h>
#include <ffcairo/avc_http.h>

/**
* Конструктор
*/
AVCEngine::AVCEngine(NetDaemon *d): daemon(d), http2(NULL)
{
	scene = new AVCScene();
}

/**
* Деструктор
*/
AVCEngine::~AVCEngine()
{
}

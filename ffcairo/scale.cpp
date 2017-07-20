
#include <ffcairo/scale.h>

/**
* Конструктор
*/
FFCScale::FFCScale(): ctx(NULL)
{
}

/**
* Деструктор
*/
FFCScale::~FFCScale()
{
	close_scale();
}

/**
* Инициализация маштабирования
*
* При необходимости сменить настройки маштабирования, init_scale()
* можно вывызывать без предварительного закрытия через close_scale()
*/
bool FFCScale::init_scale(AVFrame *dst, AVFrame *src)
{
	ctx = sws_getCachedContext(ctx,
		src->width, src->height, (AVPixelFormat)src->format,
		dst->width, dst->height, (AVPixelFormat)dst->format,
		SWS_BILINEAR, NULL, NULL, NULL);
	
	return ctx != 0;
}

/**
* Маштабировать картику
*/
bool FFCScale::scale(AVFrame *dst, AVFrame *src)
{
	if ( ctx )
	{
		sws_scale(ctx, src->data, src->linesize, 0, src->height,
			dst->data, dst->linesize);
		return true;
	}
	
	return false;
}

/**
* Финализация маштабирования
*/
void FFCScale::close_scale()
{
	if ( ctx )
	{
		sws_freeContext(ctx);
		ctx = NULL;
	}
}

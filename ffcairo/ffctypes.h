#ifndef FFC_TYPES_H
#define FFC_TYPES_H

#include <ffcairo/config.h>

/**
 * Структура описывающая опции видео-потока
 */
struct FFCVideoOptions
{
	/**
	 * Ширина картинки
	 */
	int width;
	
	/**
	 * Высота картинки
	 */
	int height;
	
	/**
	 * Формат пикселей
	 */
	AVPixelFormat pix_fmt;
	
	/**
	 * Битрейт потока
	 */
	int64_t bit_rate;
	
	/**
	 * Фундаментальная единица времени (в секундах) в терминах которой
	 * представлены метки времени кадров.
	 */
	AVRational time_base;
	
	/**
	 * Число кадров в группе, или 0 для только начального кадра
	 *
	 * Как я понял, задает частоту ключевых кадров (c) Золотов А.В.
	 */
	int gop_size;
	
	/**
	 * ID кодека
	 */
	AVCodecID codec_id;
};

/**
 * Структура описывающая опции аудио-потока
 */
struct FFCAudioOptions
{
};


#endif // FFC_TYPES_H
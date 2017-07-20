#ifndef FFC_IMAGE_H
#define FFC_IMAGE_H

#include <stdint.h>
#include <cstddef>

#include <ffcairo/ffctypes.h>
#include <nanosoft/object.h>

/**
 * Класс представляющий картинку/холст
 *
 * Картинка хранится в формате RGBA который понимает и FFMPEG и Cairo,
 * для FFMPEG это PIX_FMT_BGRA
 * для Cairo это CAIRO_FORMAT_ARGB32
 * Оба контекста (surface от Cairo и AVFrame от FFMPEG) ссылаются
 * на один общий буфер с данными
 *
 * @note Для упрощения класс не скрывает внутренние структуры и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCImage: public Object
{
public:
	/**
	 * Ширина картинки/холста в пикселях
	 */
	int width;
	
	/**
	 * Высота картинки/холста в пикселях
	 */
	int height;
	
	/**
	 * Размер строки в байтах
	 */
	int stride;
	
	/**
	 * Буфер с данными (несжатый)
	 */
	uint8_t *data;
	
	/**
	 * Размер буфера в байтах
	 */
	int size;
	
	/**
	 * Поверхность cairo
	 */
	cairo_surface_t *surface;
	
	/**
	 * Фрейм ffmpeg
	 */
	AVFrame *avFrame;
	
protected:
	/**
	 * Конструктор по умолчанию
	 *
	 * Конструктор скрыт, для создания объекта используте статический метод
	 * createImage()
	 */
	FFCImage();
	
	/**
	 * Конструктор копий
	 *
	 * Реализации конструктора копий нет и само объявление скрыто, чтобы
	 * предотвратить возможные проблемы с копированием объекта
	 */
	FFCImage(const FFCImage &img);
public:
	
	/**
	 * Конструктор
	 */
	static FFCImage* createImage(int w, int h);
	
	/**
	 * Деструктор
	 *
	 * Автоматически высвобождает буфер
	 */
	~FFCImage();
	
	/**
	 * Сохранить в файл в формате PNG
	 */
	bool savePNG(const char *fname) const;
};

#endif // FFC_IMAGE_H

#ifndef FFC_MISC_H
#define FFC_MISC_H

/*****************************************************************************

  Вспомогательные и экспериментальные классы и функции, всё что пока
  не удалось классифицировать по отдельным файлам

 ****************************************************************************/

#include <ffcairo/ffctypes.h>
#include <ffcairo/ffcmuxer.h>
#include <ffcairo/ffcdemuxer.h>

/**
 * Класс копирующий поток как есть без декодирования из одного файла в другой
 */
class FFCStreamCopy: public FFCInputStream
{
public:
	/**
	 * Выходной поток
	 */
	//ptr<FFCOutputStream> oStream;
	
	/**
	 * Мультиплексор
	 */
	ptr<FFCMuxer> muxer;
	
	/**
	 * Конструктор
	 */
	FFCStreamCopy();
	
	/**
	 * Деструктор
	 */
	~FFCStreamCopy();
protected:
	/**
	 * Обработчик присоединения
	 *
	 * Автоматически вызывается когда поток присоединяется к демультиплексору
	 */
	virtual void handleAttach(AVStream *st);
	
	/**
	 * Обработчик отсоединения
	 *
	 * Автоматически вызывается когда поток отсоединяется от демультиплексора
	 */
	virtual void handleDetach();
	
	/**
	 * Обработчик пакета
	 */
	virtual void handlePacket(AVPacket *packet);
public:
	/**
	 * Создать выходной поток в мультиплексоре и настроить его для копирования
	 */
	AVStream* createStreamCopy();
};

#endif // FFC_MISC_H
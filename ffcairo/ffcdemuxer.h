#ifndef FFC_DEMUXER_H
#define FFC_DEMUXER_H

#include <ffcairo/ffctypes.h>
#include <ffcairo/avc_decoder.h>
#include <nanosoft/object.h>

/**
 * Абстрактный класс входного потока
 *
 * Это может быть и видео и аудио и субтитры и что угодно
 *
 * NOTE Устаревший класс, используйте AVCDecoder
 */
class FFCInputStream: public Object
{
friend class FFCDemuxer;
protected:
	
	/**
	 * Обработчик присоединения
	 *
	 * Автоматически вызывается когда поток присоединяется к демультиплексору
	 */
	virtual void handleAttach(AVStream *st) = 0;
	
	/**
	 * Обработчик отсоединения
	 *
	 * Автоматически вызывается когда поток отсоединяется от демультиплексора
	 */
	virtual void handleDetach() = 0;
	
	/**
	 * Обработчик пакета
	 */
	virtual void handlePacket(AVPacket *packet) = 0;
};

/**
 * Класс входящего видео потока
 *
 * Данный класс позволяет декодировать поток и получать отдельные кадры
 *
 * NOTE Устаревший класс, используйте AVCDecoder
 */
class FFCVideoInput: public FFCInputStream, public AVCVideoDecoder
{
public:
	/**
	 * Ссылка на поток
	 */
	AVStream *avStream;
	
	/**
	 * Конструктор
	 */
	FFCVideoInput();
	
	/**
	 * Деструктор
	 */
	~FFCVideoInput();
	
	/**
	 * Открыть кодек
	 */
	bool openDecoder();
	
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
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame() = 0;
};

/**
 * Демультиплексор
 *
 * Позволяет читать и декодировать медиа
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCDemuxer: public Object
{
private:
	/**
	 * Число потоков
	 */
	int stream_count;
	
	/**
	 * Потоки
	 */
	ptr<FFCInputStream> *streams;
public:
	/**
	 * Контест avFormat
	 */
	AVFormatContext *avFormat;
	
	/**
	 * Конструктор
	 */
	FFCDemuxer();
	
	/**
	 * Деструктор
	 */
	~FFCDemuxer();
	
public:
	
	/**
	 * Создать контекст
	 */
	bool createContext();
	
	/**
	 * Открыть поток
	 *
	 * Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
	 */
	bool open(const char *uri);
	
	/**
	 * Открыть поток через AVIO
	 *
	 * NOTE асинхронная (неблокирующая) работа при открытии не поддерживается
	 * самим FFMPEG, так что прежде чем вызывать openAVIO() нужно быть
	 * уверенным что уже поступило достаточно данных чтобы прочитать заголовки
	 * файла и расчитать частоту кадров. Более того, должны поступить не только
	 * заголовки, но и несколько первых фреймов, т.к. FFMPEG еще читает
	 * какое-то количество фреймов чтобы расчитать FPS. Будем надеятся в
	 * следующих версиях они сделают полноценную поддержку неблокирующего
	 * режима, по крайней мере в avio.h заявлено что такая работа ведется,
	 * см. описание AVIO_FLAG_NONBLOCK:
	 *   Warning: non-blocking protocols is work-in-progress; this flag may be
	 *   silently ignored.
	 */
	int openAVIO();
	
	/**
	 * Попытаться прочитать заголовки потока
	 * 
	 * используется только совместно с openAVIO() т.к. в асинхронном режиме
	 * возможно что данные заголовков еще не поступили и надо ждать
	 */
	int findStreamInfo();
	
	/**
	 * Найти видео-поток
	 *
	 * Возвращает ID потока или -1 если не найден
	 */
	int findVideoStream();
	
	/**
	 * Присоединить приемник потока
	 */
	void bindStream(int i, ptr<FFCInputStream> st);
	
	/**
	 * Обработать фрейм
	 */
	bool readFrame();
};

#endif // FFC_DEMUXER_H

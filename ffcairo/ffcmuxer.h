#ifndef FFC_MUXER_H
#define FFC_MUXER_H

#include <nanosoft/object.h>
#include <ffcairo/ffctypes.h>
#include <ffcairo/avc_encoder.h>

/**
 * Класс исходящего видео потока
 *
 * NOTE Устаревший класс, используйте AVCEncoder
 */
class FFCVideoOutput: public Object, public AVCVideoEncoder
{
public:
	/**
	 * Ссылка на поток
	 */
	AVStream *avStream;
	
	/**
	 * Конструктор
	 */
	FFCVideoOutput(AVStream *st);
	
	/**
	 * Деструктор
	 */
	~FFCVideoOutput();
	
	/**
	 * Открыть кодек
	 */
	bool openEncoder(const FFCVideoOptions *opts);
	
	/**
	 * Получить пакет
	 */
	bool recv_packet(AVPacket *pkt, int &got_packet);
};

/**
 * Класс представляющий исходящий видеопоток - поток который записывается
 * в файл или передается по сети на другой хост
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCMuxer: public Object
{
public:
	/**
	 * Контест avFormat
	 */
	AVFormatContext *avFormat;
	
	/**
	 * Конструктор
	 */
	FFCMuxer();
	
	/**
	 * Деструктор
	 */
	~FFCMuxer();
	
public:
	/**
	 * Создать файл
	 *
	 * На самом деле эта функция не создает файла, а только подготавливает
	 * контекст, который надо будет еще донастроить, в частности создать
	 * потоки и настроить кодеки, после чего вызвать функцию openFile()
	 * которая начнет запись файла.
	 */
	bool createFile(const char *fname);
	
	/**
	 * Создать контекст
	 */
	bool createContext(const char *fmt);
	
	/**
	 * Вернуть кодек по умолчанию для аудио
	 */
	AVCodecID defaultAudioCodec();
	
	/**
	 * Вернуть кодек по умолчанию для видео
	 */
	AVCodecID defaultVideoCodec();
	
	/**
	 * Вернуть кодек по умолчанию для субтитров
	 */
 	AVCodecID defaultSubtitleCodec();
	
	/**
	 * Создать поток
	 */
	AVStream* createStream();
	
	/**
	 * Создать поток
	 */
	AVStream* createStream(AVCodecID codec_id);
	
	/**
	 * Создать копию потока
	 */
	AVStream* createStreamCopy(AVStream *in_stream);
	
	/**
	 * Создать видео-поток
	 */
	FFCVideoOutput* createVideoStream(const FFCVideoOptions *opts);
	
	/**
	 * Открыть файл
	 *
	 * Открывает файл и записывает заголовки, перед вызовом должен
	 * быть создан контекст, потоки, настроены кодеки.
	 */
	bool openFile(const char *fname);
	
	/**
	 * Открыть файл
	 *
	 * Открывает файл (через AVIO) и записывает заголовки, перед вызовом должен
	 * быть создан контекст, потоки, настроены кодеки.
	 */
	bool openAVIO();
	
	/**
	 * Записать пакет
	 */
	bool writeFrame(AVPacket *pkt);
	
	/**
	 * Закрыть файл
	 *
	 * Записывает финальные данные и закрывает файл.
	 */
	bool closeFile();
};

#endif // FFC_MUXER_H

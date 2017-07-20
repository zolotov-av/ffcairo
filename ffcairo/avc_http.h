#ifndef AVC_HTTP_H
#define AVC_HTTP_H

#include <nanosoft/asyncstream.h>
#include <nanosoft/easyrow.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>
#include <ffcairo/ffcmuxer.h>

/**
 * Протокол HTTP для AVCEngine
 *
 * Пока это временный класс для тестирования движка AVCEngine и FFMPEG.
 * Для начала я планирую реализовать здесь серверную часть протокола HTTP,
 * минимально необходимую чтобы vlc мог к нему подключиться и показывать видео
 */
class AVCHttp: public AsyncStream
{
public:
	
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	/**
	 * Буфер для чтения заголовков
	 */
	std::string line;
	
	/**
	 * Начальная строка (неразобранная)
	 */
	std::string method;
	
	/**
	 * Заголовки HTTP
	 */
	EasyRow headers;
	
	enum {
		/**
		 * Чтение стартовой строки
		 */
		READ_METHOD,
		
		/**
		 * Чтение заголовков
		 */
		READ_HEADERS,
		
		/**
		 * Чтение тела
		 */
		READ_BODY,
		
		/**
		 * Потоковое вещание
		 */
		STREAMING,
		
		/**
		 * Состояние ошибки
		 */
		FAILED_STATE
	} http_state;
	
	/**
	 * Флаг завершения работы (по инициативе сервера)
	 */
	bool done;
	
	/**
	 * Флаг завершения работы (клиент закрыл сокет)
	 */
	bool peer_down;
	
	/**
	 * Мультиплексор
	 */
	ptr<FFCMuxer> muxer;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Видео-поток
	 */
	AVStream *vo;
	
	/**
	 * Флаг - пропустить до ключевого фрейма
	 */
	bool skip_to_keyframe;
	
	/**
	 * time_base входного потока
	 */
	AVRational in_time_base;
	
	/**
	 * Метка времени первого пакета в единицах входного потока (in_time_base)
	 */
	int64_t first_pts;
	
	/**
	 * Конструктор
	 */
	AVCHttp(int afd, AVCEngine *e);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCHttp();
	
protected:
	
	/**
	 * Обработчик прочитанных данных
	 */
	virtual void onRead(const char *data, size_t len);
	
	/**
	 * Обработчик события опустошения выходного буфера
	 *
	 * Вызывается после того как все данные были записаны в файл/сокет
	 */
	virtual void onEmpty();
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
	/**
	 * Обработчик записи пакета в поток
	 */
	static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
public:
	
	void write(const std::string &s);
	
	/**
	 * Инициализация стриминга
	 */
	bool initStreaming();
	
	/**
	 * Отправить пакет в стрим
	 */
	int sendPacket(AVPacket *pkt);
	
};

#endif // AVC_HTTP_H

#ifndef AVC_CHANNEL_H
#define AVC_CHANNEL_H

#include <ffcairo/avc_stream.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/ffcdemuxer.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>
#include <ffcairo/avc_packet.h>
#include <nanosoft/bufferstream.h>

class AVCChannel;

/**
 * Поток видео
 */
class AVCFeedInput: public FFCVideoInput
{
public:
	
	/**
	 * Ссылка на агента
	 */
	AVCChannel *channel;
	
	/**
	 * Конструктор
	 */
	AVCFeedInput(AVCChannel *ch);
	
	/**
	 * Деструктор
	 */
	~AVCFeedInput();
	
protected:
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame();
	
};

/**
 * Канал (сетевой) сервера видеоконференций
 *
 * Данный класс описывают серверную часть канала
 */
class AVCChannel: public AVCStream
{
friend class AVCFeedInput;
public:
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	enum {
		/**
		 * Ожидание фида
		 */
		FEED_WAIT,
		
		/**
		 * Открытие фида
		 */
		FEED_OPEN_INPUT,
		
		/**
		 * Чтение тела
		 */
		FEED_STREAMING,
		
		/**
		 * Состояние ошибки
		 */
		FEED_FAILED
	} feed_state;
	
	/**
	 * Буфер
	 */
	BufferStream pktbuf;
	
	/**
	 * Число пакетов в буфере
	 */
	int pkt_count;
	
	int queue_size;
	
	int init_size;
	
	/**
	 * Демуксер
	 */
	ptr<FFCDemuxer> demux;
	
	/**
	 * Видео-поток
	 */
	ptr<AVCFeedInput> vin;
	
	/**
	 * Картинка
	 */
	ptr<FFCImage> pic;
	
	/**
	 * Контекст маштабирования
	 */
	ptr<FFCScale> scale;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Конструктор
	 */
	AVCChannel(int afd, AVCEngine *e);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCChannel();
	
protected:
	
	/**
	 * Обработчик чтения пакета из потока
	 */
	int real_read_packet(uint8_t *buf, int buf_size);
	
	/**
	 * Обработчик чтения пакета из потока
	 */
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
	/**
	 * Обработчик пакета
	 */
	virtual void onPacket(const AVCPacket *pkt);
	
	/**
	 * буферизовать пакет
	 */
	void queuePacket(const AVCPacket *pkt);
	
	/**
	 * прочитать данные из очереди
	 */
	int queueRead(uint8_t *buf, int buf_size);
	
	/**
	 * Обработка данных в фида
	 */
	void handleFeedData();
	
	/**
	 * Обработчик фрейма
	 */
	void handleFrame(AVFrame *avFrame);
	
public:
	
	/**
	 * Обработчик kick'а
	 *
	 * Временная, а может и постоянная функция, вызывается когда фидер был
	 * замещен новым потоком. Здесь мы завершаем свой поток и дисконнектим
	 * клиента
	 */
	void handleKick();
	
	/**
	 * Обработчик нового фидера
	 *
	 * Вызывается когда от клиента получает соответвующую команду, здесь
	 * нужно инициализировать демуксер и подключиться к сцене
	 */
	void handleNewFeed();
	
	/**
	 * Открыть входящий поток
	 */
	bool openFeed();
	
	/**
	 * Открыть видео поток
	 */
	bool openVideoStream();
	
};

#endif // AVC_CHANNEL_H

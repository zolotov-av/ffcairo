#ifndef AVC_FEEDAGENT_H
#define AVC_FEEDAGENT_H

#include <ffcairo/avc_agent.h>
#include <ffcairo/ffcmuxer.h>
#include <ffcairo/ffcdemuxer.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>

class AVCFeedAgent;

/**
 * Поток видео
 */
class AVCFeedStream: public FFCVideoInput
{
public:
	
	/**
	 * Ссылка на агента
	 */
	AVCFeedAgent *agent;
	
	/**
	 * Конструктор
	 */
	AVCFeedStream(AVCFeedAgent *a);
	
	/**
	 * Деструктор
	 */
	~AVCFeedStream();
	
protected:
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame();
	
};

/**
 * Агент фидера
 *
 * Данный класс будет иметь тривиальную реализацию и содержать тестовый код.
 * На первое время он будет отправлять поток серверу как есть.
 */
class AVCFeedAgent: public AVCAgent
{
friend class AVCFeedStream;
public:
	
	/**
	 * флаг стриминга
	 */
	int streaming;
	
	/**
	 * Опции видео-кодека
	 */
	FFCVideoOptions opts;
	
	/**
	 * Номер фрейма
	 */
	int iFrame;
	
	/**
	 * Демуксер
	 */
	ptr<FFCDemuxer> demux;
	
	/**
	 * Мультиплексор
	 */
	ptr<FFCMuxer> muxer;
	
	/**
	 * Хост на котором будет рисоваться видео
	 */
	ptr<FFCImage> pic;
	
	/**
	 * Контест маштабирования
	 */
	ptr<FFCScale> scale;
	
	/**
	 * Контест маштабирования
	 */
	ptr<FFCScale> scale_pic;
	
	/**
	 * Входящий видео поток
	 */
	ptr<AVCFeedStream> vin;
	
	/**
	 * Исходящий видео поток
	 */
	ptr<FFCVideoOutput> vout;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Конструктор
	 */
	AVCFeedAgent(NetDaemon *d);
	
	/**
	 * Деструктор
	 */
	~AVCFeedAgent();
	
protected:
	
	/**
	 * Обработчик станзы feed
	 */
	void handleFeedStanza(EasyTag &stanza);
	
	/**
	 * Обработчик станзы
	 */
	virtual void onStanza(EasyTag stanza);
	
	/**
	 * Обработчик записи пакета в поток
	 */
	static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
	/**
	 * Обработчик установления соединения
	 * 
	 * Этот обработчик вызывается после успешного вызова connect().
	 * В ассинхронном режиме это обычно происходит до полноценного
	 * установления соединения, но движок уже готов буферизовать данные
	 * и как только соединение будет завершено, данные будут отправлены.
	 * Если соединение установить не удасться, то будет вызван обработчик
	 * onError()
	 */
	virtual void onConnect();
	
	/**
	 * Обработчик фрейма
	 */
	void handleFrame(AVFrame *avFrame);
	
	/**
	 * Отправить фрейм в выходной поток
	 */
	int sendFrame();
	
	/**
	 * Отправить данные в поток
	 */
	bool sendData(const char *buf, int size);
	
	/**
	 * Завершить стриминг
	 *
	 * Временная функция для прерывания стрима в случае непредвиденных ошибок
	 */
	void failStreaming();
	
public:
	/**
	 * Открыть веб-камеру
	 */
	int openWebcam(const char *fname);
	
	/**
	 * Открыть видео поток
	 */
	bool openVideoStream();
	
	/**
	 * Открыть исходящий стрим
	 */
	int openFeed(int width, int height, int64_t bit_rate);
	
	/**
	 * Таймер
	 */
	void onTimer(const timeval &tv);
};

#endif // AVC_FEEDAGENT_H
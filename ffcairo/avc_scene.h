#ifndef AVC_SCENE_H
#define AVC_SCENE_H

#include <nanosoft/object.h>

#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>
#include <ffcairo/ffcmuxer.h>

class AVCHttp;
class AVCChannel;

#define MAX_HTTP_CLIENTS 8

/**
 * Класс описывающий сцену
 *
 * Это "канва" на которую будет "рисоваться" контент, этот же класс отвечает
 * за компрессию данных
 */
class AVCScene: public FFCMuxer
{
private:
	
	/**
	 * Клиенты HTTP
	 */
	ptr<AVCHttp> http_clients[MAX_HTTP_CLIENTS];
	
	/**
	 * Число клиентов HTTP
	 */
	int http_count;
	
public:
	
	/**
	 * Опции видео-кодека
	 */
	FFCVideoOptions opts;
	
	/**
	 * Номер фрейма
	 */
	int iFrame;
	
	/**
	 * Время начала стрима в мили-timestamp
	 */
	int64_t start_pts;
	
	/**
	 * Запланированное время следующего кадра
	 */
	int64_t next_pts;
	
	/**
	 * Время предыдущего кадра в мили-timestamp
	 */
	int64_t curr_pts;
	
	/**
	 * Хост на котором будет рисоваться видео
	 */
	ptr<FFCImage> pic;
	
	/**
	 * Контест маштабирования
	 */
	ptr<FFCScale> scale;
	
	/**
	 * Видео-поток
	 */
	ptr<FFCVideoOutput> vo;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Фидер
	 *
	 * Для начала пишем в лоб и поддерживаем один фидер, позже надо заменить
	 * на список
	 */
	AVCChannel *feed;
	
	/**
	 * Конструктор
	 */
	AVCScene();
	
	/**
	 * Деструктор
	 */
	~AVCScene();
	
protected:
	
	/**
	 * Обработчик записи пакета в поток
	 */
	static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
	/**
	 * Отправить фрейм в кодировщик
	 */
	int sendFrame();
	
	/**
	 * Отправить пакет в стрим
	 */
	int sendPacket(AVPacket *pkt);
	
public:
	
	/**
	 * Создать контекст с AVIO
	 */
	int createAVIO(const char *format);
	
	/**
	 * Инициализация стриминга
	 */
	int initStreaming(int width, int height, int64_t bit_rate);
	
	/**
	 * Завершить стриминг
	 *
	 * Временная функция для прерывания стрима в случае непредвиденных ошибок
	 */
	void failStreaming();
	
	/**
	 * Выпустить новый фрейм и отправить его в стриминг
	 */
	void emitFrame();
	
	/**
	 * Таймер
	 *
	 * Таймер должен запускаться не менее раз в 40мс
	 */
	void onTimer(const timeval *tv);
	
	/**
	 * Клиента HTTP
	 */
	bool addHttpClient(AVCHttp *client);
	
	/**
	 * Удалить клиента HTTP
	 */
	void removeHttpClient(AVCHttp *client);
	
	/**
	 * Заменить фидера
	 *
	 * Устанавливает фидера, если фидер уже был установлен,то заменяет его,
	 * а старого кикает с сервера
	 *
	 * Временная функция
	 */
	void replaceFeed(AVCChannel *ch);
	
	/**
	 * Удалить фидера
	 *
	 * Если текущий фидер равен указанному, то удалить фидер, если не
	 * совпадает, то ничего не делать
	 *
	 * Временная функция
	 */
	void removeFeed(AVCChannel *ch);
	
};

#endif // AVC_SCENE_H
#ifndef AVC_STREAM_H
#define AVC_STREAM_H

#include <nanosoft/asyncstream.h>

#define AVC_SIMPLE 1
#define AVC_PAYLOAD 2

/**
 * Базовая структура всех передаваемых пакетов
 */
struct avc_packet_t
{
	/**
	 * Размер пакета включая заголовки, т.е. размер пакета не может быть
	 * меньше 4 байт
	 */
	uint8_t len[2];
	
	/**
	 * Тип пакета
	 */
	uint8_t type;
	
	/**
	 * Канал
	 */
	uint8_t channel;
};

/**
 * Структура описывающая сигнальные сообщения
 */
struct avc_message_t: public avc_packet_t
{
	
};

/**
 * Структура описывающая потоковые данные
 */
struct avc_payload_t: public avc_packet_t
{
	char buf[1020];
};

/**
 * Возвращает длину пакета
 */
inline int avc_packet_len(const avc_packet_t *pkt)
{
	return pkt->len[0] + pkt->len[1] * 256;
}

/**
 * Возвращает длину содержимого пакета
 */
inline int avc_packet_payload(const avc_packet_t *pkt)
{
	return avc_packet_len(pkt) - 4;
}

/**
 * Установить размер пакета
 */
bool avc_set_packet_len(avc_packet_t *pkt, int len);

/**
 * Вывести пакет в отладочный вывод
 */
void avc_dump_packet(const char *act, const avc_packet_t *pkt);

/**
 * Класс AVCStream
 *
 * В этом классе будет реализован протокол общения клиента и сервера
 * видеоконференций. Я планирую сделать универсальный протокол, который можно
 * будет использовать и в других проектах. Мои идеи о протоколе основываются
 * на моих старых реализациях протоколов XMPP и MayCloud, но в отличие от них
 * данный протокол будет бинарным.
 */
class AVCStream: public AsyncStream
{
protected:
	
	/**
	 * Буфер для временных данных
	 */
	char buf[4096];
	
	/**
	 * Размер буферизованных данных
	 */
	int buf_len;
	
public:
	
	/**
	 * Конструктор
	 */
	AVCStream(int afd);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCStream();
	
protected:
	
	/**
	 * Обработчик прочитанных данных
	 */
	virtual void onRead(const char *data, size_t len);
	
	/**
	 * Обработчик пакета
	 */
	virtual void onPacket(const avc_packet_t *pkt) = 0;
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
public:
	
	/**
	 * Отправить пакет
	 */
	bool sendPacket(const avc_packet_t *pkt);
};


#endif // AVC_STREAM_H

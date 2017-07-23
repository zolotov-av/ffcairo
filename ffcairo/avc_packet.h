#ifndef AVC_PACKET_H
#define AVC_PACKET_H

#include <stdint.h>
#include <stdlib.h>

#define AVC_SIMPLE 1
#define AVC_PAYLOAD 2
#define AVC_STANZA 3

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
 * Пакет данных
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class AVCPacket
{
public:
	
	/**
	 * Заголовок пакета
	 */
	avc_packet_t header;
	
	/**
	 * Буфер с данными
	 */
	uint8_t *data;
	
	/**
	 * Размер с данными (без учета заголовка)
	 */
	size_t data_size;
	
	/**
	 * Конструктор
	 */
	AVCPacket();
	
	/**
	 * Деструктор
	 */
	~AVCPacket();
	
	/**
	 * Вернуть тип пакета
	 */
	int type() const { return header.type; }
	
	/**
	 * Вернуть канал пакета
	 */
	int channel() const { return header.channel; }
	
	/**
	 * Вернуть длину содежимого (без учета заголовка)
	 */
	int size() const { return avc_packet_payload(&header); }
	
	/**
	 * Сбросить пакет
	 *
	 * Если буфер был выделен, то он освобождается
	 */
	void reset();
	
	/**
	 * Пересоздать пакет
	 *
	 * Выделяет буфер указанного размера. Если буфер был ранее выделен, то
	 * данные будут утеряны. Если новый размер равен нулю, то буфер будет
	 * просто удален
	 */
	bool reset(size_t new_size);
	
	/**
	 * Изменить размер пакета
	 *
	 * Буфер перераспределяется с помощью функции realloc(), данные при этом
	 * сохрнаяются
	 */
	bool resize(size_t new_size);
	
};

#endif // AVC_PACKET_H


#include <ffcairo/avc_stream.h>
#include <nanosoft/debug.h>

/**
* Установить размер пакета
*/
bool avc_set_packet_len(avc_packet_t *pkt, int len)
{
	if ( len < 4 || len > 0xFFFF )
	{
		pkt->len[0] = 0;
		pkt->len[1] = 0;
		return false;
	}
	
	pkt->len[0] = len & 0xFF;
	pkt->len[1] = (len >> 8) & 0xFF;
}

/**
 * Вывести пакет в отладочный вывод
 */
void avc_dump_packet(const char *act, const avc_packet_t *pkt)
{
	char buf[80];
	char *p;
	int plen = avc_packet_len(pkt);
	const uint8_t *data = (const uint8_t*)pkt;
	printf("%s PACKET type=0x%02X channel=0x%02X len=0x%04X (%d)\n", act, pkt->type, pkt->channel, plen, plen);
	
	while ( plen > 0 )
	{
		int len = plen;
		if ( len > 16 ) len = 16;
		p = buf;
		for(int i = 0; i < len; i++)
		{
			p += sprintf(p, "%02X ", *data++);
			plen--;
		}
		printf("%s\n", buf);
		
		break;
	}
}

/**
* Конструктор
*/
AVCStream::AVCStream(int afd): AsyncStream(afd), buf_len(0)
{
}

/**
* Деструктор
*/
AVCStream::~AVCStream()
{
}

/**
* Обработчик прочитанных данных
*/
void AVCStream::onRead(const char *data, size_t len)
{
	do
	{
		// наполняем буфер хотя бы до 2х байт чтобы у нас была информация о
		// длине пакета
		while ( buf_len < 2 && len > 0 )
		{
			buf[buf_len] = *data;
			buf_len++;
			data++;
			len--;
		}
		
		// если не смогли наполнить буфер, значит входные данные кончились
		if ( buf_len < 2 ) return;
		
		const avc_packet_t *p = (const avc_packet_t*)buf;
		int plen = avc_packet_len(p);
		
		// наполняем буфер до размера пакета
		size_t xlen = plen - buf_len; // размер который надо дополнить
		if ( xlen > len ) xlen = len; // копируем сколько есть
		memcpy(buf + buf_len, data, xlen);
		buf_len += xlen;
		data += xlen;
		len -= xlen;
		
		// если не смогли наполнить буфер, значит входные данные кончились
		if ( buf_len < plen ) return;
		
		if ( DEBUG::DUMP_STANZA ) avc_dump_packet("READ", p);
		onPacket(p);
		buf_len = 0;
	}
	while ( len > 0 );
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCStream::onPeerDown()
{
}

/**
* Отправить пакет
*/
bool AVCStream::sendPacket(const avc_packet_t *pkt)
{
	if ( DEBUG::DUMP_STANZA ) avc_dump_packet("SEND", pkt);
	return put( (const char*)pkt, avc_packet_len(pkt) );
}

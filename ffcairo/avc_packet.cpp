
#include <ffcairo/avc_packet.h>

#include <stdlib.h>
#include <stdio.h>

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
AVCPacket::AVCPacket(): data(NULL), data_size(0)
{
}

/**
* Деструктор
*/
AVCPacket::~AVCPacket()
{
	if ( data ) free(data);
}

/**
* Сбросить пакет
*
* Если буфер был выделен, то он освобождается
*/
void AVCPacket::reset()
{
	if ( data )
	{
		free(data);
		data = NULL;
		data_size = 0;
	}
}

/**
* Пересоздать пакет
*
* Выделяет буфер указанного размера. Если буфер был ранее выделен, то
* данные будут утеряны. Если новый размер равен нулю, то буфер будет
* просто удален
*/
bool AVCPacket::reset(size_t new_size)
{
	if ( data ) free(data);
	
	if ( new_size == 0 )
	{
		data = NULL;
		data_size = 0;
		return true;
	}
	
	data = (uint8_t*) malloc(new_size);
	if ( data )
	{
		data_size = new_size;
		return true;
	}
	else
	{
		printf("malloc(%lu) failed\n", new_size);
		data = NULL;
		data_size = 0;
	}
	return false;
}

/**
* Изменить размер пакета
*
* Буфер перераспределяется с помощью функции realloc(), данные при этом
* сохрнаяются
*/
bool AVCPacket::resize(size_t new_size)
{
	printf("AVCPacket::resize(%lu)\n", new_size);
	if ( new_size == 0 )
	{
		reset();
		return true;
	}
	
	void *ptr = realloc(data, new_size);
	if ( ptr )
	{
		data = (uint8_t*)ptr;
		data_size = new_size;
		return true;
	}
	
	return false;
}


#include <ffcairo/avc_stream.h>
#include <nanosoft/debug.h>

/**
* Конструктор
*/
AVCStream::AVCStream(int afd): AsyncStream(afd), read_state(WAIT_HEADER)
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
	size_t ret = bs.write(data, len);
	if ( ret < len )
	{
		// TODO
		printf("AVCStream::onRead() read buffer full\n");
		close();
		exit(-1);
	}
	
	while ( 1 )
	{
		if ( read_state == WAIT_HEADER )
		{
			// если данных недостаточно чтобы прочитать заголовок, то выходим
			if ( bs.getDataSize() < sizeof(packet.header) ) return;
			
			// читаем заголовок пакета
			bs.read(&packet.header, sizeof(packet.header));
			
			// в пакете есть нагрузка?
			size_t plen = avc_packet_payload(&packet.header);
			if ( plen > 0 )
			{
				// если в пакете есть нагрузка, то подготовить буфер и перейти
				// к чтению данных
				if ( ! packet.reset(plen) )
				{
					// TODO
					printf("packet.reset() failed\n");
					close();
					exit(-1);
				}
				read_size = 0;
				read_state = READ_DATA;
			}
			else
			{
				// если пакет пустой, то сразу его обработать
				onPacket(&packet);
				continue;
			}
		}
		
		if ( read_state == READ_DATA )
		{
			size_t plen = avc_packet_payload(&packet.header);
			size_t rest = plen - read_size;
			size_t ret = bs.read(packet.data + read_size, rest);
			read_size += ret;
			if ( read_size == plen )
			{
				onPacket(&packet);
				read_state = WAIT_HEADER;
				continue;
			}
			
			if ( bs.getDataSize() == 0 ) return;
		}
	}
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

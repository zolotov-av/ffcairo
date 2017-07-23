
#include <ffcairo/avc_stream.h>
#include <nanosoft/debug.h>
#include <nanosoft/tagparser.h>

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
* Парсер станзы
*/
static EasyTag parseStanza(const AVCPacket *pkt)
{
	TagParser parser;
	return parser.parse(pkt->data, pkt->size());
}

/**
* Обработка станзы
*/
void AVCStream::handleStanza(const AVCPacket *pkt)
{
	EasyTag stanza = parseStanza(pkt);
	if ( stanza.isTag() )
	{
		if ( DEBUG::DUMP_STANZA )
		{
			fprintf(stdout, "RECV STANZA[%d]: \033[22;32m%s\033[0m\n", getFd(), stanza.serialize().c_str());
		}
		onStanza(stanza);
	}
	else
	{
		printf("stanza parse error: %s", stanza.cdata().c_str());
	}
}

/**
* Обработка пакета
*/
void AVCStream::handlePacket(const AVCPacket *pkt)
{
	if ( pkt->channel() == 0 )
	{
		if ( pkt->type() == AVC_STANZA )
		{
			handleStanza(pkt);
			return;
		}
	}
	
	// пока для совместимости, пока непонятно как все раскидать по функциям
	onPacket(pkt);
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
				handlePacket(&packet);
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
				handlePacket(&packet);
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
	//if ( DEBUG::DUMP_STANZA ) avc_dump_packet("SEND", pkt);
	return put( (const char*)pkt, avc_packet_len(pkt) );
}

/**
* Отправить пакет
*/
bool AVCStream::sendPacket(const AVCPacket *pkt)
{
	// TODO реализовать транзакции в NetDaemon
	bool ret = put((const char*) &pkt->header, sizeof(pkt->header));
	if ( ! ret ) return false;
	ret = put( (const char*)&pkt, pkt->size() );
	if ( ! ret )
	{
		printf("AVCStream::put(header) failed\n");
		close();
		exit(-1);
	}
	
	return true;
}

/**
* Отправить станзу в поток
*/
bool AVCStream::sendStanza(EasyTag stanza)
{
	std::string text = stanza.serialize();
	
	if ( DEBUG::DUMP_STANZA )
	{
		fprintf(stdout, "SEND STANZA[%d]: \033[22;34m%s\033[0m\n", getFd(), text.c_str());
	}
	
	avc_packet_t pkt;
	pkt.channel = 0;
	pkt.type = AVC_STANZA;
	avc_set_packet_len(&pkt, text.length() + 4);
	
	// TODO реализовать транзакции в NetDaemon
	bool ret = put((const char*) &pkt, sizeof(pkt));
	if ( ! ret ) return false;
	ret = put(text.c_str(), text.length());
	if ( ! ret )
	{
		printf("AVCStream::put(header) failed\n");
		close();
		exit(-1);
	}
	
	return true;
}

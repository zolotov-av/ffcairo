#ifndef AVC_STREAM_H
#define AVC_STREAM_H

#include <ffcairo/avc_packet.h>
#include <nanosoft/asyncstream.h>
#include <nanosoft/bufferstream.h>

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
private:
	
	enum {
		
		/**
		 * Ожидание заголовка пакета
		 */
		WAIT_HEADER,
		
		/**
		 * Чтение данных
		 */
		READ_DATA
		
	} read_state;
	
	/**
	 * Размер прочитанной части пакета
	 */
	size_t read_size;
	
	/**
	 * Пакет
	 */
	AVCPacket packet;
	
	/**
	 * Буфер
	 */
	BufferStream bs;
	
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
	virtual void onPacket(const AVCPacket *pkt) = 0;
	
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

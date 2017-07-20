/****************************************************************************

Демо-программа №3

Демонстрирует возможности ремультиплексирования - копирования потоков из
из одного контейнера в другой без перекодирования самих потоков

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcmisc.h>
#include <math.h>


int main(int argc, char *argv[])
{
	const char *iname = argc > 1 ? argv[1] : "test.avi";
	const char *oname = argc > 2 ? argv[2] : "out.avi";
	printf("input filename: %s\n", iname);
	printf("output filename = %s\n", oname);
	
	// INIT
	av_register_all();
	
	ptr<FFCDemuxer> demux = new FFCDemuxer();
	
	// открыть исходный файл
	if ( ! demux->open(iname) )
	{
		printf("fail to open file[%s]\n", iname);
		return -1;
	}
	
	// создаем копирующий поток
	ptr<FFCStreamCopy> sc = new FFCStreamCopy;
	
	// создаем мультиплексор
	sc->muxer = new FFCMuxer();
	
	// открываем выходной файл
	if ( ! sc->muxer->createFile(oname) )
	{
		printf("failed to muxer.createFile(%s)\n", oname);
		return -1;
	}
	
	// ищем видео-поток
	int video_stream = demux->findVideoStream();
	if ( video_stream < 0 )
	{
		printf("video stream not found\n");
		return -1;
	}
	
	// присоединяем обработчик к видео-потоку
	demux->bindStream(video_stream, sc);
	
	// создаем поток в выходном файле
	AVStream *oStream = sc->createStreamCopy();
	if ( oStream == NULL )
	{
		printf("failed create output stream\n");
		return -1;
	}
	
	//sc->oStream = muxer->createVideoStream(&opts);
	/*
	if ( ! vo->openEncoder(&opts) )
	{
		return -1;
	}
	
	av_dump_format(muxer->avFormat, 0, fname, 1);
	
	vo->initScale(pic);
	
	if ( ! muxer->openFile(fname) )
	{
		printf("fail to openFile(%s)\n", fname);
		return -1;
	}
	
	int frameNo = 0;
	while ( 1 )
	{
		frameNo++;
		if ( frameNo > (25 * 30) ) break;
		
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);
		int got_packet = 0;
		
		
		if ( ! vo->encode(pic, &pkt, &got_packet) )
		{
			printf("frame[%d] encode failed\n", frameNo);
			break;
		}
		
		if ( got_packet )
		{
			vo->rescale_ts(&pkt);
			
			muxer->writeFrame(&pkt);
		}
	}
	
	muxer->closeFile();
	*/
	return 0;
}

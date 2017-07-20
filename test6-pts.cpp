/****************************************************************************

Тестовый исходник для экспериментов

Тест временных меток (time_base, pts, AVRational)

****************************************************************************/

extern "C" {

#include <libavutil/rational.h>
#include <libavutil/mathematics.h>

}

#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

uint64_t tv2ts(const timeval *tv, const AVRational *tb)
{
	AVRational xtb = {1, 1000};
	int64_t ts = tv->tv_sec * 1000 + (tv->tv_usec / 1000);
	
	return av_rescale_q_rnd(ts, xtb, *tb, AV_ROUND_ZERO);
}

void test_tv2ts(const timeval *tv, const AVRational *tb)
{
	uint64_t ts = tv2ts(tv, tb);
	int sec = tv->tv_sec;
	int msec = tv->tv_usec / 1000;
	printf("tv=%d.%03d, ts=%"PRId64"\n", sec, msec, ts);
}

void tv_sub(timeval *a, timeval *b)
{
	int sec = a->tv_sec - b->tv_sec;
	int usec = a->tv_usec - b->tv_usec;
	if ( usec < 0 )
	{
		usec += 1000000;
		sec--;
	}
	if ( usec < 0 )
	{
		printf("tv_sub failed\n");
		exit(-1);
	}
	a->tv_sec = sec;
	a->tv_usec = usec;
}

int main(int argc, char *argv[])
{
	timeval tv;
	timeval tv0;
	AVRational tb = { 1, 25 };
	
	int ret = gettimeofday(&tv0, NULL);
	if ( ret < 0 )
	{
		printf("gettimeofday() failed: %s", strerror(errno));
		return -1;
	}
	
	for(int i = 0; i < 61; i ++)
	{
		tv.tv_sec = 0;
		tv.tv_usec = i * 10 * 1000;
		test_tv2ts(&tv, &tb);
	}
	
	srand(time(NULL));
	
	printf("\n");
	while ( 1 )
	{
		int ret = gettimeofday(&tv, NULL);
		if ( ret < 0 )
		{
			printf("gettimeofday() failed: %s", strerror(errno));
			return -1;
		}
		
		tv_sub(&tv, &tv0);
		test_tv2ts(&tv, &tb);
		
		// выйти через N секунд
		if ( tv.tv_sec > 10 ) break;
		
		timespec req;
		req.tv_sec = 0;
		req.tv_nsec = (rand() & 0xFF + 300 - 256) * 1000 * 1000;
		nanosleep(&req, NULL);
	}
	
	
	return 0;
}

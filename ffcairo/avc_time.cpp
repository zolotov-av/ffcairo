
#include <ffcairo/avc_time.h>

#include <sys/time.h>

/**
 * Стандартный time_base (time_base для мили-timestamp)
 */
const AVRational ms_time_base = { 1, 1000 };

/**
 * Вернуть время в мили-timestamp
 *
 * Обертка вокруг gettimeofday(). Если gettimeofday() возвращает ошибку,
 * то возвращается AV_NOPTS_VALUE
 */
uint64_t ms_time()
{
	struct timeval tv;
	int ret = gettimeofday(&tv, NULL);
	if ( ret < 0 ) return AV_NOPTS_VALUE;
	return tv2ms(&tv);
}

/**
 * Вернуть текущее время в нужном time_base
 *
 * Обертка вокруг gettimeofday(). Если gettimeofday() возвращает ошибку,
 * то возвращается AV_NOPTS_VALUE
 */
uint64_t ts_time(const AVRational *dst_tb)
{
	static const AVRational us_time_base = { 1, 1000000 };
	
	struct timeval tv;
	int ret = gettimeofday(&tv, NULL);
	if ( ret < 0 ) return AV_NOPTS_VALUE;
	
	// сначала приводим к 64-битному значению
	uint64_t usec = tv.tv_sec;
	
	// чтобы умножение было гарантированно в 64-битной арифметике
	uint64_t ts = usec * 1000000 + tv.tv_usec;
	
	return av_rescale_q(ts, us_time_base, *dst_tb);
}

/**
 * Преобразовать время из структуры timeval, в мили-timestamp
 */
uint64_t tv2ms(const timeval *tv)
{
	// я распишу код максимально однозначно, а оптимизацией пусть
	// занимается компилятор
	
	// сначала приводим к 64-битному значению
	uint64_t msec = tv->tv_sec;
	
	// чтобы умножение было гарантированно в 64-битной арифметике,
	// а микросекунды округляем к ближайшему значению до милисекунд
	return msec * 1000 + (tv->tv_usec + 500) / 1000;
}

/**
 * Преобразовать время из структуры timeval, в нужный time_base
 */
uint64_t tv2ts(const timeval *tv, const AVRational *dst_tb)
{
	return av_rescale_q(tv2ms(tv), ms_time_base, *dst_tb);
}


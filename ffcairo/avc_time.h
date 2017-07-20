#ifndef AVC_TIME_H
#define AVC_TIME_H

#include <ffcairo/ffctypes.h>

/**
 * Стандартный time_base (time_base для мили-timestamp)
 */
extern const AVRational ms_time_base;

/**
 * Вернуть время в мили-timestamp
 *
 * Обертка вокруг gettimeofday(). Если gettimeofday() возвращает ошибку,
 * то возвращается AV_NOPTS_VALUE
 */
uint64_t ms_time();

/**
 * Вернуть текущее время в нужном time_base
 *
 * Обертка вокруг gettimeofday(). Если gettimeofday() возвращает ошибку,
 * то возвращается AV_NOPTS_VALUE
 */
uint64_t ts_time(const AVRational *dst_tb);

/**
 * Преобразовать время из структуры timeval, в мили-timestamp
 */
uint64_t tv2ms(const timeval *tv);

/**
 * Преобразовать время из структуры timeval, в нужный time_base
 */
uint64_t tv2ts(const timeval *tv, const AVRational *dst_tb);

#endif // AVC_TIME_H

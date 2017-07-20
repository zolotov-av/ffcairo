
#include <ffcairo/ffcimage.h>

extern "C" {
#include <libavutil/imgutils.h>
}

#include <stdio.h>

/**
* Конструктор по умолчанию
*
* Конструктор скрыт, для создания объекта используте статический метод
* createImage()
*/
FFCImage::FFCImage()
{
}

/**
* Конструктор
*/
FFCImage* FFCImage::createImage(int w, int h)
{
	// прежде чем создавать объект проверим размеры картинки на допустимость
	int av_size = av_image_get_buffer_size(AV_PIX_FMT_BGRA, w, h, 1);
	if ( av_size < 0 )
	{
		// размеры картинки недопустимые, возвращаем NULL
		return NULL;
	}
	
	FFCImage *pic = new FFCImage();
	if ( ! pic ) return NULL;
	
	pic->width = w;
	pic->height = h;
	pic->stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);
	pic->size = pic->stride * h;
	
	// проверяем размеры которые вернул FFMPEG и Cairo, один должны быть равны
	// если нет, то берем больший размер и молимся что будет работать
	// (скорее всего будут глюки, но по крайней мере не будет сегфолта)
	if ( pic->size != av_size )
	{
		printf("cairo size=%d, ffmpeg size=%d\n", pic->size, av_size);
	}
	if ( av_size > pic->size ) pic->size = av_size;
	
	// выделяем буфер под картинку
	pic->data = (uint8_t *)av_malloc( pic->size * sizeof(uint8_t) );
	if ( ! pic->data )
	{
		printf("alloc buffer failed\n");
		goto fail;
	}
	
	pic->surface = cairo_image_surface_create_for_data(pic->data, CAIRO_FORMAT_ARGB32, w, h, pic->stride);
	if ( cairo_surface_status(pic->surface) != CAIRO_STATUS_SUCCESS )
	{
		printf("cairo_image_surface_create_for_data failed\n");
		goto fail;
	}
	
	// Allocate an AVFrame structure
	pic->avFrame = av_frame_alloc();
	if( pic->avFrame == NULL )
	{
		printf("avFrame alloc failed\n");
		goto fail;
	}
	else
	{
		pic->avFrame->width = w;
		pic->avFrame->height = h;
		pic->avFrame->format = AV_PIX_FMT_BGRA;
		
		// magic...
		int ret = av_image_fill_arrays(pic->avFrame->data, pic->avFrame->linesize, pic->data, AV_PIX_FMT_BGRA, w, h, 1);
		if ( ret < 0 )
		{
			printf("av_image_fill_arrays() failed\n");
			goto fail;
		}
	}
	
	return pic;
	
fail:
	delete pic;
	return NULL;
}

/**
* Деструктор
*
* Автоматически высвобождает буфер
*/
FFCImage::~FFCImage()
{
	av_frame_free(&avFrame);
	av_free(data);
}

/**
* Сохранить в файл в формате PNG
*/
bool FFCImage::savePNG(const char *fname) const
{
	cairo_surface_write_to_png(surface, fname);
}


#include <stdafx.h>
#pragma once

#ifndef __IMAGE_LOADER_H__
#define __IMAGE_LOADER_H__

typedef enum RC_ImageType
{
	IM_PCX,
	IM_PPM,
	IM_BITMAP,

	IM_MAX
}RC_ImageType;

class ImageLoader
{
public:
	ImageLoader() {};
	~ImageLoader() {};

	static void* LoadFromFile(const char* a_filename, u32 a_type, u32& a_w,
		u32& a_h, u8& a_bbp, void*&);
};
#endif // !__IMAGE_LOADER_H__


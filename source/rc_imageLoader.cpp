#include <stdafx.h>
#include "rc_ImageLoader.h"
#include "rc_pcxLoader.h"

void* ImageLoader::LoadFromFile(const char* a_filename, u32 a_imageType, u32& a_w,
	u32& a_h, u8& a_bpp, void*& a_imgPalette)
{
	void* imageData = nullptr;
	// Get a new file stream to load from file
	std::fstream file;
	file.open(a_filename, std::ios_base::in | std::ios_base::binary);
	if (file.is_open())
	{
		file.ignore(std::numeric_limits < std::streamsize>::max());
		std::streamsize fileLength = file.gcount();
		file.clear();
		file.seekg(0, std::ios_base::beg);
		if (fileLength == 0)
		{
			file.close();
			return nullptr;
		}
		

		// Switch statement to choose which custom image loader to call
		switch (a_imageType)
		{
		case (IM_BITMAP):{ // Add this yourself Alex!
			break;
		}
		case (IM_PCX):{
			imageData = PCXLoader::LoadFromfile(&file, a_w, a_h, a_bpp, a_imgPalette);
			if (a_bpp != 32)
			{
				imageData = PCXLoader::ConvertTo32bpp(imageData, a_imgPalette, a_w, a_h, a_bpp);
			}
			break;
		}
		case(IM_PPM): { // Add this from the tutorial already done!
			break;
		}
		default:
		{
			break;
		}
		}
		file.close();
	}
	return imageData;
}
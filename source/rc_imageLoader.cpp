#include <stdafx.h>
#include "rc_ImageLoader.h"
#include "rc_pcxLoader.h"

void* ImageLoader::LoadFromFile(const char* a_filename, u32 a_type, u32& a_w,
	u32& a_h, u8& a_bbp, void*& a_imgPalette)
{
	UNREFERENCED_PARAMETER(a_imgPalette);
	UNREFERENCED_PARAMETER(a_bbp);
	UNREFERENCED_PARAMETER(a_h);
	UNREFERENCED_PARAMETER(a_w);
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
		switch (a_type)
		{
		case IM_PCX:
		{
			break;
		}
		case IM_BITMAP:
		{
			break;
		}

		default:
		{
			file.close();
			return nullptr;
		}
		}
		file.close();
	}
	return nullptr;
}
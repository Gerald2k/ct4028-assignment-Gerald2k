#include <stdafx.h>
#include "rc_pcxLoader.h"


void* PCXLoader::LoadFromfile(const void* a_stream, u32& a_w, u32& a_h, u8& a_bpp, void*& a_imgPalette)
{
	UNREFERENCED_PARAMETER(a_imgPalette);
	UNREFERENCED_PARAMETER(a_bpp);
	UNREFERENCED_PARAMETER(a_h);
	UNREFERENCED_PARAMETER(a_w);

	std::fstream* file = (std::fstream*)a_stream;
	PCXHeader header;
	// read 128 bytes of data from the file
	file->read((char*)(&header), sizeof(PCXHeader));
	// check for valid header
	if (header.identifier != PCX_VALID_HEADER || header.encoding != PCX_RLE_ENCODING)
	{
		file->close();
		return nullptr;

	}

	// This is a valid pcx continue loading
	// Get palette info if present
	if (header.version == 3)
	{
		// Ver3 had no palette and used a deault palette
		u8* egaPalette = (u8*)(header.colourPalette);
			for (int i = 0; i < 16; ++i)
			{
				egaPalette[i] = PCX_defaultPalette[i];
			}
	}

	if (header.version == 8)
	{
		// If there are less than 8 bits per pixel no palette data outside 
		// of exisiting header palette
		file->seekg(PCX_VGA_PALETTE_OFFSET, std::ios_base::end);
		char paletteIndicator = 0;
		file->read(&paletteIndicator, 1);
		if (paletteIndicator == 0xC)
		{
			a_imgPalette = new PCXHeader::PCXPaletteColour[256];
			file->read((char*)a_imgPalette, 256 * sizeof(PCXHeader::PCXPaletteColour));
		}
		file->clear();
		file->seekg(0, std::ios_base::beg);
		file->seekg(sizeof(PCXHeader), std::ios_base::beg);
	}
	if (!a_imgPalette)
	{
		a_imgPalette = new PCXHeader::PCXPaletteColour[16];
		memcpy(a_imgPalette, header.colourPalette, 48);
	}

	return nullptr;
}
#include <stdafx.h>
#include "rc_pcxLoader.h"

int PCX_getEncodedByte(u8& a_value, u8& a_frequency, std::fstream* a_stream)
{
	if (a_stream->peek() == EOF)
	{
		return EOF;
	}
	a_frequency = 1;
	a_stream->read(char*)(&a_value), 1);
	if ((a_value & PCX_RLE_MASK) == PCX_RLE_MASK)
	{
		a_frequency = a_value & PCX_RLE_FREQ_MASK;
		if (a_stream->peek() == EOF)
		{
			return EOF;
		}
		a_stream->read((char*)(&a_value), 1);
	}
	return 0;
}

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
			for (int i = 0; i < 48; ++i)
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

	// Get pixel size of image
	a_w = header.dimensions.right - header.dimensions.left + 1;
	a_h = header.dimensions.bottom - header.dimensions.top + 1;
	a_bpp = header.bitsPerPixel * header.numColourPlanes;

	// Size of the decompressed image in bytes
	u32 bytesInRow = (u32)(a_w * (float)(a_bpp / 8.0f));
	u32 decompImageSize = a_h * bytesInRow;

	// Get the pixel size of the image
	a_w = header.dimensions.right - heaer.dimesnsions.left + 1;
	a_h = header.dimensions.bottom - heaer.dimesnsions.top + 1;
	a_bpp = header.bitsPerPixel * header.numColourPlanes;

	u32 bytesInRow = (u32)(a_w * (float)(a_bpp / 8.f));
	u32 decompImageSize = a_h * bytesInRow;

	u32 decompScanLine = header.bytesPerScanLine * header.numColourPlanes;

	u32 scanlinePadding = decompScanLine - bytesInRow;
	u32 actualBytesPerImageRow = decompScanLine - scanlinePadding;

	u8* ImageData = new u8[decompImageSize];
	memset(ImageData, 0, decompImageSize);
	u8* scanlineData = new u8[decompScanLine];
	memset(scanlineData, 0, decompScanLine);

	u8 value = 0;
	u8 frequency = 0;

	u32 bytesProcessed = 0;
	std::streamsize streamLocation;
	u32 row = 0;
	while (row < a_h - 1)
	{
		streamLocation = file->tellg();

		for (u8* slp = scanlineData; slp < (scanlineData + decompScanLine);)
		{
			if (EOF == PCX_getEncodedByte(value, frequency, file))
			{
				delete[] imageData;
				ImageData = nullptr;
				if (!a_palette)
				{
					delete[] a_palette;
					a_palette = nullptr;
				}
				return ImageData;

			}
		}
	}
}
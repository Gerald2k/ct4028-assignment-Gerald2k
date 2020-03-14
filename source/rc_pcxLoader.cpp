#include <stdafx.h>
#include "rc_pcxLoader.h"

int PCX_getEncodedByte(u8& a_value, u8& a_frequency, std::fstream* a_stream)
{
	if (a_stream->peek() == EOF)						// Test for unexpected End of file
	{
		return EOF;
	}
	a_frequency = 1;									// Default set frequency to 1
	a_stream->read((char*)(&a_value), 1);				// Read in one character from file
	if ((a_value & PCX_RLE_MASK) == PCX_RLE_MASK)		// Test to see if value is a run length frequency
	{
		a_frequency = a_value & PCX_RLE_FREQ_MASK;		// This is a run length and not a value use frequency mask to convert
		if (a_stream->peek() == EOF)					// End of file check
		{
			return EOF;
		}
		a_stream->read((char*)(&a_value), 1);			// read the next byte to get the pixel value
	}
	return 0;
}

/*! A function for loading a PCX image, returns pointer to raw image data
	\param a_stream - constant void pointer to the filestream to read from.
	\param a_width - an int reference for width in pixels of the iamge the load function will populate this value
	\param a_height - an int reference for height in pixels of the iamge the load function will populate this value
	\param a_bitsPerPixel - an int reference for the numver of bits per pixel
	\param a_imagePalette - pointer to image palette data stored in RGB format, for non palettised images pass nullptr as parameter
	\return the raw image data
*/
void* PCXLoader::LoadFromfile(const void* a_stream, u32& a_w, u32& a_h, u8& a_bpp, void*& a_imgPalette)
{

	std::fstream* file = (std::fstream*)a_stream; // Get pointer to file stream
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

	if (header.bitsPerPixel == 8)
	{
		// If there are less than 8 bits per pixel no palette data outside 
		// of exisiting header palette
		file->seekg(PCX_VGA_PALETTE_OFFSET, std::ios_base::end);	// Seek to the palette offset location 
		char paletteIndicator = 0;
		file->read(&paletteIndicator, 1);							// Read in the single byte at this offset location
		if (paletteIndicator == 0xC)								// If the byte is 0xc then this image has VGA palette data
		{
			// We have a palette at the end of the file proceed to read palette data from file
			a_imgPalette = new PCXHeader::PCXPaletteColour[256];
			file->read((char*)a_imgPalette, 256 * sizeof(PCXHeader::PCXPaletteColour));
		}
		file->clear();
		file->seekg(0, std::ios_base::beg);
		file->seekg(sizeof(PCXHeader), std::ios_base::beg);			// Seek from the beginning of the file to the end of the PCX header	
	}

	// If we get here and a_palette is still null then allocate memeory for 16 colour palette
	if (!a_imgPalette && (header.numColourPlanes * header.bitsPerPixel) < 24)
	{
		a_imgPalette = new PCXHeader::PCXPaletteColour[16];
		memcpy(a_imgPalette, header.colourPalette, 48);
	}

	// Get pixel size of image
	a_w = header.dimensions.right - header.dimensions.left + 1; // Width of the image in pixel
	a_h = header.dimensions.bottom - header.dimensions.top + 1; // Number of scanlines in the image (pixel height)
	a_bpp = header.bitsPerPixel * header.numColourPlanes;

	// Size of the decompressed image in bytes
	u32 bytesInRow = (u32)(a_w * (float)(a_bpp / 8.0f));
	u32 decompImageSize = a_h * bytesInRow;

	// The way we will process the iamge data is to decompress on image scanline at a time
	// Number of bytes in a decompressed scanline (when colour planes greater than 1 bytes per scanline give split between R/G/B values)
	u32 decompScanLine = header.bytesPerScanLine * header.numColourPlanes;

	// PCX images may contain some line padding - calcualte line padding
	u32 scanlinePadding = decompScanLine - bytesInRow;
	u32 actualBytesPerImageRow = decompScanLine - scanlinePadding;

	// Create a data buffer large enough to hold the decompressed image
	u8* imageData = new u8[decompImageSize];
	memset(imageData, 0, decompImageSize);
	u8* scanlineData = new u8[decompScanLine];
	memset(scanlineData, 0, decompScanLine);

	// Create some stack variables to hold the value and frequency fpr the data read from file
	u8 value = 0;		// Current pixel value to be decoded
	u8 frequency = 0;	// The occurances of this pixel value

	// While all of the image data has not been extracted
	u32 bytesProcessed = 0;
	std::streamsize streamLocation;
	u32 row = 0;
	while (row < a_h - 1)
	{
		streamLocation = file->tellg();
		// For each row of the image decode the compressed image data
		for (u8* slp = scanlineData; slp < (scanlineData + decompScanLine);)
		{
			// test for premature end of file
			if (EOF == PCX_getEncodedByte(value, frequency, file))
			{
				// If file ends suddenly release and null data
				delete[] imageData;
				imageData = nullptr;
				if (!a_imgPalette)
				{
					delete[] a_imgPalette;
					a_imgPalette = nullptr;
				}
				return imageData;
			}
			// For the number of runs insert the value into out image data
			for (u8 i = 0; i < frequency; ++i)
			{
				*slp++ = value;
			}
		}
		++row;
		// Completeing the loop above gives us one scanline of data decompressed to copy into our Image buffer
		// Now copy based off number of colour planes
		if (header.numColourPlanes != 1)
		{
			// Scan line is broekn down to rrrrr... ggggg... bbbbb... (aaaaa)...
			// Need to interate throught this image and copy across data to appropiate RGB channels
			u8* red = scanlineData;
			u8* green = scanlineData + header.bytesPerScanLine;
			u8* blue = scanlineData + (header.bytesPerScanLine * 2);
			u8* alpha = header.numColourPlanes == 4 ? scanlineData + (header.bytesPerScanLine * 3) : nullptr;

			for (u32 processedBytes = bytesProcessed; processedBytes < (bytesProcessed + actualBytesPerImageRow);)
			{
				if (header.bitsPerPixel == 8)
				{
					imageData[processedBytes + 0] = *red++;
					imageData[processedBytes + 1] = *green++;
					imageData[processedBytes + 2] = *blue++;
					if (alpha != nullptr)
					{
						imageData[processedBytes + 3] = *alpha++;
					}
					processedBytes += header.numColourPlanes;
				}
				else
				{
					// Format not supported yet
				}
			}
		}
		else
		{
			memcpy(&imageData[bytesProcessed], scanlineData, actualBytesPerImageRow);
		}
		bytesProcessed += actualBytesPerImageRow;
		
	}
	return imageData;
}

void* PCXLoader::ConvertTo32bpp(void* a_imageData, void* a_palette, u32& a_w, u32& a_h, u8& a_bpp)
{
	u8* rawImage = new u8[a_w * a_h * 4];
	u32 currentDataSize = a_w * (u32)(a_h * ((float)a_bpp / 8.f));
	u8* currentImage = (u8*)a_imageData;
	if (a_palette != nullptr) // Convert a palettised image
	{
		PCXHeader::PCXPaletteColour* palette = (PCXHeader::PCXPaletteColour*)a_palette;
		// For each pixel in the current data set
		for (u32 pixel = 0, i = 0; pixel < currentDataSize; ++pixel, i += 4)
		{
			u32 pi = currentImage[pixel];
			if (a_bpp == 8)
			{
				rawImage[i + 0] = palette[pi].B;
				rawImage[i + 1] = palette[pi].G;
				rawImage[i + 2] = palette[pi].R;
				rawImage[i + 3] = 0;
			}
			else if (a_bpp == 4)
			{
				rawImage[i + 0] = palette[(pi >> 4) & 0xf].B;
				rawImage[i + 1] = palette[(pi >> 4) & 0xf].G;
				rawImage[i + 2] = palette[(pi >> 4) & 0xf].R;
				rawImage[i + 3] = 0;
				i += 4;
				rawImage[i + 0] = palette[(pi >> 4) & 0xf].B;
				rawImage[i + 1] = palette[(pi >> 4) & 0xf].G;
				rawImage[i + 2] = palette[(pi >> 4) & 0xf].R;
				rawImage[i + 3] = 0;
			}
			else
			{
				// Format not supported yet
			}
		}

	}
	else // convert an RGB image to RGBA
	{
		for (u32 pixel = 0, i = 0; pixel < currentDataSize; pixel += 3, i += 4)
		{
			rawImage[i + 0] = currentImage[pixel + 2];
			rawImage[i + 1] = currentImage[pixel + 1];
			rawImage[i + 2] = currentImage[pixel + 0];
			rawImage[i + 3] = 0;
		}
	}
	delete[] a_imageData;
	a_imageData = nullptr;
	delete[] a_palette;
	a_palette = nullptr;
	return rawImage;
}
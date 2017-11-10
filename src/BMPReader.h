
#ifndef _BMPReader_H
#define _BMPReader_H

#include <Arduino.h>
#include <SdFat.h>

class BMPReader
{
public:
	enum error_t {
		eNoError = 0,
		eFileNotFound,
		eSignature,
		eFileHeader,
		ePlanes,
		eDepth,
		eCompression
	};

	BMPReader(SdFat &sd) : sd(sd) {
		err = eNoError;
	};

	bool open(const char *path);
	bool close();
	bool isOpen();
	int16_t width();
	int16_t height();
	int16_t depth();
	error_t error();
	void printInfo(Stream &s);

	uint16_t getPixel(uint32_t &p, uint16_t x, uint16_t y);
	uint16_t getPixels(uint32_t *p, uint16_t x, uint16_t y, uint16_t num);
	
protected:
	SdFat sd;
	File f;
	error_t err;

	// Width height, width, pixel depth
	int16_t w, h, dpt;

	// Data start in file, bytes per row (32 bit aligned)
	uint32_t fileSize, dataOffset, rowSize;

	// Bitmap stored in reverse
	bool rev;

	// Bytes per pixel
	uint8_t bpp; 
	
	uint16_t read16();
	uint32_t read32();
};

#endif /* _BMPReader_H */


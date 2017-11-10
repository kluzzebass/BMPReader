
#include <BMPReader.h>

bool BMPReader::open(const char *path)
{
	err = eNoError;

	if (f.isOpen()) f.close();

	if (!f.open(path, O_READ))
	{
		err = eFileNotFound;
		return false;
	}

	// 0x4d42 is ASCII "BM" as a little endian 16bit integer. I miss big endianism. :(
	if (read16() != 0x4d42) {
		err = eSignature;
		f.close();
		return false;
	}

	fileSize = read32();
	read32(); // skip garbage

	// Where in the file are the bitmap data located?
	dataOffset = read32();

	// Skip 32bit DIB header size
	read32();

	// Get the width and height of the bitmap
	w = read32();
	h = read32();

	// The spec says this has to be 1, LOL.
	if (read16() != 1)
	{
		err = ePlanes;
		f.close();
		return false;
	}

	// We only like 24 and 32 bit... maps. ¯\_(ツ)_/¯
	dpt = read16();
	if (dpt != 24 && dpt != 32)
	{
		err = eDepth;
		f.close();
		return false;
	}
	bpp = dpt >> 3;

	// We only support uncompressed bitmaps.
	if (read32())
	{
		err = eCompression;
		f.close();
		return false;
	}

	// Calculate 32 bit padded row size
	rowSize = ((dpt * w + 31) >> 3) & ~3;

	// Determine row direction
	if (h < 0) {
		h = -h;
		rev = false;
	}

	return true;
}

bool BMPReader::close()
{
	return f.close();
}

bool BMPReader::isOpen()
{
	return f.isOpen();
};


int16_t BMPReader::width() { return w; }
int16_t BMPReader::height() { return h; }
int16_t BMPReader::depth() { return dpt; }
BMPReader::error_t BMPReader::error() { return err; }

uint16_t BMPReader::read16()
{
	uint16_t r;
	f.read(&r, 2);
	return r;
}
  
uint32_t BMPReader::read32()
{
	uint32_t r;
	f.read(&r, 4);
	return r;
}

void BMPReader::printInfo(Stream &s)
{
	s.print(F("File Size:     "));
	s.println(fileSize);
	s.print(F("Data Offset:     "));
	s.println(dataOffset);
	s.print(F("Width:           "));
	s.println(w);
	s.print(F("Height:          "));
	s.println(h);
	s.print(F("Bit Depth:       "));
	s.println(dpt);
	s.print(F("Bytes Per Pixel: "));
	s.println(bpp);
	s.print(F("Row Size:        "));
	s.println(rowSize);
	s.print(F("Reverse:         "));
	s.println(rev ? F("true") : F("false"));
}

uint16_t BMPReader::getPixel(uint32_t &p, uint16_t x, uint16_t y)
{
	if (y >= h || x >= w) return 0;

	uint16_t pos = ((rev ? y : (h - y - 1)) * rowSize) + (bpp * x) + dataOffset;

	if (!f.seek(pos)) return 0;
	if (!f.read(&p, sizeof(p))) return 0;
	return 1;
}

uint16_t BMPReader::getPixels(uint32_t *p, uint16_t x, uint16_t y, uint16_t num)
{
	if (y >= h || x >= w) return 0;

	uint32_t pos = ((rev ? y: (h - y - 1)) * rowSize) + (bpp * x) + dataOffset;
	uint16_t len = ((x + num) < w) ? num : w - x;
	if (!f.seek(pos)) return 0;
	uint16_t r = f.read(p, len * bpp) / bpp;

	// If the bit depth is 32, there's no need to rearrange the bytes
	if (dpt == 32)
		return r;

	// Stretch 24bit pixels to 32bit.
	uint8_t *bp = (uint8_t *)p;
	for (uint16_t i = 0; i < r; i++)
	{
		uint16_t j = (r - i - 1) * bpp;
		uint32_t t = bp[j] | bp[j + 1] << 8 | bp[j + 2] << 16;
		p[len - i - 1] = t;
	}

	return r;
}
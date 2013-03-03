/*
 *	This file is a part of Light PNG Loader library.
 *	Copyright (c) 2009-2011 Alex Pankratov. All rights reserved.
 *
 *	http://swapped.cc/lpng
 */

/*
 *	The program is distributed under terms of BSD license. 
 *	You can obtain the copy of the license by visiting:
 *
 *	http://www.opensource.org/licenses/bsd-license.php
 */

/*
 *	lpng.c, version 16-01-2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "puff.h"
#include "lpng.h"

/*
 *	The following constant limits the maximum size of a PNG 
 *	image. Larger images are not loaded. 
 *
 *	The limit is arbitrary and can be increased as needed.
 *	It exists solely to prevent the accidental loading of 
 *	very large images.
 */
#define MAX_IDAT_SIZE (128*1024)

/*
 *
 */
#pragma warning (disable: 4996)

typedef unsigned char  uchar;
typedef unsigned long  ulong;
typedef struct _png    png_t;
typedef struct _buf    buf_t;

struct _png
{
	ulong w;
	ulong h;
	uchar bpp; /* 3 - truecolor, 4 - truecolor + alpha */
	uchar pix[1];
};

struct _buf
{
	uchar * ptr;
	ulong   len;
};

typedef int (* read_cb)(uchar * buf, ulong len, void * arg);

//
static __inline ulong get_ulong(uchar * v)
{
	ulong r = v[0];
	r = (r << 8) | v[1];
	r = (r << 8) | v[2];
	return (r << 8) | v[3];
}

static __inline uchar paeth(uchar a, uchar b, uchar c)
{
	int p = a + b - c;
        int pa = p > a ? p-a : a-p;
	int pb = p > b ? p-b : b-p;
	int pc = p > c ? p-c : c-p;
        return (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
}

static const uchar png_sig[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

/*
 *
 */
static png_t * LoadPngEx(read_cb read, void * read_arg)
{
	png_t * png = NULL;

	uchar buf[64];
	ulong w, h, i, j;
	uchar bpp;
	ulong len;
	uchar * tmp, * dat = 0;
	ulong dat_max, dat_len;
	ulong png_max, png_len;
	ulong out_len;
	int   r;
	uchar * line, * prev;

	/* sig and IHDR */
	if (! read(buf, 8 + 8 + 13 + 4, read_arg))
		return NULL;

	if (memcmp(buf, png_sig, 8) != 0)
		return NULL;

	len = get_ulong(buf+8);
	if (len != 13)
		return NULL;
	
	if (memcmp(buf+12, "IHDR", 4) != 0)
		return NULL;

	w = get_ulong(buf+16);
	h = get_ulong(buf+20);

	if (buf[24] != 8)
		return NULL;
	
	/* truecolor pngs only please */
	if ((buf[25] & 0x03) != 0x02)
		return NULL;

	/* check for the alpha channel */
	bpp = (buf[25] & 0x04) ? 4 : 3;

	if (buf[26] != 0 || buf[27] != 0 || buf[28] != 0)
		return NULL;

	/* IDAT */
	dat_max = 0;
	for (;;)
	{
		if (! read(buf, 8, read_arg))
			goto err;

		len = get_ulong(buf);
		if (memcmp(buf+4, "IDAT", 4) != 0)
		{
			if (! read(0, len+4, read_arg))
				goto err;

			if (memcmp(buf+4, "IEND", 4) == 0)
				break;
		
			continue;
		}

		/*
		 *	see comment at the top of this file
		 */
		if (len > MAX_IDAT_SIZE)
			goto err;

		if (len+4 > dat_max)
		{
			dat_max = len+4;
			tmp = realloc(dat, dat_max);
			if (! tmp)
				goto err;
			dat = tmp;
		}

		if (! read(dat, len+4, read_arg))
			goto err;

		if ((dat[0] & 0x0f) != 0x08 ||  /* compression method (rfc 1950) */
		    (dat[0] & 0xf0) > 0x70)     /* window size */
			goto err;

		if ((dat[1] & 0x20) != 0)       /* preset dictionary present */
			goto err;

		if (! png)
		{
			png_max = (w * bpp + 1) * h;
			png_len = 0;
			png = malloc(sizeof(*png) - 1 + png_max);
			if (! png)
				goto err;
			png->w = w;
			png->h = h;
			png->bpp = bpp;
		}

		out_len = png_max - png_len;
		dat_len = len - 2;
		r = puff(png->pix + png_len, &out_len, dat+2, &dat_len);
		if (r != 0)
			goto err;
		if (2+dat_len+4 != len)
			goto err;
		png_len += out_len;
	}
	free(dat);

	/* unfilter */
	len = w * bpp + 1;
	line = png->pix;
	prev = 0;
	for (i=0; i<h; i++, prev = line, line += len)
	{
		switch (line[0])
		{
		case 0: /* none */
			break;
		case 1: /* sub */
			for (j=1+bpp; j<len; j++)
				line[j] += line[j-bpp];
			break;
		case 2: /* up */
			if (! prev)
				break;
			for (j=1; j<len; j++)
				line[j] += prev[j];
			break;
		case 3: /* avg */
			if (! prev)
				goto err; /* $todo */
			for (j=1; j<=bpp; j++)
				line[j] += prev[j]/2;
			for (   ; j<len; j++)
				line[j] += (line[j-bpp] + prev[j])/2;
			break;
		case 4: /* paeth */
			if (! prev)
				goto err; /* $todo */
			for (j=1; j<=bpp; j++)
				line[j] += prev[j];
			for (   ; j<len; j++)
				line[j] += paeth(line[j-bpp], prev[j], prev[j-bpp]);			
			break;
		default:
			goto err;
		}
	}
	
	return png;
err:
	free(png);
	free(dat);
	return NULL;
}

/*
 *
 */
static int file_reader(uchar * buf, ulong len, void * arg)
{
	FILE * fh = arg;
	return buf ? 
		fread(buf, 1, len, fh) == len :
		fseek(fh, len, SEEK_CUR) == 0;
}

static png_t * LoadPngFile(const wchar_t * name)
{
	png_t * png = NULL;
	FILE  * fh;

	fh = _wfopen(name, L"rb");
	if (fh)
	{
		png = LoadPngEx(file_reader, fh);
		fclose(fh); 
	}

	return png;
}

/*
 *
 */
static int data_reader(uchar * buf, ulong len, void * arg)
{
	buf_t * m = arg;
	
	if (len > m->len)
		return 0;

	if (buf)
		memcpy(buf, m->ptr, len);

	m->len -= len;
	m->ptr += len;
	return 1;
}

static png_t * LoadPngResource(const wchar_t * name, const wchar_t * type, HMODULE module)
{
	HRSRC   hRes;
	HGLOBAL hResData;
	buf_t   buf;

	hRes = FindResource(module, name, type);
	if (! hRes)
		return NULL;

	if (! SizeofResource(module, hRes))
		return NULL;

	if (! (hResData = LoadResource(module, hRes)))
		return NULL;

	if (! (buf.ptr = LockResource(hResData)))
		return NULL;

	buf.len = SizeofResource(module, hRes);

	return LoadPngEx(data_reader, &buf);
}

/*
 *
 */
static HBITMAP PngToDib(png_t * png, BOOL premultiply)
{
	BITMAPINFO bmi = { sizeof(bmi) };
	HBITMAP dib;
	uchar * dst;
	uchar * src, * tmp;
	uchar  alpha;
	int    bpl;
	ulong  x, y;

	if (png->bpp != 3 && png->bpp != 4)
		return NULL;

	//
	bmi.bmiHeader.biWidth = png->w;
	bmi.bmiHeader.biHeight = png->h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	
	dib = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &dst, NULL, 0);
	if (! dib)
		return NULL;

	bpl = png->bpp * png->w + 1;              // bytes per line
	src = png->pix + (png->h - 1) * bpl + 1;  // start of the last line

	for (y = 0; y < png->h; y++, src -= bpl)
	{
		tmp = src;

		for (x = 0; x < png->w; x++)
		{
			alpha = (png->bpp == 4) ? tmp[3] : 0xff; // $fixme - optimize

			/*
			 *	R, G and B need to be 'pre-multiplied' to alpha as 
			 *	this is the format that AlphaBlend() expects from
			 *	an alpha-transparent DIB
			 */
			if (premultiply)
			{
				dst[0] = tmp[2] * alpha / 255;
				dst[1] = tmp[1] * alpha / 255;
				dst[2] = tmp[0] * alpha / 255;
			}
			else
			{
				dst[0] = tmp[2];
				dst[1] = tmp[1];
				dst[2] = tmp[0];
			}
			dst[3] = alpha;

			dst += 4;
			tmp += png->bpp;
		}
	}

	return dib;
}

/*
 *
 */
HBITMAP LoadPng(const wchar_t * res_name, 
		const wchar_t * res_type, 
		HMODULE         res_inst,
		BOOL            premultiply)
{
	HBITMAP  bmp;
	png_t  * png;

	if (res_type)
		png = LoadPngResource(res_name, res_type, res_inst);
	else
		png = LoadPngFile(res_name);

	if (! png)
		return NULL;

	bmp = PngToDib(png, premultiply);
	
	free(png);
	if (! bmp)
		return NULL;

	return bmp;
}

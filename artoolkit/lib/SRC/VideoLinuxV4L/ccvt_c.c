/*
   Colour conversion routines (RGB <-> YUV) in plain C
   (C) 2000 Nemosoft Unv.    nemosoft@smcc.demon.nl
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "ccvt.h"

#define PUSH_RGB24	1
#define PUSH_BGR24	2
#define PUSH_RGB32	3
#define PUSH_BGR32	4

/* This is a really simplistic approach. Speedups are welcomed. */
static void ccvt_420i(int width, int height, unsigned char *src, unsigned char *dst, int push)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	unsigned char *py, *pu, *pv;

	linewidth = width + (width >> 1);
	py = src;
	pu = py + 4;
	pv = pu + linewidth;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug =   88 * u;
	ub =  454 * u;
	v = *pv - 128;
	vg =  183 * v;
	vr =  359 * v;

	/* The biggest problem is the interlaced data, and the fact that odd
	   add even lines have V and U data, resp. 
	 */
	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy +      vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub     ) >> 8;
			
			switch(push) {
			case PUSH_RGB24:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				break;

			case PUSH_BGR24:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				break;
			
			case PUSH_RGB32:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 0;
				break;

			case PUSH_BGR32:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				*dst++ = 0;
				break;
			}
			
			y = *py++;
			yy = y << 8;
			if ((col & 3) == 3)
				py += 2; // skip u/v
			if (col & 1) {
				if ((col & 3) == 3) {
					pu += 4; // skip y
					pv += 4;
				}
				else {
					pu++;
					pv++;
				}
				u = *pu - 128;
				ug =   88 * u;
				ub =  454 * u;
				v = *pv - 128;
				vg =  183 * v;
				vr =  359 * v;
			}
		} /* ..for col */
		if (line & 1) { // odd line: go to next band
			pu += linewidth;
			pv += linewidth;
		}
		else { // rewind u/v pointers
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}

void ccvt_420i_rgb24(int width, int height, void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_RGB24);
}

void ccvt_420i_bgr24(int width, int height, void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_BGR24);
}

void ccvt_420i_rgb32(int width, int height, void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_RGB32);
}

void ccvt_420i_bgr32(int width, int height, void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_BGR32);
}


void ccvt_420i_420p(int width, int height, void *src, void *dsty, void *dstu, void *dstv)
{
	short *s, *dy, *du, *dv;
	int line, col;

	s = (short *)src;
	dy = (short *)dsty;
	du = (short *)dstu;
	dv = (short *)dstv;
	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col += 4) {
			*dy++ = *s++;
			*dy++ = *s++;
			if (line & 1)
				*dv++ = *s++;
			else
				*du++ = *s++;
		} /* ..for col */
	} /* ..for line */
}

void ccvt_420i_yuyv(int width, int height, void *src, void *dst)
{
	int line, col, linewidth;
	unsigned char *py, *pu, *pv, *d;

	linewidth = width + (width >> 1);
	py = (unsigned char *)src;
	pu = src + 4;
	pv = pu + linewidth;
	d = (unsigned char *)dst;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col += 4) {
			/* four pixels in one go */
			*d++ = *py++;
			*d++ = *pu++;
			*d++ = *py++;
			*d++ = *pv++;
			
			*d++ = *py++;
			*d++ = *pu++;
			*d++ = *py++;
			*d++ = *pv++;

			py += 2;
			pu += 4;
			pv += 4;
		} /* ..for col */
		if (line & 1) { // odd line: go to next band
			pu += linewidth;
			pv += linewidth;
		}
		else { // rewind u/v pointers
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}


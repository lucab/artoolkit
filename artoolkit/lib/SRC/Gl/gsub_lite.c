/*
 *	gsub_lite.c
 *
 *	Graphics Subroutines (Lite) for ARToolKit.
 *
 *	Copyright (c) 2003-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *	2.6.5	????-??-??	MB/HK	Original from ARToolKit-2.65DS gsub.c
 *  2.7.0   2003-08-13  PRL     Lipo'ed and whipped into shape.
 *  2.7.1   2004-03-03  PRL		Avoid defining BOOL if already defined
 *	2.7.1	2004-03-03	PRL		Don't enable lighting if it was not enabled.
 *	2.7.2	2004-04-27	PRL		Added headerdoc markup. See http://developer.apple.com/darwin/projects/headerdoc/
 *	2.7.3	2004-07-02	PRL		Much more object-orientated through use of ARGL_CONTEXT_SETTINGS type.
 *	2.7.4	2004-07-14	PRL		Added gluCheckExtension hack for GLU versions pre-1.3.
 *	2.7.5	2004-07-15	PRL		Added arglDispImageStateful(); removed extraneous glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,...) calls.
 *	2.7.6	2005-02-18	PRL		Go back to using int rather than BOOL, to avoid conflict with Objective-C.
 *
 */
/*
 * 
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

// ============================================================================
//	Private includes.
// ============================================================================
#include <AR/gsub_lite.h>

#include <stdio.h>		// fprintf(), stderr
#ifndef __APPLE__
#  include <GL/glu.h>
#  ifdef GL_VERSION_1_2
#    include <GL/glext.h>
#  endif
#else
#  include <OpenGL/glu.h>
#  include <OpenGL/glext.h>
#endif

// ============================================================================
//	Private types and defines.
// ============================================================================
#ifdef _MSC_VER
#  pragma warning (disable:4068)	// Disable MSVC warnings about unknown pragmas.
#endif

// Make some stuff handed to us by <AR/config.h> more useful.
#if defined(AR_PIX_FORMAT_ABGR)		// SGI.
#  define AR_PIX_INTFORMAT  GL_RGBA
#  ifdef GL_ABGR
#    define AR_PIX_FORMAT   GL_ABGR
#  else
#    define AR_PIX_FORMAT   GL_ABGR_EXT
#  endif
#  define AR_PIX_TYPE		GL_UNSIGNED_BYTE
#elif defined(AR_PIX_FORMAT_BGRA)	// Windows.
#  define AR_PIX_INTFORMAT  GL_RGBA
#  ifdef GL_BGRA
#    define AR_PIX_FORMAT   GL_BGRA
#  else
#    define AR_PIX_FORMAT   GL_BGRA_EXT
#  endif
#  define AR_PIX_TYPE		GL_UNSIGNED_BYTE
#elif defined(AR_PIX_FORMAT_RGBA)
#  define AR_PIX_INTFORMAT  GL_RGBA
#  define AR_PIX_FORMAT		GL_RGBA
#  define AR_PIX_TYPE		GL_UNSIGNED_BYTE
#elif defined(AR_PIX_FORMAT_ARGB)	// Mac OS X.
#  define AR_PIX_INTFORMAT  GL_RGBA
#  define AR_PIX_FORMAT		GL_BGRA
#  define AR_PIX_TYPE		GL_UNSIGNED_INT_8_8_8_8_REV
#elif defined(AR_PIX_FORMAT_RGB)
#  define AR_PIX_INTFORMAT  GL_RGB
#  define AR_PIX_FORMAT		GL_RGB
#  define AR_PIX_TYPE		GL_UNSIGNED_BYTE
#elif defined(AR_PIX_FORMAT_BGR)
#  define AR_PIX_INTFORMAT  GL_RGB
#  ifdef GL_BGR
#    define AR_PIX_FORMAT   GL_BGR
#  else
#    define AR_PIX_FORMAT   GL_BGR_EXT
#  endif
#  define AR_PIX_TYPE		GL_UNSIGNED_BYTE
#elif defined(AR_PIX_FORMAT_2vuy)   // Mac OS X, component video.
#  define AR_PIX_INTFORMAT  GL_RGB
#  define AR_PIX_FORMAT		GL_YCBCR_422_APPLE
#  define AR_PIX_TYPE		GL_UNSIGNED_SHORT_8_8_REV_APPLE
#elif defined(AR_PIX_FORMAT_yuvs)   // Mac OS X, component video.
#  define AR_PIX_INTFORMAT  GL_RGB
#  define AR_PIX_FORMAT		GL_YCBCR_422_APPLE
#  define AR_PIX_TYPE		GL_UNSIGNED_SHORT_8_8_APPLE
#else
#  error Unknown pixel format defined in config.h.
#endif

// Make sure that required OpenGL constant definitions are available at compile-time.
// N.B. These should not be used unless the renderer indicates (at run-time) that it supports them.
#ifndef GL_VERSION_1_2
#  if GL_SGIS_texture_edge_clamp
#    define GL_CLAMP_TO_EDGE				GL_CLAMP_TO_EDGE_SGIS
#  else
#    define GL_CLAMP_TO_EDGE				0x812F
#  endif
#endif
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
#  if GL_NV_texture_rectangle
#    define GL_TEXTURE_RECTANGLE			GL_TEXTURE_RECTANGLE_NV
#    define GL_PROXY_TEXTURE_RECTANGLE		GL_PROXY_TEXTURE_RECTANGLE_NV
#    define GL_MAX_RECTANGLE_TEXTURE_SIZE   GL_MAX_RECTANGLE_TEXTURE_SIZE_NV
#  elif GL_EXT_texture_rectangle
#    define GL_TEXTURE_RECTANGLE			GL_TEXTURE_RECTANGLE_EXT
#    define GL_PROXY_TEXTURE_RECTANGLE		GL_PROXY_TEXTURE_RECTANGLE_EXT
#    define GL_MAX_RECTANGLE_TEXTURE_SIZE   GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT
#  else
#    define GL_TEXTURE_RECTANGLE			0x84F5
#    define GL_PROXY_TEXTURE_RECTANGLE		0x84F7
#    define GL_MAX_RECTANGLE_TEXTURE_SIZE   0x84F8
#  endif
#endif

//#define ARGL_DEBUG

struct _ARGL_CONTEXT_SETTINGS {
	int		textureRectangleCapabilitiesChecked;
	int		texturePow2CapabilitiesChecked;
	GLuint		textureRectangle;
	GLuint		texturePow2;
	GLuint		listRectangle;
	GLuint		listPow2;
	int		initedRectangle;
	int			initedRectangleTexmapScaleFactor;
	int		initedPow2;
	int			initedPow2TexmapScaleFactor;
	GLsizei		texturePow2SizeX;
	GLsizei		texturePow2SizeY;
	GLenum		texturePow2WrapMode;
};
typedef struct _ARGL_CONTEXT_SETTINGS ARGL_CONTEXT_SETTINGS;

// ============================================================================
//	Public globals.
// ============================================================================

// It'd be nice if we could wrap these in accessor functions!
int	arglDrawMode   = DEFAULT_DRAW_MODE;
int	arglTexmapMode = DEFAULT_DRAW_TEXTURE_IMAGE;
int arglTexRectangle = TRUE; // TRUE | FALSE .

// These items relate to Apple's fast texture transfer support.
//#define ARGL_USE_TEXTURE_RANGE	// Commented out due to conflicts with GL_APPLE_ycbcr_422 extension.
#if defined(__APPLE__) && defined(APPLE_TEXTURE_FAST_TRANSFER)
int arglAppleClientStorage = TRUE; // TRUE | FALSE .
#  ifdef ARGL_USE_TEXTURE_RANGE
int arglAppleTextureRange = TRUE; // TRUE | FALSE .
GLuint arglAppleTextureRangeStorageHint = GL_STORAGE_SHARED_APPLE; // GL_STORAGE_PRIVATE_APPLE | GL_STORAGE_SHARED_APPLE | GL_STORAGE_CACHED_APPLE .
#  else
int arglAppleTextureRange = FALSE; // TRUE | FALSE .
GLuint arglAppleTextureRangeStorageHint = GL_STORAGE_PRIVATE_APPLE; // GL_STORAGE_PRIVATE_APPLE | GL_STORAGE_SHARED_APPLE | GL_STORAGE_CACHED_APPLE .
#  endif // ARGL_USE_TEXTURE_RANGE
#endif // __APPLE__ && APPLE_TEXTURE_FAST_TRANSFER

// ============================================================================
//	Private globals.
// ============================================================================


#pragma mark -
// ============================================================================
//	Private functions.
// ============================================================================

//
// Convert a camera parameter structure into an OpenGL projection matrix.
//
static void arglConvGLcpara(ARParam *param, double focalmin, double focalmax, double m[16])
{
    double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
    int      i, j;

    if(arParamDecompMat(param->mat, icpara, trans) < 0) {
        fprintf(stderr, "arglConvGLcpara(): arParamDecompMat() indicated parameter error.\n");
        return;
    }

    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / param->xsize);
    q[0][1] = (2.0 * p[0][1] / param->xsize);
    q[0][2] = ((2.0 * p[0][2] / param->xsize)  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = (2.0 * p[1][1] / param->ysize);
    q[1][2] = ((2.0 * p[1][2] / param->ysize) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (focalmax + focalmin)/(focalmax - focalmin);
    q[2][3] = -2.0 * focalmax * focalmin / (focalmax - focalmin);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = 1.0;
    q[3][3] = 0.0;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 3; j++) {
            m[i+j*4] = q[i][0] * trans[0][j]
			+ q[i][1] * trans[1][j]
			+ q[i][2] * trans[2][j];
        }
        m[i+3*4] = q[i][0] * trans[0][3]
		+ q[i][1] * trans[1][3]
		+ q[i][2] * trans[2][3]
		+ q[i][3];
    }
}

#ifndef GLU_VERSION_1_3
#  include <string.h> // strchr(), strstr(), strlen()
//
//  Provide a gluCheckExtension() function for platforms that don't have GLU version 1.3 or later.
//
GLboolean gluCheckExtension(const GLubyte* extName, const GLubyte *extString)
{
	const GLubyte *start;
	GLubyte *where, *terminator;
	
	/* Extension names should not have spaces. */
	where = (GLubyte *) strchr(extName, ' ');
	if (where || *extName == '\0')
		return GL_FALSE;
	/* It takes a bit of care to be fool-proof about parsing the
		OpenGL extensions string. Don't be fooled by sub-strings,
		etc. */
	start = extString;
	for (;;) {
		where = (GLubyte *) strstr((const char *) start, extName);
		if (!where)
			break;
		terminator = where + strlen(extName);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return GL_TRUE;
		start = terminator;
	}
	return GL_FALSE;
}
#endif // GLU_VERSION_1_3

//
//  Checks for the presence of an OpenGL capability by version or extension.
//  Reports whether the current OpenGL driver's OpenGL implementation version
//  meets or exceeds a minimum value passed in in minVersion (represented as a binary-coded
//  decimal i.e. version 1.0 is represented as 0x0100). If minVersion is zero, the
//  version test will always fail. Alternately, the test is satisfied if an OpenGL extension
//  identifier passed in as a character string
//  is non-NULL, and is found in the current driver's list of supported extensions.
//  Returns: TRUE If either of the tests passes, or FALSE if both fail.
//
static int arglGLCapabilityCheck(const unsigned short minVersion, const char *extension)
{
	const GLubyte * strRenderer;
	const GLubyte * strVersion;
	const GLubyte * strVendor;
	const GLubyte * strExtensions;
	short j, shiftVal;
	unsigned short version = 0; // binary-coded decimal gl version (ie. 1.4 is 0x0140).
	
	strRenderer = glGetString(GL_RENDERER);
	strVendor = glGetString(GL_VENDOR);
	strVersion = glGetString(GL_VERSION);
	j = 0;
	shiftVal = 8;
	// Construct BCD version.
	while (((strVersion[j] <= '9') && (strVersion[j] >= '0')) || (strVersion[j] == '.')) { // Get only basic version info (until first non-digit or non-.)
		if ((strVersion[j] <= '9') && (strVersion[j] >= '0')) {
			version += (strVersion[j] - '0') << shiftVal;
			shiftVal -= 4;
		}
		j++;
	}
	strExtensions = glGetString(GL_EXTENSIONS);
	
	if (0 < minVersion && version >= minVersion) return (TRUE);
	if (extension && gluCheckExtension(extension, strExtensions)) return (TRUE);
	return (FALSE);
}

#ifdef AR_OPENGL_TEXTURE_RECTANGLE
static int arglDispImageTexRectangleCapabilitiesCheck(const ARParam *cparam, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint textureRectangleSizeMax;
	GLint format;

    if (!arglGLCapabilityCheck(0, "GL_NV_texture_rectangle")) {
		if (!arglGLCapabilityCheck(0, "GL_EXT_texture_rectangle")) { // Alternate name.
			return (FALSE);
		}
	}
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &textureRectangleSizeMax);
	if (cparam->xsize > textureRectangleSizeMax || cparam->ysize > textureRectangleSizeMax) {
		return (FALSE);
	}
	
	// Now check that the renderer can accomodate a texture of this size.
	glTexImage2D(GL_PROXY_TEXTURE_RECTANGLE, 0, AR_PIX_INTFORMAT, cparam->xsize, cparam->ysize, 0, AR_PIX_FORMAT, AR_PIX_TYPE, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_RECTANGLE, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	if (!format) {
		return (FALSE);
	}
	
	contextSettings->textureRectangleCapabilitiesChecked = TRUE;	

	return (TRUE);
}

//
// Blit an image to the screen using OpenGL rectangle texturing.
// TODO: a cleanup routine to delete texture name, display list.
//
static void arglDispImageTexRectangle(ARUint8 *image, const ARParam *cparam, const float zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings, const int texmapScaleFactor)
{
	float	px, py, py_prev;
    double	x1, x2, y1, y2;
    float	xx1, xx2, yy1, yy2;
	int		i, j;
	
    if(!contextSettings->initedRectangle || (texmapScaleFactor != contextSettings->initedRectangleTexmapScaleFactor)) {
		
		// If it was texmapScaleFactor that changed, delete texture and list first.
		if (contextSettings->initedRectangle) {
			glDeleteTextures(1, &(contextSettings->textureRectangle));
			glDeleteLists(contextSettings->listRectangle, 1);
		}
		
		// Check texturing capabilities.
		if (!contextSettings->textureRectangleCapabilitiesChecked) {
			if (!arglDispImageTexRectangleCapabilitiesCheck(cparam, contextSettings)) {
				fprintf(stderr, "argl error: Your OpenGL implementation and/or hardware's texturing capabilities are insufficient.\n");
				exit (-1);
			}
		}
		
		// Set up the rectangle texture object.
		glGenTextures(1, &(contextSettings->textureRectangle));
		glBindTexture(GL_TEXTURE_RECTANGLE, contextSettings->textureRectangle);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
		if (arglAppleTextureRange) {
			glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, cparam->xsize * cparam->ysize * AR_PIX_SIZE, image);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, arglAppleTextureRangeStorageHint);
		} else {
			glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, 0, NULL);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
		}
#endif // ARGL_USE_TEXTURE_RANGE
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, arglAppleClientStorage);
#endif // APPLE_TEXTURE_FAST_TRANSFER
		
		// Specify the texture to OpenGL.
		if (texmapScaleFactor == 2) {
			// If arglTexmapMode is non-zero, pretend lines in the source image are
			// twice as long as they are, so that glTexImage2D will read only the first
			// half of each line, effectively discarding every second line in the source image.
			glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Our image data is tightly packed.
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, AR_PIX_INTFORMAT, cparam->xsize, cparam->ysize/texmapScaleFactor, 0, AR_PIX_FORMAT, AR_PIX_TYPE, image);
		if (texmapScaleFactor == 2) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
		
		// Set up the surface which we will texture upon.
		contextSettings->listRectangle = glGenLists(1);
		glNewList(contextSettings->listRectangle, GL_COMPILE);
		glEnable(GL_TEXTURE_RECTANGLE);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		
		py_prev = 0.0f;
		for(j = 1; j <= 20; j++) {	// Do 20 rows.
			py = py_prev;
			py_prev = cparam->ysize * j / 20.0f;

			glBegin(GL_QUAD_STRIP);
			for(i = 0; i <= 20; i++) {	// Draw 21 pairs of vertices per row to make 20 columns.
				px = cparam->xsize * i / 20.0f;

				arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py, &x1, &y1);
				arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py_prev, &x2, &y2);

				xx1 = (float)x1 * zoom;
				yy1 = (cparam->ysize - (float)y1) * zoom;
				xx2 = (float)x2 * zoom;
				yy2 = (cparam->ysize - (float)y2) * zoom;

				glTexCoord2f(px, py/texmapScaleFactor); glVertex2f(xx1, yy1);
				glTexCoord2f(px, py_prev/texmapScaleFactor); glVertex2f(xx2, yy2);
            }
			glEnd();
		}
		glDisable(GL_TEXTURE_RECTANGLE);
		glEndList();

		contextSettings->initedRectangleTexmapScaleFactor = texmapScaleFactor;
        contextSettings->initedRectangle = TRUE;
    }
	
    glBindTexture(GL_TEXTURE_RECTANGLE, contextSettings->textureRectangle);
#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
	if (arglAppleTextureRange) {
		glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, cparam->xsize * cparam->ysize * AR_PIX_SIZE, image);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, arglAppleTextureRangeStorageHint);
	} else {
		glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, 0, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
	}
#endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, arglAppleClientStorage);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, cparam->xsize, cparam->ysize/texmapScaleFactor, AR_PIX_FORMAT, AR_PIX_TYPE, image);
	glCallList(contextSettings->listRectangle);
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);	
}
#endif // AR_OPENGL_TEXTURE_RECTANGLE

static int arglDispImageTexPow2CapabilitiesCheck(const ARParam *cparam, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint format;
	GLint texture1SizeMax;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture1SizeMax);
	if (cparam->xsize > texture1SizeMax || cparam->ysize > texture1SizeMax) {
		return (FALSE);
	}
	
	// Work out how big textures needs to be.
	contextSettings->texturePow2SizeX = contextSettings->texturePow2SizeY = 1;
	while (contextSettings->texturePow2SizeX < cparam->xsize) {
		contextSettings->texturePow2SizeX *= 2;
		if (contextSettings->texturePow2SizeX > texture1SizeMax) {
			return (FALSE); // Too big to handle.
		}
	}
	while (contextSettings->texturePow2SizeY < cparam->ysize) {
		contextSettings->texturePow2SizeY *= 2;
		if (contextSettings->texturePow2SizeY > texture1SizeMax) {
			return (FALSE); // Too big to handle.
		}
	}
	
	// Now check that the renderer can accomodate a texture of this size.
#ifdef APPLE_TEXTURE_FAST_TRANSFER
	// Can't use client storage or texture range.
#  ifdef ARGL_USE_TEXTURE_RANGE
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#  endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, AR_PIX_INTFORMAT, contextSettings->texturePow2SizeX, contextSettings->texturePow2SizeY, 0, AR_PIX_FORMAT, AR_PIX_TYPE, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	if (!format) {
		return (FALSE);
	}
	
	// Decide whether we can use GL_CLAMP_TO_EDGE.
	if (arglGLCapabilityCheck(0x0120, "GL_SGIS_texture_edge_clamp")) {
		contextSettings->texturePow2WrapMode = GL_CLAMP_TO_EDGE;
	} else {
		contextSettings->texturePow2WrapMode = GL_REPEAT;
	}
	
	contextSettings->texturePow2CapabilitiesChecked = TRUE;
	
	return (TRUE);
}

//
// Blit an image to the screen using OpenGL power-of-two texturing.
// TODO: a cleanup routine to delete texture name, display list.
//
static void arglDispImageTexPow2(ARUint8 *image, const ARParam *cparam, const float zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings, const int texmapScaleFactor)
{
    float	tsx, tsy, tex, tey;
    float	px, py, qx, qy;
    double	x1, x2, x3, x4, y1, y2, y3, y4;
    float	xx1, xx2, xx3, xx4, yy1, yy2, yy3, yy4;
    int		i, j;

    if(!contextSettings->initedPow2 || (texmapScaleFactor != contextSettings->initedPow2TexmapScaleFactor)) {

		// If it was texmapScaleFactor that changed, delete texture and list first.
		if (contextSettings->initedPow2) {
			glDeleteTextures(1, &(contextSettings->texturePow2));
			glDeleteLists(contextSettings->listPow2, 1);
		}

		// Check texturing capabilities.
		if (!contextSettings->texturePow2CapabilitiesChecked) {
			if (!arglDispImageTexPow2CapabilitiesCheck(cparam, contextSettings)) {
				fprintf(stderr, "argl error: Your OpenGL implementation and/or hardware's texturing capabilities are insufficient.\n");
				exit (-1);
			}
		}

		// Set up the texture object.
		glGenTextures(1, &(contextSettings->texturePow2));
		glBindTexture(GL_TEXTURE_2D, contextSettings->texturePow2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, contextSettings->texturePow2WrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, contextSettings->texturePow2WrapMode);

#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
		// Can't use client storage or texture range.
		glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#  endif // ARGL_USE_TEXTURE_RANGE
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER

		// Request OpenGL allocate memory for a power-of-two texture of the appropriate size.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, AR_PIX_INTFORMAT, contextSettings->texturePow2SizeX, contextSettings->texturePow2SizeY, 0, AR_PIX_FORMAT, AR_PIX_TYPE, NULL);
				
		// Set up the surface which we will texture upon.
		contextSettings->listPow2 = glGenLists(1);
		glNewList(contextSettings->listPow2, GL_COMPILE); // NB Texture not specified yet so don't execute.
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

        qy = 0.0f;
        tey = 0.0f;
        for(j = 1; j <= 20; j++) {	// Do 20 rows.
            py = qy;
            tsy = tey;
            qy = cparam->ysize * j / 20.0f;
            tey = qy / contextSettings->texturePow2SizeY;

            qx = 0.0f;
            tex = 0.0f;
            for(i = 1; i <= 20; i++) {	// Draw 20 columns.
                px = qx;
                tsx = tex;
                qx = cparam->xsize * i / 20.0f;
                tex = qx / contextSettings->texturePow2SizeX;

                arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py, &x1, &y1);
                arParamObserv2Ideal(cparam->dist_factor, (double)qx, (double)py, &x2, &y2);
                arParamObserv2Ideal(cparam->dist_factor, (double)qx, (double)qy, &x3, &y3);
                arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)qy, &x4, &y4);

				xx1 = (float)x1 * zoom;
				yy1 = (cparam->ysize - (float)y1) * zoom;
				xx2 = (float)x2 * zoom;
				yy2 = (cparam->ysize - (float)y2) * zoom;
				xx3 = (float)x3 * zoom;
				yy3 = (cparam->ysize - (float)y3) * zoom;
				xx4 = (float)x4 * zoom;
				yy4 = (cparam->ysize - (float)y4) * zoom;

                glBegin(GL_QUADS);
                glTexCoord2f(tsx, tsy); glVertex2f(xx1, yy1);
                glTexCoord2f(tex, tsy); glVertex2f(xx2, yy2);
                glTexCoord2f(tex, tey); glVertex2f(xx3, yy3);
                glTexCoord2f(tsx, tey); glVertex2f(xx4, yy4);
                glEnd();
            } // columns.
        } // rows.
		glDisable(GL_TEXTURE_2D);
        glEndList();

        contextSettings->initedPow2TexmapScaleFactor = texmapScaleFactor;
		contextSettings->initedPow2 = TRUE;
	}

    glBindTexture(GL_TEXTURE_2D, contextSettings->texturePow2);
#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
	// Can't use client storage or texture range.
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	if (texmapScaleFactor == 2) {
		// If arglTexmapMode is non-zero, pretend lines in the source image are
		// twice as long as they are, so that glTexImage2D will read only the first
		// half of each line, effectively discarding every second line in the source image.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cparam->xsize, cparam->ysize/texmapScaleFactor, AR_PIX_FORMAT, AR_PIX_TYPE, image);
	glCallList(contextSettings->listPow2);
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
    glBindTexture(GL_TEXTURE_2D, 0);
}

#pragma mark -
// ============================================================================
//	Public functions.
// ============================================================================

ARGL_CONTEXT_SETTINGS_REF arglSetupForCurrentContext(void)
{
	ARGL_CONTEXT_SETTINGS_REF contextSettings;
	
	contextSettings = (ARGL_CONTEXT_SETTINGS_REF)calloc(1, sizeof(ARGL_CONTEXT_SETTINGS));
	return (contextSettings);
}

void arglCleanup(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	free(contextSettings);
}

void arglCameraFrustum(const ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16])
{
    int		i;
	ARParam	cparam_copy;

    cparam_copy = *cparam;
	for (i = 0; i < 4; i++) {
        cparam_copy.mat[1][i] = (cparam_copy.ysize - 1)*(cparam_copy.mat[2][i]) - cparam_copy.mat[1][i];
    }
    arglConvGLcpara(&cparam_copy, focalmin, focalmax, m_projection);
}

void arglCameraView(double para[3][4], GLdouble m_modelview[16], double scale)
{
    int     i, j;

    for(j = 0; j < 3; j++) {
        for(i = 0; i < 4; i++) {
            m_modelview[i*4+j] = para[j][i];
        }
    }
    m_modelview[0*4+3] = m_modelview[1*4+3] = m_modelview[2*4+3] = 0.0;
    m_modelview[3*4+3] = 1.0;
	if (scale != 0.0) {
		m_modelview[12] *= scale;
		m_modelview[13] *= scale;
		m_modelview[14] *= scale;
	}
}

void arglDispImage(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint texEnvModeSave;	
	GLboolean lightingSave;
	GLboolean depthTestSave;
#ifdef ARGL_DEBUG
	GLenum			err;
	const GLubyte	*errs;
#endif // ARGL_DEBUG

	if (!image) return;

	// Prepare an orthographic projection, set camera position for 2D drawing, and save GL state.
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texEnvModeSave); // Save GL texture environment mode.
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGetBooleanv(GL_LIGHTING, &lightingSave);			// Save enabled state of lighting.
	if (lightingSave) glDisable(GL_LIGHTING);
	glGetBooleanv(GL_DEPTH_TEST, &depthTestSave);		// Save enabled state of depth test.
	if (depthTestSave) glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, cparam->xsize, 0, cparam->ysize);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();		
	
	arglDispImageStateful(image, cparam, zoom, contextSettings);

	// Restore previous projection, camera position, and GL state.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	if (depthTestSave) glEnable(GL_DEPTH_TEST);			// Restore enabled state of depth test.
	if (lightingSave) glEnable(GL_LIGHTING);			// Restore enabled state of lighting.
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvModeSave); // Restore GL texture environment mode.
	
#ifdef ARGL_DEBUG
	// Report any errors we generated.
	while ((err = glGetError()) != GL_NO_ERROR) {
		errs = gluErrorString(err);	// fetch error code
		fprintf(stderr, "GL error: %s (%i)\n", errs, (int)err);	// write err code and number to stderr
	}
#endif // ARGL_DEBUG
	
}

void arglDispImageStateful(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	float zoomf;
	
	zoomf = (float)zoom;
	if (arglDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS) {
		glDisable(GL_TEXTURE_2D);
		glPixelZoom(zoomf, -zoomf);
		glRasterPos2f(0.0f, cparam->ysize * zoomf);
		glDrawPixels(cparam->xsize, cparam->ysize, AR_PIX_FORMAT, AR_PIX_TYPE, image);
	} else {
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
		if (!arglTexRectangle) {
#endif // AR_OPENGL_TEXTURE_RECTANGLE
			arglDispImageTexPow2(image, cparam, zoomf, contextSettings, arglTexmapMode + 1);
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
		} else {
			arglDispImageTexRectangle(image, cparam, zoomf, contextSettings, arglTexmapMode + 1);
		}
#endif // AR_OPENGL_TEXTURE_RECTANGLE
	}	
}

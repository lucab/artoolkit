/*
 *
 * Author: Hirokazu Kato, Atsishi Nakazawa.
 *         Modified by Thomas Pintaric.
 *          
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *         nakazawa@inolab.sys.es.osaka-u.ac.jp
 *         pintaric@ims.tuwien.ac.at
 *
 * Revision: 4.3b
 * Date: 2003/02/02
 *
 */
/*
 *	Copyright (c) 2004-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *	2.6.8	2004-07-20	PRL		Rewrite for ARToolKit 2.68.2
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

#ifndef AR_VIDEO_H
#define AR_VIDEO_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <AR/config.h>
#include <AR/ar.h>

#ifdef _WIN32
#  include <AR/sys/videoWin32DirectShow.h>
#  ifdef LIBARVIDEO_EXPORTS
#    define AR_DLL_API __declspec(dllexport)
#  else
#    ifdef _DLL
#      define AR_DLL_API __declspec(dllimport)
#    else
#      define AR_DLL_API extern
#    endif
#  endif
#else
#  define AR_DLL_API
#endif

#ifdef __linux
#  ifdef AR_INPUT_V4L
#    include <AR/sys/videoLinuxV4L.h>
#  endif
#  ifdef  AR_INPUT_DV
#    include <AR/sys/videoLinuxDV.h>
#  endif
#  ifdef  AR_INPUT_1394CAM
#    include <AR/sys/videoLinux1394Cam.h>
#  endif
#endif

#ifdef __sgi
#  include <AR/sys/videoSGI.h>
#endif

#ifdef __APPLE__
#  include <AR/sys/videoMacOSX.h>
#endif

AR_DLL_API  int				arVideoDispOption(void);
AR_DLL_API  int				arVideoOpen(char *config);
AR_DLL_API  int				arVideoClose(void);
AR_DLL_API  int				arVideoCapStart(void);
AR_DLL_API  int				arVideoCapStop(void);
AR_DLL_API  int				arVideoCapNext(void);
AR_DLL_API  ARUint8*		arVideoGetImage(void);
AR_DLL_API  int				arVideoInqSize(int *x, int *y);

AR_DLL_API  int				ar2VideoDispOption(void);
AR_DLL_API  AR2VideoParamT  *ar2VideoOpen(char *config);
AR_DLL_API  int				ar2VideoClose(AR2VideoParamT *vid);
AR_DLL_API  int				ar2VideoCapStart(AR2VideoParamT *vid);
AR_DLL_API  int				ar2VideoCapNext(AR2VideoParamT *vid);
AR_DLL_API  int				ar2VideoCapStop(AR2VideoParamT *vid);
AR_DLL_API  ARUint8			*ar2VideoGetImage(AR2VideoParamT *vid);
AR_DLL_API  int				ar2VideoInqSize(AR2VideoParamT *vid, int *x, int *y);

// Functions added for Studierstube/OpenTracker.
#ifdef _WIN32
#  ifndef __MEMORY_BUFFER_HANDLE__
#  define __MEMORY_BUFFER_HANDLE__
#  define DEFAULT_NUMBER_OF_ALLOCATOR_BUFFERS 3
typedef struct _MemoryBufferHandle
{
	unsigned long  n; // sample number
	__int64 t;		  // timestamp
} MemoryBufferHandle;
#  endif // __MEMORY_BUFFER_HANDLE__
AR_DLL_API  int				ar2VideoInqFreq(AR2VideoParamT *vid, float *fps);
AR_DLL_API  int				ar2VideoInqFlipping(AR2VideoParamT *vid, int *flipH, int *flipV);
AR_DLL_API  unsigned char	*ar2VideoLockBuffer(AR2VideoParamT *vid, MemoryBufferHandle *pHandle);
AR_DLL_API  int				ar2VideoUnlockBuffer(AR2VideoParamT *vid, MemoryBufferHandle Handle);
#endif // _WIN32

#ifdef  __cplusplus
}
#endif

#endif // AR_VIDEO_H

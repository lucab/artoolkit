/*  --------------------------------------------------------------------------
 *	Copyright (c) 2004-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
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
 *   -------------------------------------------------------------------------*/
/**
*  \file video.h
*  \brief ARToolkit video subroutines.
*
*  This component furnishs a multi-platform video support for ARToolkit. 
*  It provides access to hardware video input available on different machines, OS. 
*  For each platform a specific sub-component have been implemented, unified in the
*  same interface (with the prefix function name arVideoXXX).
*
*  The actual supported platforms (and the driver/library used) are:
*  - Windows: with DirectShow (VFW obsolete).
*  - Linux: with V4L, IEEE1394 Library.
*  - Macintosh: with QuickTime 6.
*  - SGI: with VL.
*
*  This component have two separated interfaces according to your videos input
*  number:
*  - one single alone camera: used the <b>arXXX</b> functions (XXX template name).
*  - multiple camera: used the <b>ar2XXX</b> functions (XXX template name).
*
*  More information are available on <a href="video.html">the video page</a>.
*
*  \remark 
* the arXXX uses a global variable and wrap all calls to ar2XXX functions
* - WINDOWS: only one camera is supported.
*
*  History :
*		modified by Thomas Pintaric (pintaric@ims.tuwien.ac.at) for adding
*       a fully transparent DirectShow Driver.
*
*  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp
*  \author Atsishi Nakazawa nakazawa@inolab.sys.es.osaka-u.ac.jp
*  \author Thomas Pintaric pintaric@ims.tuwien.ac.at
*  \version 4.3b
*  \date 03/02/02
*
*
**/
/*  --------------------------------------------------------------------------
*   History : 
*   Rev		Date		Who		Changes
 *	2.6.8	2004-07-20	PRL		Rewrite for ARToolKit 2.68.2
*
*----------------------------------------------------------------------------*/

#ifndef AR_VIDEO_H
#define AR_VIDEO_H

#ifdef  __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <AR/config.h>
#include <AR/ar.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

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

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/*
mono-camera
*/

/** \fn int arVideoDispOption( void )
* \brief display the video option.
*
* According to your platform, output in the
* standard console the video option available.
* \return 0
*/
AR_DLL_API  int				arVideoDispOption(void);

/** \fn int arVideoOpen( char *config )
* \brief open a video source.
* 
* this function open a video input path with the
* driver (and device) present on your platform. 
* According to your operating system and the
* hardware the initialization will be different : a
* generic string structure is used for this issue.
* You can find all details in this <a href="videoconfig.html">
* webpage</a>
* \param config string of the selected video configuration.
* \return 0 if successful, -1 if a video path couldn’t be opened
*/	
AR_DLL_API  int				arVideoOpen(char *config);

/** \fn int arVideoClose( void )
* \brief close the video source.
* a function that needs to be called in order to shut down the video capture
* \return 0 if shut down successfully, otherwise –1.
*/
AR_DLL_API  int				arVideoClose(void);

/** \fn int arVideoCapStart( void )
* \brief start the capture of video.
*
* This function start the video capture routine. According
* to the architecture can start a video thread, call a specific
* run-time function in the driver.
* \remark this function coupled with arVideoCapStop, can
* be call many times in your program (reduce the CPU load
* when they stop for long and critical operations)
* \return 0 if successful, -1 if the capture can't be start
*/
AR_DLL_API  int				arVideoCapStart(void);

/** \fn int arVideoCapStop( void )
* \brief stop the capture of video.
*
* This function stop the video capture routine. According
* to the architecture can stop a video thread, call a specific
* run-time function in the driver.
* \remark this function coupled with arVideoCapStart, can
* be call many times in your program (reduce the CPU load
* when they stop for long and critical operations)
* \return 0 if successful, -1 if the capture can't be stop
*/
AR_DLL_API  int				arVideoCapStop(void);

/** \fn int arVideoCapNext( void )
* \brief call for the next grabbed video frame.
*
* According to your platform, this function can
* be have different effects. A call to the function
* indicates that a new frame can be grabbed. The outcome
* is you can't use more the last video frame. You need to
* call this function after analysis the video image and
* display it.
* 
* \remark if your video grabber is multithread, this
* function do nothing since a buffer is still available.
* \return 0 if successful, -1 if the next grabbed frame can't be start
*/
AR_DLL_API  int				arVideoCapNext(void);

/** \fn ARUint8 *arVideoGetImage( void )
* \brief get the video image.
*
* This function return a buffer with a captured video image.
* XXXBK : do we need to delete the pointed area ? 
*         is the area still available in the next call ?
* \return a pointer to the captured video frame, or NULL if a frame isn’t captured.
*/
AR_DLL_API  ARUint8*		arVideoGetImage(void);

/** \fn int arVideoInqSize( int *x, int *y )
* \brief get the video image size.
*
* a function that returns the size of the captured video frame.
* \param x a pointer to the length of the captured image
* \param y a pointer to the width of the captured image
* \return 0 if the dimensions are found successfully, otherwise –1
*/
AR_DLL_API  int				arVideoInqSize(int *x, int *y);

/*
multiple cameras
*/

/** \fn int ar2VideoDispOption ( void )
* \brief display the video option (multiple video inputs)
*
* idem of arVideoDispOption for multiple video sources.
* \return 0
*/
AR_DLL_API  int				ar2VideoDispOption(void);

/** \fn AR2VideoParamT  *ar2VideoOpen ( char *config )
* \brief open a video source (multiple video inputs)
*
* idem of arVideoOpen for multiple video sources (the selected
* source in argument).
* \param config string of the selected video configuration.
* \return 0 if successful, -1 if a video path couldn’t be opened
*/
AR_DLL_API  AR2VideoParamT  *ar2VideoOpen(char *config);

/** \fn int ar2VideoClose ( AR2VideoParamT *vid )
* \brief close a video source (multiple video inputs)
*
* idem of arVideoClose for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \return 0 if shut down successfully, otherwise –1.
*/
AR_DLL_API  int				ar2VideoClose(AR2VideoParamT *vid);

/** \fn int ar2VideoCapStart (AR2VideoParamT *vid )
* \brief start the capture of a video source (multiple video inputs)
*
* idem of arVideoCapStart for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \return 0 if successful, -1 if the capture can't be start
*/
AR_DLL_API  int				ar2VideoCapStart(AR2VideoParamT *vid);

/** \fn int ar2VideoCapNext ( AR2VideoParamT *vid )
* \brief call for the next grabbed video frame of a video source (multiple video inputs)
*
* idem of arVideoCapNext for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \return 0 if successful, -1 if the next grabbed frame can't be start
*/
AR_DLL_API  int				ar2VideoCapNext(AR2VideoParamT *vid);

/** \fn int ar2VideoCapStop ( AR2VideoParamT *vid )
* \brief stop the capture of a video source (multiple video inputs)
*
* idem of arVideoCapStop for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \return 0 if successful, -1 if the next grabbed frame can't be stop
*/
AR_DLL_API  int				ar2VideoCapStop(AR2VideoParamT *vid);

/** \fn int ar2VideoGetImage ( AR2VideoParamT *vid )
* \brief get a video image from a video source (multiple video inputs)
*
* idem of arVideoGetImage for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \return a pointer to the captured video frame, or NULL if a frame isn’t captured.
*/
AR_DLL_API  ARUint8			*ar2VideoGetImage(AR2VideoParamT *vid);

/** \fn int ar2VideoInqSize ( AR2VideoParamT *vid, int *x,int *y )
* \brief get the video image size of a video source (multiple video inputs)
*
* idem of arVideoInqSize for multiple video sources (the selected
* source in argument).
* \param vid a video handle structure for multi-camera grabbing
* \param x a pointer to the length of the captured image
* \param y a pointer to the width of the captured image
* \return 0 if the dimensions are found successfully, otherwise –1
*/
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

 /* ========================================================================
  * PROJECT: DirectShow Video Processing Library
  * FILE:    ARToolkit 2.6x WIN32/DirectShow video input
  * ========================================================================
  * Author:  Thomas Pintaric, Vienna University of Technology
  * Contact: pintaric@ims.tuwien.ac.at http://ims.tuwien.ac.at/~thomas
  * =======================================================================
  *
  * Copyright (C) 2003  Vienna University of Technology
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  * For further information please contact Thomas Pintaric under
  * <pintaric@ims.tuwien.ac.at> or write to Thomas Pintaric,
  * Vienna University of Technology, Favoritenstr. 9-11/188, A1040 Vienna,
  * Austria.
  * ========================================================================*/
/*
 *	Copyright (c) 2004-2004 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *	2.6.8	2004-07-20	PRL		Rewrite for ARToolKit 2.68.2
 *
 */

#include "DsVideoLib.h"
#include <string.h>
#include <AR/video.h>
#include <stdlib.h>
#include "comutil.h"

// -----------------------------------------------------------------------------------------------------------------

struct _AR2VideoParamT {
	CGraphManager		*graphManager;
	MemoryBufferHandle  g_Handle;
	__int64				g_Timestamp; // deprecated, use (g_Handle.t) instead.
	//bool flip_horizontal = false; // deprecated.
	//bool flip_vertical = false;   // deprecated.
};

// -----------------------------------------------------------------------------------------------------------------

static AR2VideoParamT   *gVid = NULL;

#ifdef FLIPPED // compatibility with videoLinux*
static const bool		FLIPPED_defined =  true;		// deprecated
#else
static const bool		FLIPPED_defined =  false;		// deprecated
#endif
const long				frame_timeout_ms = INFINITE;	// set to INFINITE if arVideoGetImage()
														// is called from a separate worker thread

// -----------------------------------------------------------------------------------------------------------------

int arVideoDispOption(void)
{
    return (ar2VideoDispOption());
}

int arVideoOpen(char *config)
{
    if (gVid != NULL) {
        fprintf(stderr, "arVideoOpen(): Error, device is already open.\n");
        return (-1);
    }
    gVid = ar2VideoOpen(config);
    if (gVid == NULL) return (-1);
	
    return (0);
}

int arVideoClose(void)
{
	int result;
	
    if (gVid == NULL) return (-1);
	
	result = ar2VideoClose(gVid);
	gVid = NULL;
    return (result);
}  

int arVideoInqSize(int *x, int *y)
{
    if (gVid == NULL) return (-1);
	
    return (ar2VideoInqSize(gVid, x, y));
}       

ARUint8 *arVideoGetImage(void)
{   
    if (gVid == NULL) return (NULL);
	
    return (ar2VideoGetImage(gVid));
}

int arVideoCapStart(void)
{
    if (gVid == NULL) return (-1);
	
    return (ar2VideoCapStart(gVid));
}  

int arVideoCapStop(void)
{
    if (gVid == NULL) return (-1);
	
    return (ar2VideoCapStop(gVid));
}       

int arVideoCapNext(void)
{   
    if (gVid == NULL) return (-1);  
	
    return (ar2VideoCapNext(gVid)); 
}

// -----------------------------------------------------------------------------------------------------------------

int ar2VideoDispOption(void)
{
	
	printf("parameter format is either NULL or a list of tokens, separated by commas ","\n");
	printf("\n");
	printf("BINARY TOKENS:\n");
	printf("--------------\n");
	printf("flipH     : flip image horizontally (WARNING: non-optimal performance)\n");
	printf("flipV     : flip image vertically (WARNING: non-optimal performance)\n");
	printf("showDlg   : displays either WDM capture filter's property page or\n");
	printf("            MSDV decoder format dialog (depending on source media type).\n");
	printf("            only applies to WDM_CAP, will be ignored for AVI_FILE\n");
	printf("loopAVI   : continuously loops through an AVI file (applies only to AVI_FILE)\n");
	printf("noClock   : does not use a Reference Clock to synchronize samples;\n");
	printf("            use this flag for offline post-processing (applies only to AVI_FILE)\n");
	printf("renderSec : render secondary streams (applies only to AVI_FILE)\n");
	printf("            An AVI file can contain an arbitrary number of multiplexed A/V substreams,\n");
	printf("            usually there are at most 2 substreams (1st: video, 2nd: audio).\n");
	printf("            the AVI_FILE input module will only try to render stream 0x00 (assuming that\n");
	printf("            it's video) and ignore the remaning substreams.\n");
	printf("            Use this flag to force IGraphBuilder->Render(IPin*) calls on substreams 1..n\n");
	printf("            DO NOT SET this flag if your AVI file contains more than one video stream\n");
	printf("\n");
	printf("PARAMETRIZED TOKENS:\n");
	printf("--------------------\n");
	printf("inputDevice=? : supported parameters: \n");
	printf("                \"WDM_CAP\"  (WDM_VIDEO_CAPTURE_FILTER) use the DirectShow WDM wrapper\n");
	printf("                           to obtain live video input from a streaming capture device\n");
	printf("                           such as a IEEE 1394 DV camera or USB webcam.\n");
	printf("                           OPTIONAL: set deviceName=? and/or ieee1394id=? for better\n");
	printf("                                     control over the choice of suitable WDM drivers\n");
	printf("                \"AVI_FILE\" (ASYNC_FILE_INPUT_FILTER) use an Asynchronous File Input\n");
	printf("                           Filter to read & decode AVI video data\n");
	printf("                           NOTE: be sure to specify the input file name by pointing\n");
	printf("                                 fileName=? to a valid AVI file.\n");
	printf("\n");
	printf("                EXAMPLE: \"inputDevive=WDM_CAP\", or \"inputDevice=AVI_FILE\"\n");
	printf("                DEFAULT: \"WDM_CAP\" will be selected if you omit this token\n");
	printf("\n");
	printf("videoWidth=?  : preferred video width, EXAMPLE: \"videoWidth=720\"\n");
	printf("                only applies to WDM_CAP, will be ignored for AVI_FILE\n");
	printf("videoHeight=? : preferred video height, EXAMPLE: \"videoHeight=576\"\n");
	printf("                only applies to WDM_CAP, will be ignored for AVI_FILE\n");
	printf("\n");
	printf("pixelFormat=? : internal pixel format (see PixelFormat.h for supported types)\n");
	printf("                PERFORMANCE WARNING: Be sure to match your IDirect3DTexture/OpenGL texture\n");
	printf("                                     formats to whatever you specify here, i.e. use\n");
	printf("                                     PXtoOpenGL(format), PXtoD3D(format) for creating your\n");
	printf("                                     textures! (see PixelFormat.h for details)\n");
	printf("                EXAMPLE: \"pixelFormat=PIXELFORMAT_RGB32\"\n");
	printf("                NOTE: if you omit the pixelFormat=? token, the global\n");
	printf("                constant (default_PIXELFORMAT, usually PIXELFORMAT_RGB32)\n");
	printf("                will be selected.\n");
	printf("\n");
	printf("friendlyName=? : only applies to WDM_CAP, will be IGNORED if \"inputDevice=WDM_CAP\" is not set.\n");
	printf("                Used to select a preferred WDM device. WILL BE IGNORED IF deviceName=? IS SET.");
	printf("                (WARNING: WDM \"friendly  names\" are locale-dependent), i.e. try to match substring <?>\n");
	printf("                with  the \"friendly names\" of enumerated DirectShow  WDM wrappers (ksproxy.ax).\n");
	printf("                EXAMPLE: \"friendlyName=Microsoft DV Camera\" for IEEE1394 DV devices\n");
	printf("                         \"friendlyName=QuickCam\" for Logitech QuickCam\n");
	printf("\n");
	printf("deviceName=? :  only applies to WDM_CAP, will be IGNORED if \"inputDevice=WDM_CAP\" is not set.\n");
	printf("                Used to select a preferred WDM device.  WILL ALWAYS WILL OVERRIDE friendlyName=?\n");
	printf("                i.e. try to match substring <?> with the \"device  names\" of enumerated DirectShow\n");
	printf("                WDM wrappers (ksproxy.ax).\n");
	printf("                Device names look like: \"@device:*:{860BB310-5D01-11d0-BD3B-00A0C911CE86}\n");
	printf("                Use GraphEdit (part  of the DirectX SDK, under \\DXSDK\\bin\\DxUtils\\graphedt.exe)\n");
	printf("                to figure out your  camera's device name.\n");
	printf("                EXAMPLE: \"deviceName=1394#unibrain&fire-i_1.2#4510000061431408\n");
	printf("\n");
	printf("fileName=?    : only applies to AVI_FILE, will be IGNORED if \"inputDevice=AVI_FILE\" is not set.\n");
	printf("                input file name, if just use a file's name (without its full path), the WIN32\n");
	printf("                API fuction SearchPath() (Winbase.h) will be used to locate the file.\n");
	printf("                EXAMPLE: \"fileName=C:\\Some Directory\\Another Directory\\Video.AVI\"\n");
	printf("                         \"fileName=video.AVI\" (will succeed if C:\\Some Directory\\Another Directory\\\n");
	printf("                                               is:  * the application's startup directory\n");
	printf("                                                    * the current directory\n");
	printf("                                                    * listed in the PATH environment variable)\n");
	printf("\n");
	printf("ieee1394id=?  : only applies to WDM_CAP, will be IGNORED if \"inputDevice=WDM_CAP\" is not set.\n");
	printf("                Unique 64-bit device identifier, as defined by IEEE 1394.\n");
	printf("                Hexadecimal value expected, i.e. \"ieee1394id=437d3b0201460008\"\n");
	printf("                Use /bin/IEEE394_id.exe to determine your camera's ID.\n");
	printf("\n");
	printf("deinterlaceState=?  : supported parameters (see VFX_STATE_names[])\n");
	printf("                        \"off\"  : disable deinterlacing (DEFAULT)\n");
	printf("                        \"on\"   : force deinterlacing (even for progressive frames) \n");
	printf("                        \"auto\" : enable deinterlacing only if \n");
	printf("                                 (VIDEOINFOHEADER.dwInterlaceFlags & AMINTERLACE_IsInterlaced)\n");
	printf("                      WARNING: EXPERIMENTAL FEATURE!\n");
	printf("\n");
	printf("deinterlaceMethod=? : deinterlacing method (see VFxDeinterlaceParam.h for supported modes)\n");
	printf("                      supported parameters (see DEINTERLACE_METHOD_names[]):\n");
	printf("                        \"blend\"      : blend fields (best quality)\n");
	printf("                        \"duplicate1\" : duplicate first field\n");
	printf("                        \"duplicate2\" : duplicate second field\n");
	printf("                      NOTE: omitting this token results in default mode (DEINTERLACE_BLEND) being used.\n");
	printf("                      WARNING: EXPERIMENTAL FEATURE!\n");
	printf("\n");
	printf("EXAMPLES:\n");
	printf("\n");
	printf("arVideoOpen(NULL);\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,showDlg\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,flipH,flipV,showDlg\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,pixelFormat=PIXELFORMAT_RGB24,showDlg\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,showDlg,deinterlaceState=on,deinterlaceMethod=duplicate1\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,videoWidth=640,flipH,videoHeight=480,showDlg,deinterlaceState=auto\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,friendlyName=Microsoft DV Camera,videoWidth=720,videoHeight=480\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,friendlyName=Logitech,videoWidth=320,videoHeight=240,flipV\");\n");
	printf("arVideoOpen(\"inputDevice=WDM_CAP,friendlyName=Microsoft DV Camera,ieee1394id=437d3b0201460008\");\n");
	printf("arVideoOpen(\"inputDevice=AVI_FILE,fileName=C:\\Some Directory\\Another Directory\\Video.AVI\");\n");
	printf("arVideoOpen(\"inputDevice=AVI_FILE,fileName=Video.AVI,pixelFormat=PIXELFORMAT_RGB24\");\n");
	printf("\n");
	
    return (0);
}

AR2VideoParamT *ar2VideoOpen(char *config)
{
	AR2VideoParamT *vid = NULL;
	DS_MEDIA_FORMAT mf;
	
	// Allocate the parameters structure and fill it in.
	arMalloc(vid, AR2VideoParamT, 1);
	memset(vid, 0, sizeof(AR2VideoParamT));

	CoInitialize(NULL);
	
	vid->graphManager = new CGraphManager();
	
	if(FAILED(vid->graphManager->BuildGraph(config))) return(NULL);
	
	vid->graphManager->GetCurrentMediaFormat(&mf);
	//vid->flip_horizontal = mf.flipH;
	//vid->flip_vertical =   mf.flipV;

	if(FAILED(vid->graphManager->EnableMemoryBuffer())) return(NULL);

	return (vid);
}


int ar2VideoClose(AR2VideoParamT *vid)
{
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return (-1);
	
	vid->graphManager->CheckinMemoryBuffer(vid->g_Handle, true);
	vid->graphManager->Stop();
	delete vid->graphManager;
	vid->graphManager = NULL;
	free (vid);
	
    return(0);
}

unsigned char *ar2VideoGetImage(AR2VideoParamT *vid)
{
	DWORD wait_result;
	unsigned char* pixelBuffer;
	
	if (vid == NULL) return (NULL);
	if (vid->graphManager == NULL) return (NULL);
	
	wait_result = vid->graphManager->WaitForNextSample(frame_timeout_ms);
	if(wait_result == WAIT_OBJECT_0) {
		if (FAILED(vid->graphManager->CheckoutMemoryBuffer(&(vid->g_Handle), &pixelBuffer, NULL, NULL, NULL, &(vid->g_Timestamp)))) return(NULL);
		return (pixelBuffer);
	}
	return(NULL);
}

int ar2VideoCapStart(AR2VideoParamT *vid)
{
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return (-1);
	
	if(FAILED(vid->graphManager->Run())) return (-1);
	return (0);
}

int ar2VideoCapStop(AR2VideoParamT *vid)
{
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return (-1);

	vid->graphManager->CheckinMemoryBuffer(vid->g_Handle, true);

	if(FAILED(vid->graphManager->Stop())) return (-1);

	return (0);
}

int ar2VideoCapNext(AR2VideoParamT *vid)
{
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return (-1);

	return (SUCCEEDED(vid->graphManager->CheckinMemoryBuffer(vid->g_Handle)) ? 0 : -1);
}

int ar2VideoInqSize(AR2VideoParamT *vid, int *x, int *y)
{
	DS_MEDIA_FORMAT mf;
	
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return(-1);
	
	vid->graphManager->GetCurrentMediaFormat(&mf);
	*x = (int) mf.biWidth;
	*y = (int) mf.biHeight;
	
    return (0);
}

// -----------------------------------------------------------------------------------------------------------------

int ar2VideoInqFlipping(AR2VideoParamT *vid, int *flipH, int *flipV)
{
	DS_MEDIA_FORMAT mf;
	
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return(-1);
	
	vid->graphManager->GetCurrentMediaFormat(&mf);
	*flipH = (int) mf.flipH;
	*flipV = (int) mf.flipV;
	//vid->flipH = (int) mf.flipH;
	//vid->flipV = (int) mf.flipV;
	
	return (0);
}

int ar2VideoInqFreq(AR2VideoParamT *vid, float *fps)
{
	DS_MEDIA_FORMAT mf;
	
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return(-1);
	
	vid->graphManager->GetCurrentMediaFormat(&mf);
	*fps = (float) mf.frameRate;
	
    return (0);
}

unsigned char *ar2VideoLockBuffer(AR2VideoParamT *vid, MemoryBufferHandle* pHandle)
{
	unsigned char *pixelBuffer;
	
	if (vid == NULL) return (NULL);
	if (vid->graphManager == NULL) return (NULL);
	
	if (FAILED(vid->graphManager->CheckoutMemoryBuffer(pHandle, &pixelBuffer))) return (NULL);
	
	return (pixelBuffer);
}

int ar2VideoUnlockBuffer(AR2VideoParamT *vid, MemoryBufferHandle Handle)
{
	if (vid == NULL) return (-1);
	if (vid->graphManager == NULL) return(-1);
	
	if (FAILED(vid->graphManager->CheckinMemoryBuffer(Handle))) return(-1);
	
	return (0);
}

// -----------------------------------------------------------------------------------------------------------------


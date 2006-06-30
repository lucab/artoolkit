/*
 * Video capture module utilising the GStreamer pipeline for AR Toolkit
 * 
 * (c) Copyrights 2003-2006 Hartmut Seichter
 * 
 * Licensed under the terms of the GPL
 *
 */
#ifndef AR_VIDEO_GSTREAMER_H
#define AR_VIDEO_GSTREAMER_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <AR/config.h>
#include <AR/ar.h>
#include <gst/gst.h>

typedef struct {

	/* GStreamer pipeline */
	GstElement *pipeline;
	
	/* GStreamer identity needed for probing */
	GstElement *probe;
	
	/* size of the image */
	int	width, height;

	/* the actual video buffer */
    ARUint8             *videoBuffer;
    
} AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif

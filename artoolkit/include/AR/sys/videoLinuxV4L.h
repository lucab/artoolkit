/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 4.4
 * Date: 2002/01/01
 *
*******************************************************/
#ifndef AR_VIDEO_LINUX_V4L_H
#define AR_VIDEO_LINUX_V4L_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev.h>

#include <AR/config.h>
#include <AR/ar.h>

typedef struct {
    char                dev[256];
    int                 width;
    int                 height;
    int                 channel;
    int                 mode;
    int                 debug;
    double              contrast;
    double              brightness;
    double              color;

    int                 fd;
    int                 video_cont_num;
    ARUint8             *map;
    struct video_mbuf   vm;
    struct video_mmap   vmm;
} AR2VideoParamT;

#ifdef  __cplusplus
}
#endif
#endif

/* 
 *   Video capture subrutine for Linux/Video4Linux devices 
 *   author: Nakazawa,Atsushi ( nakazawa@inolab.sys.es.osaka-u.ac.jp )
             Hirokazu Kato ( kato@sys.im.hiroshima-cu.ac.jp )
 *
 *   Revision: 5.2   Date: 2000/08/25
 */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <AR/config.h>
#include <AR/ar.h>
#include <AR/video.h>

#define MAXCHANNEL   10

static AR2VideoParamT   *gVid = NULL;

int arVideoDispOption( void )
{
    return  ar2VideoDispOption();
}

int arVideoOpen( char *config )
{
    if( gVid != NULL ) {
        printf("Device has been opened!!\n");
        return -1;
    }
    gVid = ar2VideoOpen( config );
    if( gVid == NULL ) return -1;

    return 0;
}

int arVideoClose( void )
{
	int result;
	
    if( gVid == NULL ) return -1;

	result = ar2VideoClose(gVid);
	gVid = NULL;
    return (result);
}

int arVideoInqSize( int *x, int *y )
{
    if( gVid == NULL ) return -1;

    return ar2VideoInqSize( gVid, x, y );
}

ARUint8 *arVideoGetImage( void )
{
    if( gVid == NULL ) return NULL;

    return ar2VideoGetImage( gVid );
}

int arVideoCapStart( void )
{
    if( gVid == NULL ) return -1;

    return ar2VideoCapStart( gVid );
}

int arVideoCapStop( void )
{
    if( gVid == NULL ) return -1;

    return ar2VideoCapStop( gVid );
}

int arVideoCapNext( void )
{
    if( gVid == NULL ) return -1;

    return ar2VideoCapNext( gVid );
}

/*-------------------------------------------*/

int ar2VideoDispOption( void )
{
    printf("ARVideo may be configured using one or more of the following options,\n");
    printf("separated by a space:\n\n");
    printf(" -width=N\n");
    printf("    specifies expected width of image.\n");
    printf(" -height=N\n");
    printf("    specifies expected height of image.\n");
    printf(" -contrast=N\n");
    printf("    specifies contrast. (0.0 <-> 1.0)\n");
    printf(" -brightness=N\n");
    printf("    specifies brightness. (0.0 <-> 1.0)\n");
    printf(" -color=N\n");
    printf("    specifies color. (0.0 <-> 1.0)\n");
    printf(" -channel=N\n");
    printf("    specifies source channel.\n");
    printf(" -dev=filepath\n");
    printf("    specifies device file.\n");
    printf(" -mode=[PAL|NTSC|SECAM]\n");
    printf("    specifies TV signal mode.\n");
    printf("\n");

    return 0;
}

AR2VideoParamT *ar2VideoOpen( char *config )
{
    AR2VideoParamT            *vid;
    struct video_capability   vd;
    struct video_channel      vc[MAXCHANNEL];
    struct video_picture      vp;
    char                      *a, line[256];
    int                       i;

    arMalloc( vid, AR2VideoParamT, 1 );
    strcpy( vid->dev, DEFAULT_VIDEO_DEVICE );
    vid->width      = DEFAULT_VIDEO_WIDTH;
    vid->height     = DEFAULT_VIDEO_HEIGHT;
    vid->channel    = DEFAULT_VIDEO_CHANNEL;
    vid->mode       = DEFAULT_VIDEO_MODE;
    vid->debug      = 0;
    vid->contrast   = 0.5;
    vid->brightness = 0.5;
    vid->color      = 0.5;

    a = config;
    if( a != NULL) {
        for(;;) {
            while( *a == ' ' || *a == '\t' ) a++;
            if( *a == '\0' ) break;

            if( strncmp( a, "-width=", 7 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[7], "%d", &vid->width ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-height=", 8 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[8], "%d", &vid->height ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-contrast=", 10 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[10], "%lf", &vid->contrast ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-brightness=", 12 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[12], "%lf", &vid->brightness ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-color=", 7 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[7], "%lf", &vid->color ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-channel=", 9 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[9], "%d", &vid->channel ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-dev=", 5 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[5], "%s", vid->dev ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-mode=", 6 ) == 0 ) {
                if( strncmp( &a[6], "PAL", 3 ) == 0 )        vid->mode = VIDEO_MODE_PAL;
                else if( strncmp( &a[6], "NTSC", 4 ) == 0 )  vid->mode = VIDEO_MODE_NTSC;
                else if( strncmp( &a[6], "SECAM", 5 ) == 0 ) vid->mode = VIDEO_MODE_SECAM;
                else {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-debug", 6 ) == 0 ) {
                vid->debug = 1;
            }
            else {
                ar2VideoDispOption();
                free( vid );
                return 0;
            }

            while( *a != ' ' && *a != '\t' && *a != '\0') a++;
        }
    }

    vid->fd = open(vid->dev, O_RDWR);
    if(vid->fd < 0){
        printf("video device (%s) open failed\n",vid->dev); 
        free( vid );
        return 0;
    }

    if(ioctl(vid->fd,VIDIOCGCAP,&vd) < 0){
        printf("ioctl failed\n");
        free( vid );
        return 0;
    }

    if( vid->debug ) {
        printf("=== debug info ===\n");
        printf("  vd.name      =   %s\n",vd.name);
        printf("  vd.channels  =   %d\n",vd.channels);
        printf("  vd.maxwidth  =   %d\n",vd.maxwidth);
        printf("  vd.maxheight =   %d\n",vd.maxheight);
        printf("  vd.minwidth  =   %d\n",vd.minwidth);
        printf("  vd.minheight =   %d\n",vd.minheight);
    }
    
    /* check capture size */
    if(vd.maxwidth  < vid->width  || vid->width  < vd.minwidth ||
       vd.maxheight < vid->height || vid->height < vd.minheight ) {
        printf("arVideoOpen: width or height oversize \n");
        free( vid );
        return 0;
    }

    /* check channel */
    if(vid->channel < 0 || vid->channel >= vd.channels){
        printf("arVideoOpen: channel# is not valid. \n");
        free( vid );
        return 0;
    }

    if( vid->debug ) {
        printf("==== capture device channel info ===\n");
    }

    for(i = 0;i < vd.channels && i < MAXCHANNEL; i++){
        vc[i].channel = i;
        if(ioctl(vid->fd,VIDIOCGCHAN,&vc[i]) < 0){
            printf("error: acquireing channel(%d) info\n",i);
            free( vid );
            return 0;
        }

        if( vid->debug ) {
            printf("    channel = %d\n",  vc[i].channel);
            printf("       name = %s\n",  vc[i].name);
            printf("     tuners = %d",    vc[i].tuners);

            printf("       flag = 0x%08x",vc[i].flags);
            if(vc[i].flags & VIDEO_VC_TUNER) 
                printf(" TUNER");
            if(vc[i].flags & VIDEO_VC_AUDIO) 
                printf(" AUDIO");
            printf("\n");

            printf("     vc[%d].type = 0x%08x", i, vc[i].type);
            if(vc[i].type & VIDEO_TYPE_TV) 
                printf(" TV");
            if(vc[i].type & VIDEO_TYPE_CAMERA) 
                printf(" CAMERA");
            printf("\n");       
        }
    }

    /* select channel */
    vc[vid->channel].norm = vid->mode;       /* 0: PAL 1: NTSC 2:SECAM 3:AUTO */
    if(ioctl(vid->fd, VIDIOCSCHAN, &vc[vid->channel]) < 0){
        printf("error: selecting channel %d\n", vid->channel);
        free( vid );
        return 0;
    }

    /* set video picture */
    vp.brightness = 32767 * 2.0 * vid->brightness;
    vp.hue        = 32767;
    vp.colour     = 32767 * 2.0 * vid->color;
    vp.contrast   = 32767 * 2.0 * vid->contrast;
    vp.whiteness  = 32767;
    vp.depth      = 24;                      /* color depth    */
#if defined(AR_PIX_FORMAT_BGRA)
    vp.palette    = VIDEO_PALETTE_RGB32;     /* palette format */
#elif defined(AR_PIX_FORMAT_BGR)
    vp.palette    = VIDEO_PALETTE_RGB24;     /* palette format */
#endif
    if(ioctl(vid->fd, VIDIOCSPICT, &vp)) {
        printf("error: setting palette\n");
        free( vid );
        return 0;
    }

    /* get mmap info */
    if(ioctl(vid->fd,VIDIOCGMBUF,&vid->vm) < 0){
        printf("error: videocgmbuf\n");
        free( vid );
        return 0;
    }

    if( vid->debug ) {
        printf("===== Image Buffer Info =====\n");
        printf("   size   =  %d[bytes]\n", vid->vm.size);
        printf("   frames =  %d\n", vid->vm.frames);
    }
    if(vid->vm.frames < 2){
        printf("this device can not be supported by libARvideo.\n");
        printf("(vm.frames < 2)\n");
        free( vid );
        return 0;
    }


    /* get memory mapped io */
    if((vid->map = (ARUint8 *)mmap(0, vid->vm.size, PROT_READ|PROT_WRITE, MAP_SHARED, vid->fd, 0)) < 0){
        printf("error: mmap\n");
        free( vid );
        return 0;
    }

    /* setup for vmm */ 
    vid->vmm.frame  = 0;
    vid->vmm.width  = vid->width;
    vid->vmm.height = vid->height;
#if defined(AR_PIX_FORMAT_BGRA)
    vid->vmm.format = VIDEO_PALETTE_RGB32;
#elif defined(AR_PIX_FORMAT_BGR)
    vid->vmm.format = VIDEO_PALETTE_RGB24;
#endif

    vid->video_cont_num = -1;

    return vid;
}

int ar2VideoClose( AR2VideoParamT *vid )
{
    if(vid->video_cont_num >= 0){
        ar2VideoCapStop( vid );
    }
    close(vid->fd);
    free( vid );

    return 0;
} 


int ar2VideoCapStart( AR2VideoParamT *vid )
{
    if(vid->video_cont_num >= 0){
        printf("arVideoCapStart has already been called.\n");
        return -1;
    }

    vid->video_cont_num = 0;
    vid->vmm.frame      = vid->video_cont_num;
    if(ioctl(vid->fd, VIDIOCMCAPTURE, &vid->vmm) < 0) {
        return -1;
    }
    vid->vmm.frame = 1 - vid->vmm.frame;
    if( ioctl(vid->fd, VIDIOCMCAPTURE, &vid->vmm) < 0) {
        return -1;
    }

    return 0;
}

int ar2VideoCapNext( AR2VideoParamT *vid )
{
    if(vid->video_cont_num < 0){
        printf("arVideoCapStart has never been called.\n");
        return -1;
    }

    vid->vmm.frame = 1 - vid->vmm.frame;
    ioctl(vid->fd, VIDIOCMCAPTURE, &vid->vmm);

    return 0;
}

int ar2VideoCapStop( AR2VideoParamT *vid )
{
    if(vid->video_cont_num < 0){
        printf("arVideoCapStart has never been called.\n");
        return -1;
    }
    if(ioctl(vid->fd, VIDIOCSYNC, &vid->video_cont_num) < 0){
        printf("error: videosync\n");
        return -1;
    }
    vid->video_cont_num = -1;

    return 0;
}


ARUint8 *ar2VideoGetImage( AR2VideoParamT *vid )
{
    if(vid->video_cont_num < 0){
        printf("arVideoCapStart has never been called.\n");
        return NULL;
    }

    if(ioctl(vid->fd, VIDIOCSYNC, &vid->video_cont_num) < 0){
        printf("error: videosync\n");
        return NULL;
    }
    vid->video_cont_num = 1 - vid->video_cont_num;

    if(vid->video_cont_num == 0)
        return (vid->map + vid->vm.offsets[1]); 
    else
        return (vid->map + vid->vm.offsets[0]);
}

int ar2VideoInqSize(AR2VideoParamT *vid, int *x,int *y)
{
    *x = vid->vmm.width;
    *y = vid->vmm.height;

    return 0;
}

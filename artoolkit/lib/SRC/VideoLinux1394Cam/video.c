/* 
 *   Video capture subrutine for Linux/libdc1394 devices
 *   author: Kiyoshi Kiyokawa ( kiyo@crl.go.jp )
             Hirokazu Kato ( kato@sys.im.hiroshima-cu.ac.jp )
 *
 *   Revision: 1.0   Date: 2002/01/01
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
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include <AR/config.h>
#include <AR/ar.h>
#include <AR/video.h>


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


typedef struct __arVideo1394
{
    raw1394handle_t         handle;
    int                     numNodes;
    int                     numCameras;
    nodeid_t               *camera_nodes;
    int                    *activeFlag;
} ARVideo1394;

static ARVideo1394   arV1394;
static int           initFlag = 0;

static int ar2Video1394Init( int debug );


int ar2VideoDispOption( void )
{
    printf("ARVideo may be configured using one or more of the following options,\n");
    printf("separated by a space:\n\n");
    printf(" -node=N\n");
    printf("    specifies detected node ID of a FireWire camera (-1: Any).\n");
    printf(" -mode=[320x240_YUV422|640x480_RGB|640x480_YUV411|\n");
    printf("        640x480_YUV411_HALF|640x480_MONO|640x480_MONO_COLOR\n");
    printf("        640x480_MONO_COLOR_HALF]\n");
    printf("    specifies input image format.\n");
    printf(" -rate=N\n");
    printf("    specifies desired framerate of a FireWire camera. \n");
    printf("    (1.875, 3.75, 7.5, 15, 30, 60)\n");
    printf("\n");

    return 0;
}

AR2VideoParamT *ar2VideoOpen( char *config )
{
    AR2VideoParamT            *vid;
    ARUint32                  p1,p2;
    quadlet_t                 value;
    char                      *a, line[256];
    int                       i;

    arMalloc( vid, AR2VideoParamT, 1 );
    vid->node         = DEFAULT_VIDEO_NODE;
    vid->mode         = DEFAULT_VIDEO_MODE;
    vid->rate         = DEFAULT_VIDEO_FRAME_RATE;
    //vid->channel      = 0;
    vid->speed        = SPEED_400;
    vid->format       = FORMAT_VGA_NONCOMPRESSED;
    vid->dma_buf_num  = 16;
    vid->debug        = 0;
    vid->status       = 0;

    a = config;
    if( a != NULL) {
        for(;;) {
            while( *a == ' ' || *a == '\t' ) a++;
            if( *a == '\0' ) break;

            if( strncmp( a, "-mode=", 6 ) == 0 ) {
                if ( strncmp( &a[6], "320x240_YUV422", 14 ) == 0 ) {
                    vid->mode = VIDEO_MODE_320x240_YUV422;
                }
                else if ( strncmp( &a[6], "640x480_YUV411_HALF", 19 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_YUV411_HALF;
                }
                else if ( strncmp( &a[6], "640x480_YUV411", 14 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_YUV411;
                }
                else if ( strncmp( &a[6], "640x480_RGB", 11 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_RGB;
                }
                else if ( strncmp( &a[6], "640x480_MONO_COLOR_HALF", 23 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_MONO_COLOR_HALF;
                }
                else if ( strncmp( &a[6], "640x480_MONO_COLOR", 18 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_MONO_COLOR;
                }
                else if ( strncmp( &a[6], "640x480_MONO", 12 ) == 0 ) {
                    vid->mode = VIDEO_MODE_640x480_MONO;
                }
                else {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-node=", 6 ) == 0 ) {
                sscanf( a, "%s", line );
                if( sscanf( &line[6], "%d", &vid->node ) == 0 ) {
                    ar2VideoDispOption();
                    free( vid );
                    return 0;
                }
            }
            else if( strncmp( a, "-rate=", 6 ) == 0 ) {
                if ( strncmp( &a[6], "1.875", 5 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_1_875;
                }
                else if ( strncmp( &a[6], "3.75", 4 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_3_75;
                }
                else if ( strncmp( &a[6], "7.5", 3 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_7_5;
                }
                else if ( strncmp( &a[6], "15", 2 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_15;
                }
                else if ( strncmp( &a[6], "30", 2 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_30;
                }
                else if ( strncmp( &a[6], "60", 2 ) == 0 ) {
                    vid->rate = VIDEO_FRAME_RATE_60;
                }
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

    if( initFlag == 0 ) {
        if( ar2Video1394Init(vid->debug) < 0 ) exit(0);
        initFlag = 1;
    }
  

    for( i=0; i<arV1394.numCameras; i++ ) {
        if( (vid->node == VIDEO_NODE_ANY && arV1394.activeFlag[i] == 0) || (arV1394.camera_nodes[i] == vid->node) ) {
            if( vid->debug ) printf("Using a camera #%d (Node ID = %d)\n", i, arV1394.camera_nodes[i]);
	    vid->internal_id = i;
            if( vid->node == VIDEO_NODE_ANY ) vid->node = arV1394.camera_nodes[i];
            break;
	}
    }
    if( i == arV1394.numCameras ) {
        printf("cound not find the specified camera.\n");
        exit(0);
    }

    vid->channel = vid->node;
  
    switch( vid->mode ) {
        case VIDEO_MODE_320x240_YUV422:
          vid->int_mode = MODE_320x240_YUV422;
          break;
        case VIDEO_MODE_640x480_YUV411:
        case VIDEO_MODE_640x480_YUV411_HALF:
          vid->int_mode = MODE_640x480_YUV411;
          break;
        case VIDEO_MODE_640x480_RGB:
          vid->int_mode = MODE_640x480_RGB;
          break;
        case VIDEO_MODE_640x480_MONO:
        case VIDEO_MODE_640x480_MONO_COLOR:
        case VIDEO_MODE_640x480_MONO_COLOR_HALF:
          vid->int_mode = MODE_640x480_MONO;
          break;
        default:
          printf("Sorry, Unsupported Video Format for IEEE1394 Camera.\n");
          exit(1);
    }
    switch( vid->rate ) {
        case VIDEO_FRAME_RATE_1_875:
          vid->int_rate = FRAMERATE_1_875;
          break;
        case VIDEO_FRAME_RATE_3_75:
          vid->int_rate = FRAMERATE_3_75;
          break;
        case VIDEO_FRAME_RATE_7_5:
          vid->int_rate = FRAMERATE_7_5;
          break;
        case VIDEO_FRAME_RATE_15:
          vid->int_rate = FRAMERATE_15;
          break;
        case VIDEO_FRAME_RATE_30:
          vid->int_rate = FRAMERATE_30;
          break;
        case VIDEO_FRAME_RATE_60:
          vid->int_rate = FRAMERATE_60;
          break;
        default:
          printf("Sorry, Unsupported Frame Rate for IEEE1394 Camera.\n");
          exit(1);
    }
  
    /*-----------------------------------------------------------------------*/
    /*  report camera's features                                             */
    /*-----------------------------------------------------------------------*/
    if( dc1394_get_camera_feature_set(arV1394.handle, 
				      vid->node,
				      &(vid->features)) != DC1394_SUCCESS ) {
        fprintf( stderr, "unable to get feature set\n");
    }
    else if( vid->debug ) {
      dc1394_print_feature_set( &(vid->features) );
    }

    /*-----------------------------------------------------------------------*/
    /*  check parameters                                                     */
    /*-----------------------------------------------------------------------*/
    if( dc1394_query_supported_formats(arV1394.handle, vid->node, &value) != DC1394_SUCCESS ) {
        fprintf( stderr, "unable to query_supported_formats\n");
    }
    i = 31 - (FORMAT_VGA_NONCOMPRESSED - FORMAT_MIN);
    p1 = 1 << i;
    p2 = value & p1;
    if( p2 == 0 ) {
        fprintf( stderr, "unable to use this camera on VGA_NONCOMPRESSED format.\n");
        exit(0);
    }
    dc1394_query_supported_modes(arV1394.handle, vid->node,  FORMAT_VGA_NONCOMPRESSED, &value);
    i = 31 - (vid->int_mode - MODE_FORMAT0_MIN);
    p1 = 1 << i;
    p2 = value & p1;
    if( p2 == 0 ) {
        fprintf( stderr, "Unsupported Mode for the specified camera.\n");
        ar2VideoDispOption();
        exit(0);
    }
    dc1394_query_supported_framerates(arV1394.handle, vid->node, FORMAT_VGA_NONCOMPRESSED, vid->int_mode, &value);
    i = 31 - (vid->int_rate - FRAMERATE_MIN);
    p1 = 1 << i;
    p2 = value & p1;
    if( p2 == 0 ) {
        fprintf( stderr, "Unsupported Framerate for the specified mode.\n");
        ar2VideoDispOption();
        exit(0);
    }

    /*-----------------------------------------------------------------------*/
    /*  setup capture                                                        */
    /*-----------------------------------------------------------------------*/
    if( dc1394_dma_setup_capture(arV1394.handle,
			         vid->node,
			         vid->channel,
			         vid->format,
			         vid->int_mode,
			         vid->speed,
			         vid->int_rate,
			         vid->dma_buf_num,
			         &(vid->camera)) != DC1394_SUCCESS ) {
        fprintf( stderr,"unable to setup camera-\n"
                "check if you did 'insmod video1394' or,\n"
                "check line %d of %s to make sure\n"
                "that the video mode,framerate and format are\n"
                "supported by your camera\n",
                __LINE__,__FILE__);
        exit(0);
    }
  
    /* set trigger mode */
    if( dc1394_set_trigger_mode(arV1394.handle, vid->node, TRIGGER_MODE_0) != DC1394_SUCCESS ) {
        fprintf( stderr, "unable to set camera trigger mode (ignored)\n");
    }
  
    arMalloc( vid->image, ARUint8, (vid->camera.frame_width * vid->camera.frame_height * AR_PIX_SIZE) );

    arV1394.activeFlag[vid->internal_id] = 1;

    return vid;
}

int ar2VideoClose( AR2VideoParamT *vid )
{
    int     i;

    if( vid->status > 0 ) ar2VideoCapStop( vid );

#if 0
    dc1394_dma_release_camera(arV1394.handle, &(vid->camera));
#endif
    arV1394.activeFlag[vid->internal_id] = 0;
    free( vid->image );
    free( vid );
  
    for( i=0; i<arV1394.numCameras; i++ ) {
        if( arV1394.activeFlag[i] ) break;
    }
    if( i == arV1394.numCameras ) {
        raw1394_destroy_handle(arV1394.handle);
        free (arV1394.activeFlag);
        initFlag = 0;
    }

    return 0;
} 

int ar2VideoCapStart( AR2VideoParamT *vid )
{
    if(vid->status != 0 && vid->status != 3){
        printf("arVideoCapStart has already been called.\n");
        return -1;
    }

    /*-----------------------------------------------------------------------*/
    /*  setup capture                                                        */
    /*-----------------------------------------------------------------------*/
    if( vid->status == 3 ) {
        if( dc1394_dma_setup_capture(arV1394.handle,
			             vid->node,
			             vid->channel,
			             vid->format,
			             vid->int_mode,
			             vid->speed,
			             vid->int_rate,
			             vid->dma_buf_num,
			             &(vid->camera)) != DC1394_SUCCESS ) {
            fprintf( stderr,"unable to setup camera-\n"
                    "check if you did 'insmod video1394' or,\n"
                    "check line %d of %s to make sure\n"
                    "that the video mode,framerate and format are\n"
                    "supported by your camera\n",
                    __LINE__,__FILE__);
            exit(0);
        }
    }

    if( dc1394_start_iso_transmission(arV1394.handle, vid->node) != DC1394_SUCCESS ) {
        fprintf( stderr, "unable to start camera iso transmission\n");
        return -1;
    }

    vid->status = 1;

    return 0;
}

int ar2VideoCapNext( AR2VideoParamT *vid )
{
    if(vid->status == 0 || vid->status == 3){
        printf("arVideoCapStart has never been called.\n");
        return -1;
    }
    if(vid->status == 2) vid->status = 1;

    dc1394_dma_done_with_buffer( &(vid->camera) );

    return 0;
}

int ar2VideoCapStop( AR2VideoParamT *vid )
{
    if(vid->status == 2){
        if( dc1394_dma_single_capture( &(vid->camera) ) != DC1394_SUCCESS ) {
            fprintf( stderr, "unable to capture a frame\n");
        }
    }
    if(vid->status == 0){
        printf("arVideoCapStart has never been called.\n");
        return -1;
    }
    vid->status = 3;

    if( dc1394_stop_iso_transmission(arV1394.handle, vid->node) != DC1394_SUCCESS ) {
        printf("couldn't stop the camera?\n");
        return -1;
    }

    dc1394_dma_release_camera(arV1394.handle, &(vid->camera));

    return 0;
}

int ar2VideoInqSize(AR2VideoParamT *vid, int *x,int *y)
{
    *x = vid->camera.frame_width;
    *y = vid->camera.frame_height;

    return 0;
}

ARUint8 *ar2VideoGetImage( AR2VideoParamT *vid )
{
    register ARUint8 *buf, *buf2;
    register ARUint8 *p1, *p2, *p3;
    register int i, j;
    register int U, V, R, G, B, V2, U5, UV;
    register int Y0, Y1, Y2, Y3;
    register ARUint8 r, g, b;

    if(vid->status == 0){
        printf("arVideoCapStart has never been called.\n");
        return NULL;
    }
    if(vid->status == 2){
        printf("arVideoCapNext has never been called since previous arVideoGetImage.\n");
        return NULL;
    }

    if( dc1394_dma_single_capture( &(vid->camera) ) != DC1394_SUCCESS ) {
        fprintf( stderr, "unable to capture a frame\n");
        return NULL;
    }
    vid->status = 2;


    switch( vid->mode ) {
        case VIDEO_MODE_640x480_RGB:
          return (ARUint8 *)vid->camera.capture_buffer;

        case VIDEO_MODE_640x480_YUV411:
          buf  = vid->image;
          buf2 = (ARUint8 *)vid->camera.capture_buffer;
          for( i = vid->camera.frame_height * vid->camera.frame_width / 4; i; i--) {
              U   = ((ARUint8)*buf2++ - 128) * 0.354;
              U5  = 5*U;
              Y0  = (ARUint8)*buf2++;
              Y1  = (ARUint8)*buf2++;
              V   = ((ARUint8)*buf2++ - 128) * 0.707;
              V2  = 2*V;
              Y2  = (ARUint8)*buf2++;
              Y3  = (ARUint8)*buf2++;
              UV  = - U - V;

              // Original equations
              // R = Y           + 1.402 V
              // G = Y - 0.344 U - 0.714 V
              // B = Y + 1.772 U
              R = Y0 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y0 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y0 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;

              //---
              R = Y1 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y1 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y1 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;

              //---
              R = Y2 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y2 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y2 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;

              //---
              R = Y3 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y3 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y3 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;
          }
          return vid->image;

        case VIDEO_MODE_640x480_YUV411_HALF:
          buf  = vid->image;
          buf2 = (ARUint8 *)vid->camera.capture_buffer;
          for( j = 0; j < vid->camera.frame_height / 2; j++ ) {
              for( i = 0; i < vid->camera.frame_width / 4; i++ ) {
                  U   = ((ARUint8)*buf2++ - 128) * 0.354;
                  U5  = 5*U;
                  Y0  = (ARUint8)*buf2++;
                  Y1  = (ARUint8)*buf2++;
                  V   = ((ARUint8)*buf2++ - 128) * 0.707;
                  V2  = 2*V;
                  Y2  = (ARUint8)*buf2++;
                  Y3  = (ARUint8)*buf2++;
                  UV  = - U - V;

                  // Original equations
                  // R = Y           + 1.402 V
                  // G = Y - 0.344 U - 0.714 V
                  // B = Y + 1.772 U
                  R = Y0 + V2;
                  if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

                  G = Y0 + UV;
                  if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

                  B = Y0 + U5;
                  if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
                  *buf++ = (ARUint8)R;
                  *buf++ = (ARUint8)G;
                  *buf++ = (ARUint8)B;

                  //---
                  R = Y1 + V2;
                  if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

                  G = Y1 + UV;
                  if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

                  B = Y1 + U5;
                  if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
                  *buf++ = (ARUint8)R;
                  *buf++ = (ARUint8)G;
                  *buf++ = (ARUint8)B;

                  //---
                  R = Y2 + V2;
                  if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;
    
                  G = Y2 + UV;
                  if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

                  B = Y2 + U5;
                  if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
                  *buf++ = (ARUint8)R;
                  *buf++ = (ARUint8)G;
                  *buf++ = (ARUint8)B;

                  //---
                  R = Y3 + V2;
                  if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

                  G = Y3 + UV;
                  if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

                  B = Y3 + U5;
                  if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
     
                  *buf++ = (ARUint8)R;
                  *buf++ = (ARUint8)G;
                  *buf++ = (ARUint8)B;
              }
              buf2 += (vid->camera.frame_width / 4) * 6;
              buf  += vid->camera.frame_width * 3;
          }
          return vid->image;

        case VIDEO_MODE_320x240_YUV422:
          buf  = vid->image;
          buf2 = (ARUint8 *)vid->camera.capture_buffer;
          for( i = vid->camera.frame_height * vid->camera.frame_width / 2; i; i-- ) {
              U   = ((ARUint8)*buf2++ - 128) * 0.354;
              U5  = 5*U;
              Y0  = (ARUint8)*buf2++;
              V   = ((ARUint8)*buf2++ - 128) * 0.707;
              V2  = 2*V;
              Y1  = (ARUint8)*buf2++;
              UV  = - U - V;

              //---
              R = Y0 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y0 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y0 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
      
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;

              //---
              R = Y1 + V2;
              if ((R >> 8) > 0) R = 255; else if (R < 0) R = 0;

              G = Y1 + UV;
              if ((G >> 8) > 0) G = 255; else if (G < 0) G = 0;

              B = Y1 + U5;
              if ((B >> 8) > 0) B = 255; else if (B < 0) B = 0;
      
              *buf++ = (ARUint8)R;
              *buf++ = (ARUint8)G;
              *buf++ = (ARUint8)B;
          }
          return vid->image;

        case VIDEO_MODE_640x480_MONO:
          buf  = vid->image;
          buf2 = (ARUint8 *)vid->camera.capture_buffer;
          for( i = vid->camera.frame_height * vid->camera.frame_width; i; i--) {
              *buf++ = *buf2;
              *buf++ = *buf2;
              *buf++ = *buf2++;
          }
          return vid->image;

        case VIDEO_MODE_640x480_MONO_COLOR:
          buf  = vid->image;
          p2 = (ARUint8 *)vid->camera.capture_buffer;
          p3 = p2 + vid->camera.frame_width;

          *buf++ = *p2;
          *buf++ = *(p2+1);
          *buf++ = *(p3+1);
          p2++;
          p3++;
          for( i = vid->camera.frame_width/2-1; i; i-- ) {
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              *buf++ = *p2;
              *buf++ = *p3;
              p2++;
              p3++;
              *buf++ = *p2;
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              *buf++ = ((int)*(p3-1) + (int)*(p3+1))/2;
              p2++;
              p3++;
          }
          *buf++ = *(p2-1);
          *buf++ = *p2;
          *buf++ = *p3;
          p2++;
          p3++;
          p1 = (ARUint8 *)vid->camera.capture_buffer;

          for( j = vid->camera.frame_height/2-1; j; j-- ) {
              *buf++ = ((int)*p1 + (int)*p3)/2;
              *buf++ = *p2;
              *buf++ = *(p2+1);
              p1++;
              p2++;
              p3++;
              for( i = vid->camera.frame_width/2-1; i; i-- ) {
                  *buf++ = ((int)*(p1-1) + (int)*(p1+1) + (int)*(p3-1) + (int)*(p3+1))/4;
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  *buf++ = *p2;
                  p1++;
                  p2++;
                  p3++;
                  *buf++ = ((int)*p1 + (int)*p3)/2;
                  *buf++ = *p2;
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  p1++;
                  p2++;
                  p3++;
              }
              *buf++ = ((int)*(p1-1) + (int)*(p3-1))/2;
              *buf++ = ((int)*p1 + (int)*p3)/2;
              *buf++ = *p2;
              p1++;
              p2++;
              p3++;

              *buf++ = *p2;
              *buf++ = ((int)*p1 + (int)*p3)/2;
              *buf++ = ((int)*(p1+1) + (int)*(p3+1))/2;
              p1++;
              p2++;
              p3++;
              for( i = vid->camera.frame_width/2-1; i; i-- ) {
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  *buf++ = *p2;
                  *buf++ = ((int)*p1 + (int)*p3)/2;
                  p1++;
                  p2++;
                  p3++;
                  *buf++ = *p2;
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  *buf++ = ((int)*(p1-1) + (int)*(p1+1) + (int)*(p3-1) + (int)*(p3+1))/4;
                  p1++;
                  p2++;
                  p3++;
              }
              *buf++ = *(p2-1);
              *buf++ = *p2;
              *buf++ = ((int)*p1 + (int)*p3)/2;
              p1++;
              p2++;
              p3++;
          }

          *buf++ = *p1;
          *buf++ = *p2;
          *buf++ = *(p2+1);
          p1++;
          p2++;
          for( i = vid->camera.frame_width/2; i; i-- ) {
              *buf++ = ((int)*(p1-1) + (int)*(p1+1))/2;
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              *buf++ = *p2;
              p1++;
              p2++;
              *buf++ = *p1;
              *buf++ = *p2;
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              p1++;
              p2++;
          }
          *buf++ = *(p1-1);
          *buf++ = *(p2-1);
          *buf++ = *p2;

          return vid->image;

        case VIDEO_MODE_640x480_MONO_COLOR_HALF:
          buf  = vid->image;
          p2 = (ARUint8 *)vid->camera.capture_buffer;
          p3 = p2 + vid->camera.frame_width;

          *buf++ = *p2;
          *buf++ = *(p2+1);
          *buf++ = *(p3+1);
          p2++;
          p3++;
          for( i = vid->camera.frame_width/2-1; i; i-- ) {
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              *buf++ = *p2;
              *buf++ = *p3;
              p2++;
              p3++;
              *buf++ = *p2;
              *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
              *buf++ = ((int)*(p3-1) + (int)*(p3+1))/2;
              p2++;
              p3++;
          }
          *buf++ = *(p2-1);
          *buf++ = *p2;
          *buf++ = *p3;
          p2++;
          p3++;
          p1 = (ARUint8 *)vid->camera.capture_buffer;

          for( j = vid->camera.frame_height/2-1; j; j-- ) {
              p1 += vid->camera.frame_width;
              p2 += vid->camera.frame_width;
              p3 += vid->camera.frame_width;
              buf += vid->camera.frame_width * 3;

              *buf++ = *p2;
              *buf++ = ((int)*p1 + (int)*p3)/2;
              *buf++ = ((int)*(p1+1) + (int)*(p3+1))/2;
              p1++;
              p2++;
              p3++;
              for( i = vid->camera.frame_width/2-1; i; i-- ) {
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  *buf++ = *p2;
                  *buf++ = ((int)*p1 + (int)*p3)/2;
                  p1++;
                  p2++;
                  p3++;
                  *buf++ = *p2;
                  *buf++ = ((int)*(p2-1) + (int)*(p2+1))/2;
                  *buf++ = ((int)*(p1-1) + (int)*(p1+1) + (int)*(p3-1) + (int)*(p3+1))/4;
                  p1++;
                  p2++;
                  p3++;
              }
              *buf++ = *(p2-1);
              *buf++ = *p2;
              *buf++ = ((int)*p1 + (int)*p3)/2;
              p1++;
              p2++;
              p3++;
          }

          return vid->image;
    }

    return NULL;
}




static int ar2Video1394Init( int debug )
{
    int     i;

    /*-----------------------------------------------------------------------*/
    /*  Open ohci and asign handle to it                                     */
    /*-----------------------------------------------------------------------*/
    arV1394.handle = dc1394_create_handle(0);
    if (arV1394.handle==NULL) {
        fprintf( stderr, "Unable to aquire a raw1394 handle\n"
                "did you insmod the drivers?\n");
        exit(0);
    }

    /*-----------------------------------------------------------------------*/
    /*  get the camera nodes and describe them as we find them               */
    /*-----------------------------------------------------------------------*/
    arV1394.numNodes = raw1394_get_nodecount(arV1394.handle);
    arV1394.camera_nodes = dc1394_get_camera_nodes(arV1394.handle,&arV1394.numCameras,((debug)? 1: 0));
    fflush(stdout);
    if (arV1394.numCameras<1) {
        fprintf( stderr, "no cameras found :(\n");
        raw1394_destroy_handle(arV1394.handle);
        exit(0);
    }
    if( debug ) {
        printf("%d camera(s) found on the bus\n", arV1394.numCameras);
        printf("Node IDs are: ");
        for (i=0; i<arV1394.numCameras; i++) {
	    printf("%d, ", arV1394.camera_nodes[i]);
        }
        printf("\n");
    }

    arMalloc( arV1394.activeFlag, int, arV1394.numCameras );
    for( i=0; i<arV1394.numCameras; i++ ) {
	if( arV1394.camera_nodes[i] == arV1394.numNodes-1) {
	    fprintf( stderr, "\n"
                    "If ohci1394 is not working as root, please do:\n"
                    "\n"
                    "   rmmod ohci1394\n"
                    "   insmod ohci1394 attempt_root=1\n"
                    "\n"
                    "Otherwise, try to change FireWire connections so that\n"
                    "the highest number is not given to any camera.\n");
            exit(0);
        }

        arV1394.activeFlag[i] = 0;
    }

    return 0;
}


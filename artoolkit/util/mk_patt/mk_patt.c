#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>

#if defined(__sgi)
char            *vconf = "-size=FULL";
#elif defined(__linux)
#  if defined(AR_INPUT_V4L)
char            *vconf = "-width=640 -height=480";
#  elif defined(AR_INPUT_1394CAM)
#    ifndef DRAGONFLY
char            *vconf = "-mode=640x480_YUV411";
#    else
char            *vconf = "-mode=640x480_MONO_COLOR";
#    endif
#  elif defined(AR_INPUT_DV)
char            *vconf = "";
#  endif
#elif defined(_WIN32)
char			*vconf = "flipV,showDlg";
#elif defined(__APPLE__)
char			*vconf = "-width=640 -height=480";
#else
char			*vconf = "";
#endif


int             xsize;
int             ysize;

ARUint8*        image   = NULL;
ARMarkerInfo*   target  = NULL;
ARParam         param;
int             thresh  = 100;


static void init(void);
static void cleanup(void);
static void keyEvent( unsigned char key, int x, int y);
static void mouseEvent(int button, int state, int x, int y);
static void mainLoop(void);
static void img_copy( ARUint8 *src, ARUint8 *dst, int size );

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
    init();

    arVideoCapStart();
    argMainLoop( mouseEvent, keyEvent, mainLoop );
	return (0);
}


static void init( void )
{
    ARParam  wparam;
    char     name1[256], name2[256];

    printf("Enter camera parameter filename");
    printf("(Data/camera_para.dat): ");
    if( fgets(name1, 256, stdin) == NULL ) exit(0);
    if( sscanf(name1, "%s", name2) != 1 ) {
        strcpy( name2, "Data/camera_para.dat");
    }
    if( arParamLoad(name2, 1, &wparam) < 0 ) {
        printf("Parameter load error !!\n");
        exit(0);
    }

    if( arVideoOpen(vconf) < 0 ) exit(0);
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    arMalloc( image, ARUint8, xsize*ysize*AR_PIX_SIZE );
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    arParamChangeSize( &wparam, xsize, ysize, &param );
    arParamDisp( &param );
    arInitCparam( &param );

    argInit( &param, 1.0, 0, 0, 0, 0 );
    argDrawMode2D();
}

static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1b ) {
        cleanup();
        exit(0);
    }

    /* change the threshold value when 't' key pressed */
    if( key == 't' ) {
        printf("Enter new threshold value (default = 100): ");
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
    }
}

static void mouseEvent(int button, int state, int x, int y)
{
    char   name1[256], name2[256];

    if( button == GLUT_RIGHT_BUTTON  && state == GLUT_DOWN ) {
        cleanup();
        exit(0);
    }
    if( button == GLUT_MIDDLE_BUTTON  && state == GLUT_DOWN ) {
        printf("Enter new threshold value (default = 100): ");
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
    }
    if( button == GLUT_LEFT_BUTTON  && state == GLUT_DOWN && target != NULL ) {
        printf("Enter filename: ");
        if( fgets(name1, 256, stdin) == NULL ) return;
        if( sscanf(name1, "%s", name2) != 1 ) return;
        if( arSavePatt(image, target, name2) < 0 ) {
            printf("ERROR!!\n");
        }
        else {
            printf("  Saved\n");
        }
    }
}

static void mainLoop(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             areamax;
    int             i;

    if( (dataPtr = (unsigned char *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    img_copy( dataPtr, image, xsize*ysize*AR_PIX_SIZE );
    arVideoCapNext();

    if( arDetectMarker(image, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }

    areamax = 0;
    target = NULL;
    for( i = 0; i < marker_num; i++ ) {
        if( marker_info[i].area > areamax ) {
            areamax = marker_info[i].area;
            target = &(marker_info[i]);
        }
    }
    argDispImage( image, 0, 0 );


    if( target != NULL ) {
        glLineWidth( 2.0 );
        glColor3d( 0.0, 1.0, 0.0 );
        argLineSeg( target->vertex[0][0], target->vertex[0][1],
                    target->vertex[1][0], target->vertex[1][1], 0, 0 );
        argLineSeg( target->vertex[3][0], target->vertex[3][1],
                    target->vertex[0][0], target->vertex[0][1], 0, 0 );
        glColor3d( 1.0, 0.0, 0.0 );
        argLineSeg( target->vertex[1][0], target->vertex[1][1],
                    target->vertex[2][0], target->vertex[2][1], 0, 0 );
        argLineSeg( target->vertex[2][0], target->vertex[2][1],
                    target->vertex[3][0], target->vertex[3][1], 0, 0 );
    }
    argSwapBuffers();
 
    return;
}

static void img_copy( ARUint8 *src, ARUint8 *dst, int size )
{
    while( (size--) > 0 ) *(dst++) = *(src++);
}

#ifdef _WIN32
#include <windows.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
#  include <GL/glut.h>
#  ifdef GL_VERSION_1_2
#    include <GL/glext.h>
#  endif
#else
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#endif
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/matrix.h>
#include "calib_camera.h"

#define   USE_TEXMAP

#ifndef GL_ABGR
#define GL_ABGR GL_ABGR_EXT
#endif
#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif
#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif
#ifndef GL_RGBA
#define GL_RGBA GL_RGBA_EXT
#endif
#ifndef GL_RGB
#define GL_RGB GL_RGB_EXT
#endif

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
char			*vconf = "WDM_camera_flipV.xml";
#elif defined(__APPLE__)
char			*vconf = "-width=640 -height=480";
#else
char			*vconf = "";
#endif
/*****************************************************************************/


int             win;
int             xsize;
int             ysize;
int             thresh = THRESH;
unsigned char   *clipImage;

CALIB_PATT_T    patt;          
double          dist_factor[4];
double          mat[3][4];

int             point_num;
int             sx, sy, ex, ey;

int             status;
int             check_num;

#ifdef USE_TEXMAP
static GLuint   glid[2];
static int      tex1Xsize1 = 1;
static int      tex1Xsize2 = 1;
static int      tex1Ysize  = 1;
static void dispImageTex1( unsigned char *pimage );
static void dispImageTex2( unsigned char *pimage );
#endif

static void     init( int argc, char *argv[] );
static void     mouseEvent(int button, int state, int x, int y);
static void     motionEvent( int x, int y );
static void     keyEvent(unsigned char key, int x, int y);
static void     dispImage(void);
static void     dispImage2( unsigned char *pimage );
static void     dispClipImage( int sx, int sy, int xsize, int ysize, ARUint8 *clipImage );
static void     draw_warp_line( double a, double b , double c );
static void     draw_line(void);
static void     draw_line2( double *x, double *y, int num );
static void     draw_warp_line( double a, double b , double c );
static void     print_comment( int status );
static void     save_param(void);

static void     save_param(void)
{
    char     name[256];
    ARParam  param;
    int      i, j;

    param.xsize = xsize;
    param.ysize = ysize;
    for( i = 0; i < 4; i++ ) param.dist_factor[i] = dist_factor[i];
    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            param.mat[j][i] = mat[j][i];
        }
    }
    arParamDisp( &param );

    printf("Filename: ");
    scanf( "%s", name );
    arParamSave( name, 1, &param );

    return;
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
    init( argc, argv );

    glutKeyboardFunc(keyEvent);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(motionEvent);
    glutIdleFunc(dispImage);
    glutDisplayFunc(dispImage);

    print_comment(0);
    status = 0;
    point_num = 0;
    arVideoCapStart();
    glutMainLoop();
	return (0);
}

static void init(int argc, char *argv[])
{
    double  length;
    char    line[512];
    int     i, j;

    patt.h_num    = H_NUM;
    patt.v_num    = V_NUM;
    patt.loop_num = 0;
    if (patt.h_num < 3 || patt.v_num < 3) exit(0);

    printf("Input the length between each markers: ");
    scanf("%lf", &length);
	while (getchar() != '\n');
    patt.world_coord = (CALIB_COORD_T *)malloc(sizeof(CALIB_COORD_T) * patt.h_num * patt.v_num);
    for (j = 0; j < patt.v_num; j++) {
        for (i = 0; i < patt.h_num; i++) {
            patt.world_coord[j*patt.h_num+i].x_coord = length * i;
            patt.world_coord[j*patt.h_num+i].y_coord = length * j;
        }
    }

    strcpy(line, vconf);
    for (i = 1; i < argc; i++) {
        strcat(line, " ");
        strcat(line, argv[i]);
    }
    if (arVideoOpen(line) < 0) exit(0);
    if (arVideoInqSize(&xsize, &ysize) < 0) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(xsize, ysize);
    glutInitWindowPosition(100,100);
    win = glutCreateWindow("Calibrate distortion parameter");

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xsize, 0, ysize, -1.0, 1.0);
    glViewport(0, 0, xsize, ysize);

	arMalloc(clipImage, unsigned char, xsize * ysize * AR_PIX_SIZE);

#ifdef USE_TEXMAP
    glGenTextures(2, glid);
    glBindTexture( GL_TEXTURE_2D, glid[0] );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    glBindTexture( GL_TEXTURE_2D, glid[1] );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    if( xsize > 512 ) {
        tex1Xsize1 = 512;
        tex1Xsize2 = 1;
        while( tex1Xsize2 < xsize - tex1Xsize1 ) tex1Xsize2 *= 2;
    }
    else {
        tex1Xsize1 = 1;
        while( tex1Xsize1 < xsize ) tex1Xsize1 *= 2;
    }
    tex1Ysize  = 1;
    while( tex1Ysize < ysize ) tex1Ysize *= 2;
#endif
}

static void motionEvent( int x, int y )
{
    unsigned char   *p, *p1;
    int             ssx, ssy, eex, eey;
    int             i, j, k;

    if( status == 1 && sx != -1 && sy != -1 ) {
        ex = x;
        ey = y;

        if( sx < ex ) { ssx = sx; eex = ex; }
         else         { ssx = ex; eex = sx; }
        if( sy < ey ) { ssy = sy; eey = ey; }
         else         { ssy = ey; eey = sy; }
        p1 = clipImage;
        for( j = ssy; j <= eey; j++ ) {
            p = &(patt.savedImage[patt.loop_num-1][(j*xsize+ssx)*AR_PIX_SIZE]);
            for( i = ssx; i <= eex; i++ ) {
#if defined(AR_PIX_FORMAT_BGRA)
                k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_ABGR)
                k = (255*3 - (*(p+1) + *(p+2) + *(p+3))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+1) = *(p1+2) = *(p1+3) = k;
#elif defined(AR_PIX_FORMAT_ARGB)
                k = (255*3 - (*(p+1) + *(p+2) + *(p+3))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+1) = *(p1+2) = *(p1+3) = k;
#elif defined(AR_PIX_FORMAT_BGR)
                k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_RGBA)
                k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_RGB)
                k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_2vuy)
                k = *(p+1);
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+1) = k;
#elif defined(AR_PIX_FORMAT_yuvs)
                k = *(p+0);
                if( k < thresh ) k = 0;
                else k = 255;
                *(p1+0) = k;
#else
#  error Unknown pixel format defined in config.h
#endif
                p  += AR_PIX_SIZE;
                p1 += AR_PIX_SIZE;
            }
        }
    }
}

static void mouseEvent(int button, int state, int x, int y)
{
    unsigned char   *p, *p1;
    int             ssx, ssy, eex, eey;
    int             i, j, k;

    if( button == GLUT_RIGHT_BUTTON  && state == GLUT_UP ) {
        if( status == 0 ) {
            arVideoCapStop();
            arVideoClose();

            if( patt.loop_num > 0 ) {
                calc_distortion( &patt, xsize, ysize, dist_factor );
                printf("--------------\n");
                printf("Center X: %f\n", dist_factor[0]);
                printf("       Y: %f\n", dist_factor[1]);
                printf("Dist Factor: %f\n", dist_factor[2]);
                printf("Size Adjust: %f\n", dist_factor[3]);
                printf("--------------\n");
                status = 2;
                check_num = 0;
                print_comment(5);
            }
            else {
                glutDestroyWindow( win );
                exit(0);
            }
        }
        else if( status == 1 ) {
            if( patt.loop_num == 0 ) {printf("error!!\n"); exit(0);}
            patt.loop_num--;
            free( patt.point[patt.loop_num] );
            free( patt.savedImage[patt.loop_num] );
            status = 0;
            point_num = 0;
            arVideoCapStart();

            if( patt.loop_num == 0 ) print_comment(0);
             else                    print_comment(4);
        }
    }

    if( button == GLUT_LEFT_BUTTON  && state == GLUT_DOWN ) {
        if( status == 1 && point_num < patt.h_num*patt.v_num ) {
            sx = ex = x;
            sy = ey = y;

            p  = &(patt.savedImage[patt.loop_num-1][(y*xsize+x)*AR_PIX_SIZE]);
            p1 = &(clipImage[0]);
#if defined(AR_PIX_FORMAT_BGRA)
            k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
            if( k < thresh ) k = 0;
                else k = 255;
            *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_ABGR)
            k = (255*3 - (*(p+1) + *(p+2) + *(p+3))) / 3;
            if( k < thresh ) k = 0;
                else k = 255;
            *(p1+1) = *(p1+2) = *(p1+3) = k;
#elif defined(AR_PIX_FORMAT_ARGB)
            k = (255*3 - (*(p+1) + *(p+2) + *(p+3))) / 3;
            if( k < thresh ) k = 0;
			else k = 255;
            *(p1+1) = *(p1+2) = *(p1+3) = k;
#elif defined(AR_PIX_FORMAT_BGR)
            k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
            if( k < thresh ) k = 0;
                else k = 255;
            *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_RGBA)
            k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
            if( k < thresh ) k = 0;
                else k = 255;
            *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_RGB)
            k = (255*3 - (*(p+0) + *(p+1) + *(p+2))) / 3;
            if( k < thresh ) k = 0;
                else k = 255;
            *(p1+0) = *(p1+1) = *(p1+2) = k;
#elif defined(AR_PIX_FORMAT_2vuy)
            k = *(p+1);
            if( k < thresh ) k = 0;
			else k = 255;
            *(p1+1) = k;
#elif defined(AR_PIX_FORMAT_yuvs)
            k = *(p+0);
            if( k < thresh ) k = 0;
			else k = 255;
            *(p1+0) = k;
#else
#  error Unknown pixel format defined in config.h
#endif
        }
    }

    if( button == GLUT_LEFT_BUTTON  && state == GLUT_UP ) {
        if( status == 0 && patt.loop_num < LOOP_MAX ) {
            while( (p = (unsigned char *)arVideoGetImage()) == NULL ) {
                arUtilSleep(2);
            }
#ifdef USE_TEXMAP
            patt.savedImage[patt.loop_num] = (unsigned char *)malloc( xsize*tex1Ysize*AR_PIX_SIZE );
#else
            patt.savedImage[patt.loop_num] = (unsigned char *)malloc( xsize*ysize*AR_PIX_SIZE );
#endif
            if( patt.savedImage[patt.loop_num] == NULL ) exit(0);

            p1 = patt.savedImage[patt.loop_num];
            for(i=0;i<xsize*ysize*AR_PIX_SIZE;i++) *(p1++) = *(p++);
            arVideoCapStop();

            patt.point[patt.loop_num] = (CALIB_COORD_T *)malloc( sizeof(CALIB_COORD_T)*patt.h_num*patt.v_num );
            if( patt.point[patt.loop_num] == NULL ) exit(0);

            patt.loop_num++;
            status = 1;
            sx = sy = ex= ey = -1;

            print_comment(1);
        }
        else if( status == 1 && point_num == patt.h_num*patt.v_num ) {
            status = 0;
            point_num = 0;
            arVideoCapStart();

            printf("### No.%d ###\n", patt.loop_num);
            for( j = 0; j < patt.v_num; j++ ) {
                for( i = 0; i < patt.h_num; i++ ) {
                    printf("%2d, %2d: %6.2f, %6.2f\n", i+1, j+1,
                           patt.point[patt.loop_num-1][j*patt.h_num+i].x_coord,
                           patt.point[patt.loop_num-1][j*patt.h_num+i].y_coord);
                }
            }
            printf("\n\n");
            if( patt.loop_num < LOOP_MAX ) print_comment(4);
             else                          print_comment(6);
        }
        else if( status == 1 ) {
            if( sx < ex ) { ssx = sx; eex = ex; }
             else         { ssx = ex; eex = sx; }
            if( sy < ey ) { ssy = sy; eey = ey; }
             else         { ssy = ey; eey = sy; }

            patt.point[patt.loop_num-1][point_num].x_coord = 0.0;
            patt.point[patt.loop_num-1][point_num].y_coord = 0.0;
            p = clipImage;
            k = 0;
            for( j = 0; j < (eey-ssy+1); j++ ) {
                for( i = 0; i < (eex-ssx+1); i++ ) {
                    patt.point[patt.loop_num-1][point_num].x_coord += i * *(p+1);
                    patt.point[patt.loop_num-1][point_num].y_coord += j * *(p+1);
                    k += *(p+1);
                    p += AR_PIX_SIZE;
                }
            }
            if( k != 0 ) {
                patt.point[patt.loop_num-1][point_num].x_coord /= k;
                patt.point[patt.loop_num-1][point_num].y_coord /= k;
                patt.point[patt.loop_num-1][point_num].x_coord += ssx;
                patt.point[patt.loop_num-1][point_num].y_coord += ssy;
                point_num++;
            }
            sx = sy = ex= ey = -1;

            printf(" # %d/%d\n", point_num, patt.h_num*patt.v_num);
            if( point_num == patt.h_num*patt.v_num ) print_comment(2);
        }
        else if( status == 2 ) {
            check_num++;
            if( check_num == patt.loop_num ) {
                if(patt.loop_num >= 2) {
                    if( calc_inp(&patt, dist_factor, xsize, ysize, mat) < 0 ) {
                        printf("Calibration failed.\n");
                        exit(0);
                    }
                    save_param();
                }
                glutDestroyWindow( win );
                exit(0);
            }

            if( check_num+1 == patt.loop_num ) {
                printf("\nLeft Mouse Button: Next Step.\n");
            }
            else {
                printf("   %d/%d.\n", check_num+1, patt.loop_num);
            }
        }
    }
}

static void keyEvent(unsigned char key, int x, int y)
{
    if( key == 't' ) {
        printf("Enter new threshold value (now = %d): ", thresh);
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
    }

    if( key == '1' ) thresh -= 5;
    if( thresh < 0 ) thresh = 0;

    if( key == '2' ) thresh += 5;
    if( thresh > 255 ) thresh = 255;
}

static void dispImage(void)
{
    unsigned char  *dataPtr;
    double         x, y;
    int            ssx, eex, ssy, eey;
    int            i;

    if( status == 0 ) {
        if( (dataPtr = (unsigned char *)arVideoGetImage()) == NULL ) {
            arUtilSleep(2);
            return;
        }
        dispImage2( dataPtr );
        arVideoCapNext();
    }

    else if( status == 1 ) {
        dispImage2( patt.savedImage[patt.loop_num-1] );

        for( i = 0; i < point_num; i++ ) {
            x = patt.point[patt.loop_num-1][i].x_coord;
            y = patt.point[patt.loop_num-1][i].y_coord;
            glColor3f( 1.0, 0.0, 0.0 );
            glBegin(GL_LINES);
              glVertex2f( x-10, (ysize-1)-y );
              glVertex2f( x+10, (ysize-1)-y );
              glVertex2f( x, (ysize-1)-(y-10) );
              glVertex2f( x, (ysize-1)-(y+10) );
            glEnd();
        }

        if( sx != -1 && sy != -1 ) {
            if( sx < ex ) { ssx = sx; eex = ex; }
             else         { ssx = ex; eex = sx; }
            if( sy < ey ) { ssy = sy; eey = ey; }
             else         { ssy = ey; eey = sy; }
            dispClipImage( ssx, ysize-1-ssy, eex-ssx+1, eey-ssy+1, clipImage );
#if 0
            glColor3f( 0.0, 0.0, 1.0 );
            glBegin(GL_LINE_LOOP);
              glVertex2f( sx, (ysize-1)-sy );
              glVertex2f( ex, (ysize-1)-sy );
              glVertex2f( ex, (ysize-1)-ey );
              glVertex2f( sx, (ysize-1)-ey );
            glEnd();
#endif
        }
    }

    else if( status == 2 ) {
        dispImage2( patt.savedImage[check_num] );
        for( i = 0; i < patt.h_num*patt.v_num; i++ ) {
            x = patt.point[check_num][i].x_coord;
            y = patt.point[check_num][i].y_coord;
            glColor3f( 1.0, 0.0, 0.0 );
            glBegin(GL_LINES);
              glVertex2f( x-10, (ysize-1)-y );
              glVertex2f( x+10, (ysize-1)-y );
              glVertex2f( x, (ysize-1)-(y-10) );
              glVertex2f( x, (ysize-1)-(y+10) );
            glEnd();
        }
        draw_line();
    }

    glutSwapBuffers();
}

static void dispImage2( unsigned char *pimage )
{
#ifndef USE_TEXMAP
    float      sx, sy;

    sx = 0;
    sy = ysize - 0.5;
    glPixelZoom( 1.0, -1.0);
    glRasterPos2f( sx, sy );

#if defined(AR_PIX_FORMAT_ARGB)
    glDrawPixels( xsize, ysize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pimage );
#elif defined(AR_PIX_FORMAT_ABGR)
    glDrawPixels( xsize, ysize, GL_ABGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGRA)
    glDrawPixels( xsize, ysize, GL_BGRA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGR)
    glDrawPixels( xsize, ysize, GL_BGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGB)
    glDrawPixels( xsize, ysize, GL_RGB, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGBA)
    glDrawPixels( xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_2vuy)
    glDrawPixels( xsize, ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, pimage );
#elif defined(AR_PIX_FORMAT_yuvs)
    glDrawPixels( xsize, ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pimage );
#else
#  error Unknown pixel format defined in config.h
#endif
#else
    glDisable( GL_DEPTH_TEST );
    if( xsize > tex1Xsize1 ) dispImageTex1( pimage );
     else                    dispImageTex2( pimage );
#endif
}

#ifdef USE_TEXMAP
static void dispImageTex1( unsigned char *pimage )
{
    float    sx, sy, ex, ey, z;
    float    tx, ty;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glPixelStorei( GL_UNPACK_ROW_LENGTH, xsize );

    glBindTexture( GL_TEXTURE_2D, glid[0] );
#if defined(AR_PIX_FORMAT_ARGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pimage );
#elif defined(AR_PIX_FORMAT_ABGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_ABGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGRA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGBA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_2vuy)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, pimage );
#elif defined(AR_PIX_FORMAT_yuvs)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pimage );
#else
#  error Unknown pixel format defined in config.h
#endif
    tx = 1.0;
    ty = (double)ysize / (double)tex1Ysize;
    sx = 0;
    sy = ysize;
    ex = sx + tex1Xsize1;
    ey = sy - ysize;
    z = 1.0;
    glBegin(GL_QUADS );
      glTexCoord2f( 0.0, 0.0 ); glVertex3f( sx, sy, z );
      glTexCoord2f(  tx, 0.0 ); glVertex3f( ex, sy, z );
      glTexCoord2f(  tx,  ty ); glVertex3f( ex, ey, z );
      glTexCoord2f( 0.0,  ty ); glVertex3f( sx, ey, z );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, glid[1] );
#if defined(AR_PIX_FORMAT_ARGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_ABGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_ABGR, GL_UNSIGNED_BYTE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_BGRA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_BYTE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_BGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_BGR, GL_UNSIGNED_BYTE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_RGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_RGBA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_2vuy)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, pimage+tex1Xsize1*AR_PIX_SIZE );
#elif defined(AR_PIX_FORMAT_yuvs)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize2, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pimage+tex1Xsize1*AR_PIX_SIZE );
#else
#  error Unknown pixel format defined in config.h
#endif
    tx = (double)(xsize-tex1Xsize1) / (double)tex1Xsize2;
    ty = (double)ysize / (double)tex1Ysize;
    sx = tex1Xsize1;
    sy = ysize;
    ex = sx + tex1Xsize2;
    ey = sy - ysize;
    z = 1.0;
    glBegin(GL_QUADS );
      glTexCoord2f( 0.0, 0.0 ); glVertex3f( sx, sy, z );
      glTexCoord2f(  tx, 0.0 ); glVertex3f( ex, sy, z );
      glTexCoord2f(  tx,  ty ); glVertex3f( ex, ey, z );
      glTexCoord2f( 0.0,  ty ); glVertex3f( sx, ey, z );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}

static void dispImageTex2( unsigned char *pimage )
{
    float    sx, sy, ex, ey, z;
    float    tx, ty;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glPixelStorei( GL_UNPACK_ROW_LENGTH, xsize );

    glBindTexture( GL_TEXTURE_2D, glid[0] );
#if defined(AR_PIX_FORMAT_ARGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pimage );
#elif defined(AR_PIX_FORMAT_ABGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_ABGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGRA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGRA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGR)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_BGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGB)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGBA)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_2vuy)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, pimage );
#elif defined(AR_PIX_FORMAT_yuvs)
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1Xsize1, tex1Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pimage );
#else
#  error Unknown pixel format defined in config.h
#endif
    tx = (double)xsize / (double)tex1Xsize1;
    ty = (double)ysize / (double)tex1Ysize;
    sx = 0;
    sy = ysize;
    ex = sx + xsize;
    ey = sy - ysize;
    z = 1.0;
    glBegin(GL_QUADS );
      glTexCoord2f( 0.0, 0.0 ); glVertex3f( sx, sy, z );
      glTexCoord2f(  tx, 0.0 ); glVertex3f( ex, sy, z );
      glTexCoord2f(  tx,  ty ); glVertex3f( ex, ey, z );
      glTexCoord2f( 0.0,  ty ); glVertex3f( sx, ey, z );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}
#endif

static void draw_line(void)
{
    double   *x, *y;
    int      max;
    // int      num; // unreferenced
    int      i, j, k, l;
    int      p;

    max = (patt.v_num > patt.h_num)? patt.v_num: patt.h_num;
    x = (double *)malloc( sizeof(double)*max );
    y = (double *)malloc( sizeof(double)*max );
    if( x == NULL || y == NULL ) exit(0);

    i = check_num;

    for( j = 0; j < patt.v_num; j++ ) {
        for( k = 0; k < patt.h_num; k++ ) {
            x[k] = patt.point[i][j*patt.h_num+k].x_coord;
            y[k] = patt.point[i][j*patt.h_num+k].y_coord;
        }
        draw_line2( x, y, patt.h_num );
    }

    for( j = 0; j < patt.h_num; j++ ) {
        for( k = 0; k < patt.v_num; k++ ) {
            x[k] = patt.point[i][k*patt.h_num+j].x_coord;
            y[k] = patt.point[i][k*patt.h_num+j].y_coord;
        }
        draw_line2( x, y, patt.v_num );
    }

    for( j = 3 - patt.v_num; j < patt.h_num - 2; j++ ) {
        p = 0;
        for( k = 0; k < patt.v_num; k++ ) {
            l = j+k;
            if( l < 0 || l >= patt.h_num ) continue;
            x[p] = patt.point[i][k*patt.h_num+l].x_coord;
            y[p] = patt.point[i][k*patt.h_num+l].y_coord;
            p++;
        }
        draw_line2( x, y, p );
    }

    for( j = 2; j < patt.h_num + patt.v_num - 3; j++ ) {
        p = 0;
        for( k = 0; k < patt.v_num; k++ ) {
            l = j-k;
            if( l < 0 || l >= patt.h_num ) continue;
            x[p] = patt.point[i][k*patt.h_num+l].x_coord;
            y[p] = patt.point[i][k*patt.h_num+l].y_coord;
            p++;
        }
        draw_line2( x, y, p );
    }

    free( x );
    free( y );
}

static void draw_line2( double *x, double *y, int num )
{
    ARMat    *input, *evec;
    ARVec    *ev, *mean;
    double   a, b, c;
    int      i;

    ev     = arVecAlloc( 2 );
    mean   = arVecAlloc( 2 );
    evec   = arMatrixAlloc( 2, 2 );

    input  = arMatrixAlloc( num, 2 );
    for( i = 0; i < num; i++ ) {
        arParamObserv2Ideal( dist_factor, x[i], y[i],
                             &(input->m[i*2+0]), &(input->m[i*2+1]) );
    }
    if( arMatrixPCA(input, evec, ev, mean) < 0 ) exit(0);
    a =  evec->m[1];
    b = -evec->m[0];
    c = -(a*mean->v[0] + b*mean->v[1]);

    arMatrixFree( input );
    arMatrixFree( evec );
    arVecFree( mean );
    arVecFree( ev );

    draw_warp_line(a, b, c);
}

static void draw_warp_line( double a, double b , double c )
{
    double   x, y;
    double   x1, y1;
    // double   d; // unreferenced
    int      i;

    glLineWidth( 1.0 );
    glBegin(GL_LINE_STRIP);
    if( a*a >= b*b ) {
        for( i = -20; i <= ysize+20; i+=10 ) {
            x = -(b*i + c)/a;
            y = i;

            arParamIdeal2Observ( dist_factor, x, y, &x1, &y1 );
            glVertex2f( x1, ysize-1-y1 );
        }
    }
    else {
        for( i = -20; i <= xsize+20; i+=10 ) {
            x = i;
            y = -(a*i + c)/b;

            arParamIdeal2Observ( dist_factor, x, y, &x1, &y1 );
            glVertex2f( x1, ysize-1-y1 );
        }
    }
    glEnd();
}

static void     print_comment( int status )
{
    printf("\n-----------\n");
    switch( status ) {
        case 0:
           printf("Mouse Button\n");
           printf(" Left   : Grab image.\n");
           printf(" Right  : Quit.\n");
           break;
        case 1:
           printf("Mouse Button\n");
           printf(" Left   : Rubber-bounding of feature. (%d x %d)\n", patt.h_num, patt.v_num);
           printf(" Right  : Cansel rubber-bounding & Retry grabbing.\n");
           break;
        case 2:
           printf("Mouse Button\n");
           printf(" Left   : Save feature position.\n");
           printf(" Right  : Discard & Retry grabbing.\n");
           break;
        case 4:
           printf("Mouse Button\n");
           printf(" Left   : Grab next image.\n");
           printf(" Right  : Calc parameter.\n");
           break;
        case 5:
           printf("Mouse Button\n");
           printf(" Left   : Check fittness.\n");
           printf(" Right  :\n");
           printf("   %d/%d.\n", check_num+1, patt.loop_num);
           break;
        case 6:
           printf("Mouse Button\n");
           printf(" Left   :\n");
           printf(" Right  : Calc parameter.\n");
           printf("   %d/%d.\n", check_num+1, patt.loop_num);
           break;
    }
    printf("-----------\n");
}

static void dispClipImage( int sx, int sy, int xsize, int ysize, ARUint8 *clipImage )
{
    float   ssx, ssy;

    ssx = sx;
    ssy = sy + 0.5;
    glPixelZoom( (GLfloat)1.0, (GLfloat)-1.0);
    glRasterPos2f( ssx, ssy );
#if defined(AR_PIX_FORMAT_ARGB)
    glDrawPixels( xsize, ysize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, clipImage );
#elif defined(AR_PIX_FORMAT_ABGR)
    glDrawPixels( xsize, ysize, GL_ABGR, GL_UNSIGNED_BYTE, clipImage );
#elif defined(AR_PIX_FORMAT_BGRA)
    glDrawPixels( xsize, ysize, GL_BGRA, GL_UNSIGNED_BYTE, clipImage );
#elif defined(AR_PIX_FORMAT_BGR)
    glDrawPixels( xsize, ysize, GL_BGR, GL_UNSIGNED_BYTE, clipImage );
#elif defined(AR_PIX_FORMAT_RGB)
    glDrawPixels( xsize, ysize, GL_RGB, GL_UNSIGNED_BYTE, clipImage );
#elif defined(AR_PIX_FORMAT_RGBA)
    glDrawPixels( xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, clipImage );
#elif defined(AR_PIX_FORMAT_2vuy)
    glDrawPixels( xsize, ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, clipImage );
#elif defined(AR_PIX_FORMAT_yuvs)
    glDrawPixels( xsize, ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, clipImage );
#else
#  error Unknown pixel format defined in config.h
#endif
}

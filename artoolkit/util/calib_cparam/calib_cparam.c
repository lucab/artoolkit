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
//	Includes
// ============================================================================

#ifdef _WIN32
#  include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include <AR/ar.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/gsub_lite.h>
#include "calib_cparam.h"

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


ARParam         param;
double          dist_factor[4];

/* set up the video format globals */

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
char			*vconf = "Data\\WDM_camera_flipV.xml";
#elif defined(__APPLE__)
char			*vconf = "-width=640 -height=480";
#else
char			*vconf = "";
#endif

int             gXsize;
int             gYsize;
unsigned char   *gImage;
int             refresh;


int             line_num;
int             loop_num;
int             line_mode[LINE_MAX];
double          inter_coord[LOOP_MAX][LINE_MAX][LINE_MAX][3];
double          line_info[LOOP_MAX][LINE_MAX][4];

int             win;
int             mode;
int             line_no;
int             loop_no;
double          theta;
double          radius;
double          sx, sy, ex, ey;

#ifdef _WIN32
static int      drawMode = AR_DRAW_BY_GL_DRAW_PIXELS;
#else
static int      drawMode = DEFAULT_DRAW_MODE;
#endif
static GLuint   glid[2];
static int      tex1Xsize1 = 1;
static int      tex1Xsize2 = 1;
static int      tex1Ysize  = 1;
static void dispImageTex1( unsigned char *pimage );
static void dispImageTex2( unsigned char *pimage );

static void   init(void);
static void   keyboard(unsigned char key, int x, int y);
static void   special(int key, int x, int y);
static void   mouse(int button, int state, int x, int y);
static void   dispImage(void);
static void   drawPrevLine(void);
static void   drawNextLine(void);
static void   draw_warp_line( double sx, double ex, double sy, double ey );
static void   getCpara(void);
static void   intersection( double line1[4], double line2[4], double *screen_coord );
static void   dispImage2( ARUint8 *image );


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
    init();

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutIdleFunc(dispImage);
    glutDisplayFunc(dispImage);

    mode = 0;
    line_no = 0;
    loop_no = 0;
    arVideoCapStart();
    glutMainLoop();
	return (0);
}

static void init(void)
{
    printf("Input center coordinates: X = ");
    scanf("%lf", &dist_factor[0]);
    while( getchar()!='\n' );
    printf("                        : Y = ");
    scanf("%lf", &dist_factor[1]);
    while( getchar()!='\n' );
    printf("Input distortion ratio: F = ");
    scanf("%lf", &dist_factor[2]);
    while( getchar()!='\n' );
    printf("Input size adjustment factor: S = ");
    scanf("%lf", &dist_factor[3]);
    while( getchar()!='\n' );

    initLineModel( &line_num, &loop_num, line_mode, inter_coord );

    if( arVideoOpen( vconf ) < 0 ) exit(0);
    if( arVideoInqSize(&gXsize, &gYsize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", gXsize, gYsize);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(gXsize, gYsize);
    glutInitWindowPosition(100,100);
    win = glutCreateWindow("Camera calibration");

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gXsize, 0, gYsize, -1.0, 1.0);
    glViewport(0, 0, gXsize, gYsize);

    glPixelZoom( (GLfloat)1.0, (GLfloat)-1.0);
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT);

    param.xsize = gXsize;
    param.ysize = gYsize;
    param.dist_factor[0] = dist_factor[0];
    param.dist_factor[1] = dist_factor[1];
    param.dist_factor[2] = dist_factor[2];
    param.dist_factor[3] = dist_factor[3];

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
    if( gXsize > 512 ) {
        tex1Xsize1 = 512;
        tex1Xsize2 = 1;
        while( tex1Xsize2 < gXsize - tex1Xsize1 ) tex1Xsize2 *= 2;
    }
    else {
        tex1Xsize1 = 1;
        while( tex1Xsize1 < gXsize ) tex1Xsize1 *= 2;
    }
    tex1Ysize  = 1;
    while( tex1Ysize < gYsize ) tex1Ysize *= 2;

    gImage = (unsigned char *)malloc( gXsize*tex1Ysize*AR_PIX_SIZE );
    if( gImage == NULL ) {printf("malloc error!!\n"); exit(0);}
/*
    gImage = (unsigned char *)malloc( gXsize*gYsize*AR_PIX_SIZE );
    if( gImage == NULL ) {printf("malloc error!!\n"); exit(0);}
*/
}

static void mouse(int button, int state, int x, int y)
{
    if( button == GLUT_RIGHT_BUTTON  && state == GLUT_DOWN ) {
        if( mode == 0 ) arVideoCapStop();
        arVideoClose();
        glutDestroyWindow( win );
        exit(0);
    }

    if( button == GLUT_LEFT_BUTTON  && state == GLUT_DOWN && mode == 0 ) {
        arVideoCapStop();
        mode = 1;
        refresh = 1;
        line_no = 0;
        if( line_mode[line_no] == L_HORIZONTAL ) {
            theta  = 90;
            radius = gYsize/2;
        }
        else {
            theta  = 0;
            radius = gXsize/2;
        }
    }
}

static void keyboard(unsigned char key, int x, int y)
{
    ARParam  iparam;
    double   trans[3][4];
    char     name[256];
    int      k = 1;

    if( mode == 0 ) return;

    switch( key ) {
        case 0x1B:
            sx = -1.0;
            sy = -1.0;
            ex = -1.0;
            ey = -1.0;
        case 0x0D:
            line_info[loop_no][line_no][0] = sx;
            line_info[loop_no][line_no][1] = sy;
            line_info[loop_no][line_no][2] = ex;
            line_info[loop_no][line_no][3] = ey;
            line_no++;

            if( line_no != line_num ) {
                if( line_mode[line_no] == L_HORIZONTAL ) {
                    if( line_mode[line_no] == line_mode[line_no-1] ) {
                        radius += 10.0;
                    }
                    else {
                        theta  = 90;
                        radius = gYsize/2;
                    }
                }
                else {
                    if( line_mode[line_no] == line_mode[line_no-1] ) {
                        radius += 10.0;
                    }
                    else {
                        theta  = 0;
                        radius = gXsize/2;
                    }
                }
            }
            else {
                loop_no++;
                if( loop_no != loop_num ) {
                    arVideoCapStart();
                    mode = 0;
                    return;
                }
                arVideoClose();
                glutDestroyWindow( win );

                getCpara();
                arParamDecomp( &param, &iparam, trans );
                arParamDisp( &iparam );

                printf("Input filename: ");
                scanf("%s", name);
                arParamSave( name, 1, &iparam );

                exit(0);
            }
            break;
        default:
            k = 0;
            break;
    }
    if( k ) refresh = 1;
}


static void special(int key, int x, int y)
{
    double   mx, my;
    int      k = 1;

    if (mode == 0) return;

    if (line_mode[line_no] == L_HORIZONTAL) {
        switch (key) {
            case GLUT_KEY_UP:
                radius -= 1.0;
                break;
            case GLUT_KEY_DOWN:
                radius += 1.0;
                break;
            case GLUT_KEY_LEFT:
                mx = (sx + ex)/ 2.0;
                my = (sy + ey)/ 2.0;
                theta -= 0.25;
                radius = cos( (double)(theta*3.141592/180.0) ) * mx
                       + sin( (double)(theta*3.141592/180.0) ) * my;
                break;
            case GLUT_KEY_RIGHT:
                mx = (sx + ex)/ 2.0;
                my = (sy + ey)/ 2.0;
                theta += 0.25;
                radius = cos( (double)(theta*3.141592/180.0) ) * mx
                       + sin( (double)(theta*3.141592/180.0) ) * my;
                break;
            default:
                k = 0;
                break;
        }
    } else {
        switch (key) {
            case GLUT_KEY_UP:
                mx = (sx + ex)/ 2.0;
                my = (sy + ey)/ 2.0;
                theta += 0.25;
                radius = cos( (double)(theta*3.141592/180.0) ) * mx
                       + sin( (double)(theta*3.141592/180.0) ) * my;
                break;
            case GLUT_KEY_DOWN:
                mx = (sx + ex)/ 2.0;
                my = (sy + ey)/ 2.0;
                theta -= 0.25;
                radius = cos( (double)(theta*3.141592/180.0) ) * mx
                       + sin( (double)(theta*3.141592/180.0) ) * my;
                break;
            case GLUT_KEY_LEFT:
                radius -= 1.0;
                break;
            case GLUT_KEY_RIGHT:
                radius += 1.0;
                break;
            default:
                k = 0;
                break;
        }
    }

    if (k) refresh = 1;
}

static void dispImage( void )
{
    unsigned char *dataPtr;
    unsigned char *p1, *p2;
    int           i;

    if( mode == 0 ) {
        if( (dataPtr = (unsigned char *)arVideoGetImage()) == NULL ) {
            arUtilSleep(2);
            return;
        }
        p1 = dataPtr;
        p2 = gImage;
        for( i = 0; i < gXsize*gYsize*AR_PIX_SIZE; i++ ) *(p2++) = *(p1++);
        arVideoCapNext();

        glClearColor( 0.0, 0.0, 0.0, 0.0 );
        glClear(GL_COLOR_BUFFER_BIT);
        dispImage2( gImage );
        glutSwapBuffers();
    }
    else {
        if( refresh == 0 ) {
            arUtilSleep(10);
            return;
        }
        refresh = 0;

        glClearColor( 0.0, 0.0, 0.0, 0.0 );
        glClear(GL_COLOR_BUFFER_BIT);
        dispImage2( gImage );
        drawPrevLine();
        drawNextLine();
        glutSwapBuffers();
    }
}

static void drawPrevLine( void )
{
    int     i;

    glColor3f(0.0,0.0,1.0);
    for( i = 0; i < line_no; i++ ) {
        if( line_info[loop_no][i][0] == -1.0 ) continue;
        draw_warp_line( line_info[loop_no][i][0], line_info[loop_no][i][2],
                        line_info[loop_no][i][1], line_info[loop_no][i][3] );
    }
}

static void drawNextLine(void)
{
    double sin_theta, cos_theta;
    double x1, x2;
    double y1, y2;

    sin_theta = sin( (double)(theta*3.141592/180.0) );
    cos_theta = cos( (double)(theta*3.141592/180.0) );

    if( cos_theta != 0 ) {
        x1 = radius / cos_theta;
        x2 = (radius - (gYsize-1)*sin_theta) / cos_theta;
    }
    else {
        x1 = x2 = -1.0;
    }

    if( sin_theta != 0 ) {
        y1 = radius / sin_theta;
        y2 = (radius - (gXsize-1)*cos_theta) / sin_theta;
    }
    else {
        y1 = y2 = -1.0;
    }

    ey = -1;
    if( x1 >= 0 && x1 <= gXsize-1 ) {
         sx = x1;
         sy = 0;
         if( x2 >= 0 && x2 <= gXsize-1 ) {
             ex = x2;
             ey = gYsize-1;
         }
         else if( y1 >= 0 && y1 <= gYsize-1 ) {
             ex = 0;
             ey = y1;
         }
         else if( y2 >= 0 && y2 <= gYsize-1 ) {
             ex = gXsize-1;
             ey = y2;
         }
         else printf("???\n");
    }
    else if( y1 >= 0 && y1 <= gYsize-1 ) {
         sx = 0;
         sy = y1;
         if( x2 >= 0 && x2 <= gXsize-1 ) {
             ex = x2;
             ey = gYsize-1;
         }
         else if( y2 >= 0 && y2 <= gYsize-1 ) {
             ex = gXsize-1;
             ey = y2;
         }
         else printf("???\n");
    }
    else if( x2 >= 0 && x2 <= gXsize-1 ) {
         sx = x2;
         sy = gYsize-1;
         if( y2 >= 0 && y2 <= gYsize-1 ) {
             ex = gXsize-1;
             ey = y2;
         }
         else printf("???\n");
    }

    glColor3f(1.0,1.0,1.0);
    if( ey != -1 ) {
        draw_warp_line( sx, ex, sy, ey );
    }
}

static void draw_warp_line( double sx, double ex, double sy, double ey )
{
    double   a, b, c;
    double   x, y;
    double   x1, y1;
    int      i;

    a = ey - sy;
    b = sx - ex;
    c = sy*ex - sx*ey;

    glLineWidth( 1.0 );
    glBegin(GL_LINE_STRIP);
    if( a*a >= b*b ) {
        for( i = -20; i <= gYsize+20; i+=10 ) {
            x = -(b*i + c)/a;
            y = i;

            arParamIdeal2Observ( dist_factor, x, y, &x1, &y1 );
            glVertex2f( x1, gYsize-1-y1 );
        }
    }
    else {
        for( i = -20; i <= gXsize+20; i+=10 ) {
            x = i;
            y = -(a*i + c)/b;

            arParamIdeal2Observ( dist_factor, x, y, &x1, &y1 );
            glVertex2f( x1, gYsize-1-y1 );
        }
    }
    glEnd();
}



static void getCpara(void)
{
    double  *world_coord;
    double  *screen_coord;
    int     point_num;
    int     i, j, k;

    point_num = 0;
    for( k = 0; k < loop_num; k++ ) {
        for( i = 0; i < line_num; i++ ) {
            for( j = 0; j < line_num; j++ ) {
                if( inter_coord[k][i][j][0] != -10000.0 ) point_num++;
            }
        }
    }
    world_coord = (double *)malloc( point_num * 3 * sizeof(double) );
    if( world_coord == NULL ) {printf("malloc error!!\n"); exit(0);}
    screen_coord = (double *)malloc( point_num * 2 * sizeof(double) );
    if( screen_coord == NULL ) {printf("malloc error!!\n"); exit(0);}

    point_num = 0;
    for( k = 0; k < loop_num; k++ ) {
        for( i = 0; i < line_num; i++ ) {
            if( line_info[k][i][0] == -1.0 ) continue;
            for( j = 0; j < line_num; j++ ) {
                if( line_info[k][j][0] == -1.0 ) continue;
                if( inter_coord[k][i][j][0] == -10000.0 ) continue;

                world_coord[point_num*3+0] = inter_coord[k][i][j][0];
                world_coord[point_num*3+1] = inter_coord[k][i][j][1];
                world_coord[point_num*3+2] = inter_coord[k][i][j][2];
                intersection( line_info[k][i], line_info[k][j],
                              &(screen_coord[point_num*2]) );

                point_num++;
            }
        }
    }
    printf("point_num = %d\n", point_num);
    if( arParamGet((double (*)[3])world_coord, (double (*)[2])screen_coord, point_num, param.mat) < 0 ) {
        printf("ddd error!!\n");
        exit(0);
    }

    free( world_coord );
    free( screen_coord );
}

static void intersection( double line1[4], double line2[4], double *screen_coord )
{
    double a, b, c, d, e, f, g;

    a = line1[1] - line1[3];
    b = line1[2] - line1[0];
    c = line1[0] * a + line1[1] * b;
    d = line2[1] - line2[3];
    e = line2[2] - line2[0];
    f = line2[0] * d + line2[1] * e;

    g = a*e - b*d;
    if( g == 0.0 ) { printf("???\n"); exit(0); }

    screen_coord[0] = (c * e - b * f) / g;
    screen_coord[1] = (a * f - c * d) / g;
}

static void dispImage2( ARUint8 *pimage )
{
    if( drawMode == AR_DRAW_BY_GL_DRAW_PIXELS ) {
        float    sx, sy;

        sx = 0;
        sy = gYsize - 0.5;
        glPixelZoom( 1.0, -1.0);
        glRasterPos3i( sx, sy, 0 );

#if defined(AR_PIX_FORMAT_ARGB)
        glDrawPixels( gXsize, gYsize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pimage );
#elif defined(AR_PIX_FORMAT_ABGR)
        glDrawPixels( gXsize, gYsize, GL_ABGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGRA)
        glDrawPixels( gXsize, gYsize, GL_BGRA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_BGR)
        glDrawPixels( gXsize, gYsize, GL_BGR, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGB)
        glDrawPixels( gXsize, gYsize, GL_RGB, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_RGBA)
        glDrawPixels( gXsize, gYsize, GL_RGBA, GL_UNSIGNED_BYTE, pimage );
#elif defined(AR_PIX_FORMAT_2vuy)
		glDrawPixels( gXsize, gYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, pimage );
#elif defined(AR_PIX_FORMAT_yuvs)
		glDrawPixels( gXsize, gYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pimage );
#else
#  error Unknown pixel format defined in config.h
#endif
    }
    else {
        glDisable( GL_DEPTH_TEST );
        if( gXsize > tex1Xsize1 ) dispImageTex1( pimage );
         else                    dispImageTex2( pimage );
    }
}

static void dispImageTex1( unsigned char *pimage )
{
    float    sx, sy, ex, ey, z;
    float    tx, ty;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glPixelStorei( GL_UNPACK_ROW_LENGTH, gXsize );

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
    ty = (double)gYsize / (double)tex1Ysize;
    sx = 0;
    sy = gYsize;
    ex = sx + tex1Xsize1;
    ey = sy - gYsize;
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
    tx = (double)(gXsize-tex1Xsize1) / (double)tex1Xsize2;
    ty = (double)gYsize / (double)tex1Ysize;
    sx = tex1Xsize1;
    sy = gYsize;
    ex = sx + tex1Xsize2;
    ey = sy - gYsize;
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

    glPixelStorei( GL_UNPACK_ROW_LENGTH, gXsize );

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
    tx = (double)gXsize / (double)tex1Xsize1;
    ty = (double)gYsize / (double)tex1Ysize;
    sx = 0;
    sy = gYsize;
    ex = sx + gXsize;
    ey = sy - gYsize;
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

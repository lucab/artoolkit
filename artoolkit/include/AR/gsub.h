#ifndef AR_GSUB_H
#define AR_GSUB_H
#ifdef __cplusplus
extern "C" {
#endif


#include <AR/config.h>
#include <AR/param.h>
#include <AR/ar.h>

extern int  argDrawMode;
extern int  argTexmapMode;

void argInit( ARParam *cparam, double zoom, int fullFlag, int xwin, int ywin, int hmd_flag );
void argLoadHMDparam( ARParam *lparam, ARParam *rparam );
void argCleanup( void );
void argSwapBuffers( void );

void argMainLoop( void (*mouseFunc)(int button, int state, int x, int y),
                  void (*keyFunc)(unsigned char key, int x, int y),
                  void (*mainFunc)(void) );


void argDrawMode2D( void );
void argDraw2dLeft( void );
void argDraw2dRight( void );
void argDrawMode3D( void );
void argDraw3dLeft( void );
void argDraw3dRight( void );
void argDraw3dCamera( int xwin, int ywin );


void argConvGlpara( double para[3][4], double gl_para[16] );
void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] );

void argDispImage( ARUint8 *image, int xwin, int ywin );
void argDispHalfImage( ARUint8 *image, int xwin, int ywin );

void argDrawSquare( double vertex[4][2], int xwin, int ywin );
void argLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin );
void argLineSegHMD( double x1, double y1, double x2, double y2 );

void argInqSetting( int *hmdMode, 
                    int *gMiniXnum2, int *gMiniYnum2,
                    void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y),
                    void (**mainFunc)(void) );


/*-------------------------*/

void argsInit( ARSParam *scparam, double zoom, int twinFlag, int fullFlag, int xwin, int ywin );
void argsDraw3dCamera( int xwin, int ywin, int LorR, int stencil_flag );
void argsConvGLcpara( ARSParam *sparam, double gnear, double gfar, double mL[16], double mR[16] );
void argsDispImage( ARUint8 *image, int LorR, int xwin, int ywin );
void argsDispHalfImage( ARUint8 *image, int LorR, int xwin, int ywin );
void argsLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin, int LorR );
void argsDrawSquare( double  vertex[4][2], int xwin, int ywin, int LorR );


#ifdef __cplusplus
}
#endif
#endif

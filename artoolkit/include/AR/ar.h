/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 3.1
 * Date: 01/12/07
 *
*******************************************************/

#ifndef AR_H
#define AR_H
#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include <AR/config.h>
#include <AR/param.h>

#define arMalloc(V,T,S)  \
{ if( ((V) = (T *)malloc( sizeof(T) * (S) )) == 0 ) \
{printf("malloc error!!\n"); exit(1);} }

typedef char              ARInt8;
typedef short             ARInt16;
typedef int               ARInt32;
typedef unsigned char     ARUint8;
typedef unsigned short    ARUint16;
typedef unsigned int      ARUint32;


typedef struct {
    int     area;
    int     id;
    int     dir;
    double  cf;
    double  pos[2];
    double  line[4][3];
    double  vertex[4][2];
} ARMarkerInfo;

typedef struct {
    int     area;
    double  pos[2];
    int     coord_num;
    int     x_coord[AR_CHAIN_MAX];
    int     y_coord[AR_CHAIN_MAX];
    int     vertex[5];
} ARMarkerInfo2;


extern int      arDebug;
extern ARUint8  *arImage;
extern int      arFittingMode;
extern int      arImageProcMode;
extern ARParam  arParam;
extern int      arImXsize, arImYsize;
extern int      arTemplateMatchingMode;
extern int      arMatchingPCAMode;


/*
   For initializing
*/

int arInitCparam( ARParam *param );
int arLoadPatt( const char *filename );


/*
   For detecting
*/

int arDetectMarker( ARUint8 *dataPtr, int thresh,
                    ARMarkerInfo **marker_info, int *marker_num );

int arDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                        ARMarkerInfo **marker_info, int *marker_num );

double arGetTransMat( ARMarkerInfo *marker_info,
                      double center[2], double width, double conv[3][4] );

double arGetTransMatCont( ARMarkerInfo *marker_info, double prev_conv[3][4],
                          double center[2], double width, double conv[3][4] );

double arGetTransMat2( double rot[3][3], double pos2d[][2],
                       double pos3d[][2], int num, double conv[3][4] );
double arGetTransMat3( double rot[3][3], double ppos2d[][2],
                     double ppos3d[][2], int num, double conv[3][4],
                     double *dist_factor, double cpara[3][4] );
double arGetTransMat4( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4] );
double arGetTransMat5( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4],
                       double *dist_factor, double cpara[3][4] );


int arFreePatt( int patt_no );
int arActivatePatt( int pat_no );
int arDeactivatePatt( int pat_no );
int arSavePatt( ARUint8 *image,
                ARMarkerInfo *marker_info, char *filename );


/*
    Utility
*/

int    arUtilMatInv( double s[3][4], double d[3][4] );
int    arUtilMatMul( double s1[3][4], double s2[3][4], double d[3][4] );
int    arUtilMat2QuatPos( double m[3][4], double q[4], double p[3] );
int    arUtilQuatPos2Mat( double q[4], double p[3], double m[3][4] );

double arUtilTimer(void);
void   arUtilTimerReset(void);
void   arUtilSleep( int msec );


/*
   For internal processing
*/


ARInt16 *arLabeling( ARUint8 *image, int thresh,
                     int *label_num, int **area, double **pos, int **clip,
                     int **label_ref );
void arGetImgFeature( int *num, int **area, int **clip, double **pos );

ARMarkerInfo2 *arDetectMarker2( ARInt16 *limage,
                                int label_num, int *label_ref,
                                int *warea, double *wpos, int *wclip,
                                int area_max, int area_min, double factor, int *marker_num );

ARMarkerInfo *arGetMarkerInfo( ARUint8 *image,
                               ARMarkerInfo2 *marker_info2, int *marker_num );


int arGetCode( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               int *code, int *dir, double *cf );

int arGetPatt( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] );

int arGetLine(int x_coord[], int y_coord[], int coord_num,
              int vertex[], double line[4][3], double v[4][2]);

int arGetContour( ARInt16 *limage, int *label_ref,
                  int label, int clip[4], ARMarkerInfo2 *marker_info2 );

double arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4],
                             double vertex[][3], double pos2d[][2], int num );

int arGetAngle( double rot[3][3], double *wa, double *wb, double *wc );

int arGetRot( double a, double b, double c, double rot[3][3] );

int arGetNewMatrix( double a, double b, double c,
                    double trans[3], double trans2[3][4],
                    double cpara[3][4], double ret[3][4] );

int arGetInitRot( ARMarkerInfo *marker_info, double cpara[3][4], double rot[3][3] );

typedef struct {
    ARMarkerInfo  marker;
    int     count;
} arPrevInfo;


/*------------------------------------*/

extern ARUint8  *arImageL;
extern ARUint8  *arImageR;
extern ARSParam arsParam;
extern double   arsMatR2L[3][4];

int           arsInitCparam      ( ARSParam *sparam );
void          arsGetImgFeature   ( int *num, int **area, int **clip, double **pos, int LorR );
ARInt16      *arsLabeling        ( ARUint8 *image, int thresh,
                                   int *label_num, int **area, double **pos, int **clip,
                                   int **label_ref, int LorR );
int           arsGetLine         ( int x_coord[], int y_coord[], int coord_num,
                                   int vertex[], double line[4][3], double v[4][2], int LorR);
ARMarkerInfo *arsGetMarkerInfo   ( ARUint8 *image,
                                   ARMarkerInfo2 *marker_info2, int *marker_num, int LorR );
int           arsDetectMarker    ( ARUint8 *dataPtr, int thresh,
                                   ARMarkerInfo **marker_info, int *marker_num, int LorR );
int           arsDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                                   ARMarkerInfo **marker_info, int *marker_num, int LorR );
double        arsGetTransMat     ( ARMarkerInfo *marker_infoL, ARMarkerInfo *marker_infoR,
                                   double center[2], double width,
                                   double transL[3][4], double transR[3][4] );
double        arsGetTransMatCont ( ARMarkerInfo *marker_infoL, ARMarkerInfo *marker_infoR,
                                   double prev_conv[3][4],
                                   double center[2], double width,
                                   double transL[3][4], double transR[3][4] );
double        arsGetTransMat2    ( double rot[3][3],
                                   double ppos2dL[][2], double ppos3dL[][3], int numL,
                                   double ppos2dR[][2], double ppos3dR[][3], int numR,
                                   double transL[3][4], double transR[3][4] );
double        arsGetPosErr( double pos2dL[2], double pos2dR[2] );
int           arsCheckPosition   ( double pos2dL[2], double pos2dR[2], double thresh );
int           arsCheckMarkerPosition( ARMarkerInfo *marker_infoL, ARMarkerInfo *marker_infoR,
                                      double thresh );

double arsModifyMatrix( double rot[3][3], double trans[3], ARSParam *arsParam,
                        double pos3dL[][3], double pos2dL[][2], int numL,
                        double pos3dR[][3], double pos2dR[][2], int numR );

#ifdef __cplusplus
}
#endif
#endif

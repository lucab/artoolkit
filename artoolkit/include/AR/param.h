/*******************************************************
 *
 * Author: Takeshi Mita, Shinsaku Hiura, Hirokazu Kato
 *
 *         tmita@inolab.sys.es.osaka-u.ac.jp
 *         shinsaku@sys.es.osaka-u.ac.jp
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 4.1
 * Date: 01/12/07
 *
*******************************************************/
#ifndef AR_PARAM_H
#define AR_PARAM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <AR/config.h>

typedef struct {
    int      xsize, ysize;
    double   mat[3][4];
    double   dist_factor[4];
} ARParam;

typedef struct {
    int      xsize, ysize;
    double   matL[3][4];
    double   matR[3][4];
    double   matL2R[3][4];
    double   dist_factorL[4];
    double   dist_factorR[4];
} ARSParam;



int  arParamGet( double global[][3], double screen[][2], int data_num,
                 double mat[3][4] );

int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] );

int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] );


int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
                         double *ox, double *oy );

int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy );

int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam );

int    arParamSave( char *filename, int num, ARParam *param, ...);
int    arParamLoad( const char *filename, int num, ARParam *param, ...);
int    arParamDisp( ARParam *param );

/*-------------------*/

int    arsParamChangeSize( ARSParam *source, int xsize, int ysize, ARSParam *newparam );
int    arsParamSave( char *filename, ARSParam *sparam );
int    arsParamLoad( char *filename, ARSParam *sparam );
int    arsParamDisp( ARSParam *sparam );
int    arsParamGetMat( double matL[3][4], double matR[3][4],
                       double cparaL[3][4], double cparaR[3][4], double matL2R[3][4] );

#ifdef __cplusplus
}
#endif
#endif

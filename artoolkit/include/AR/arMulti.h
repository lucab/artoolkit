/*******************************************************
 *
 * Author: Hirokazu Kato
 *
 *         kato@sys.im.hiroshima-cu.ac.jp
 *
 * Revision: 1.0
 * Date: 01/09/05
 *
*******************************************************/

#ifndef AR_MULTI_H
#define AR_MULTI_H
#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <AR/config.h>
#include <AR/param.h>
#include <AR/ar.h>

typedef struct {
    int     patt_id;
    double  width;
    double  center[2];
    double  trans[3][4];
    double  itrans[3][4];
    double  pos3d[4][3];
    int     visible;
/*---*/
    int     visibleR;
} ARMultiEachMarkerInfoT;

typedef struct {
    ARMultiEachMarkerInfoT  *marker;
    int                     marker_num;
    double                  trans[3][4];
    int                     prevF;
/*---*/
    double                  transR[3][4];
} ARMultiMarkerInfoT;

ARMultiMarkerInfoT *arMultiReadConfigFile( const char *filename );

double  arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num,
                           ARMultiMarkerInfoT *config);

int arMultiActivate( ARMultiMarkerInfoT *config );

int arMultiDeactivate( ARMultiMarkerInfoT *config );

int arMultiFreeConfig( ARMultiMarkerInfoT *config );

double arsMultiGetTransMat(ARMarkerInfo *marker_infoL, int marker_numL,
                           ARMarkerInfo *marker_infoR, int marker_numR,
                           ARMultiMarkerInfoT *config);


#ifdef __cplusplus
}
#endif
#endif

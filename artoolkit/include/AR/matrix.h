/*******************************************************
 *   matrix.h  matrix handring library header
 *
 *
*******************************************************/
#ifndef AR_MATRIX_H
#define AR_MATRIX_H
#ifdef __cplusplus
extern "C" {
#endif


#include <math.h>
#include <AR/config.h>

/* === matrix definition ===

  <---- clm --->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |
  [ 13  14  15 ] v

=========================== */

typedef struct {
	double *m;
	int row;
	int clm;
} ARMat;

typedef struct {
        double *v;
        int    clm;
} ARVec;


/* 0 origin */
#define ARELEM0(mat,r,c) ((mat)->m[(r)*((mat)->clm)+(c)])
/* 1 origin */
#define ARELEM1(mat,row,clm) ARELEM0(mat,row-1,clm-1)



ARMat  *arMatrixAlloc(int row, int clm);
int    arMatrixFree(ARMat *m);

int    arMatrixDup(ARMat *dest, ARMat *source);
ARMat  *arMatrixAllocDup(ARMat *source);

int    arMatrixUnit(ARMat *unit);
ARMat  *arMatrixAllocUnit(int dim);

int    arMatrixMul(ARMat *dest, ARMat *a, ARMat *b);
ARMat  *arMatrixAllocMul(ARMat *a, ARMat *b);

int    arMatrixTrans(ARMat *dest, ARMat *source);
ARMat  *arMatrixAllocTrans(ARMat *source);

int    arMatrixInv(ARMat *dest, ARMat *source);
int    arMatrixSelfInv(ARMat *m);
ARMat  *arMatrixAllocInv(ARMat *source);

double arMatrixDet(ARMat *m);

int    arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean );
int    arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev );

int    arMatrixDisp(ARMat *m);


ARVec  *arVecAlloc( int clm );
int    arVecFree( ARVec *v );
int    arVecDisp( ARVec *v );
double arVecHousehold( ARVec *x );
double arVecInnerproduct( ARVec *x, ARVec *y );
int    arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e );


#ifdef __cplusplus
}
#endif
#endif

#ifndef CALIB_DIST_H
#define CALIB_DIST_H

#define  THRESH     150
#define  H_NUM        6
#define  V_NUM        4
#define  LOOP_MAX    20

typedef struct {
    double   x_coord;
    double   y_coord;
} CALIB_COORD_T;

typedef struct patt {
    unsigned char  *savedImage[LOOP_MAX];
    CALIB_COORD_T  *point[LOOP_MAX];
    int            h_num;
    int            v_num;
    int            loop_num;
} CALIB_PATT_T;

void calc_distortion( CALIB_PATT_T *patt, int xsize, int ysize, double dist_factor[4] );

#endif

#ifndef AR_GSUB_UTIL_H
#define AR_GSUB_UTIL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <AR/param.h>

void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) );

#ifdef __cplusplus
}
#endif
#endif

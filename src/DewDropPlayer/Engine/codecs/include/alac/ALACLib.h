#ifndef _ALACLIB_H_
#define _ALACLIB_H_

#define ALACLIB_ERR_NO_ERROR            0
#define ALACLIB_ERR_DECODER_INIT        -10
#define ALACLIB_ERR_DECODER_UNAVAILABLE -20
#define ALACLIB_ERR_ALREADY_INITIALIZED -30
#define ALACLIB_ERR_INVALID_DATA        -40
#define ALACLIB_ERR_DECODE              -50

#ifdef __cplusplus
extern "C"
{
#endif

#include <windows.h>

#define ALACLIBAPI extern __declspec (dllexport)

int ALACLIBAPI ALACLIB_Init(WORD nChannels, DWORD nSampleRate);
int ALACLIBAPI ALACLIB_GetMaxSizes(LPDWORD pInSampleSize, LPDWORD pOutSampleSize);
int ALACLIBAPI ALACLIB_Decode(LPBYTE lpInData, UINT nSampleDataSize, LPBYTE lpOutData, LPDWORD pDecodedDataSize);
int ALACLIBAPI ALACLIB_Flush();

#ifdef __cplusplus
}
#endif

#endif

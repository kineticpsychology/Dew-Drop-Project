#ifndef _AIFFLIB_H_
#define _AIFFLIB_H_

#ifndef UNICODE
#define UNICODE
#endif

#define AIFFLIB_ERROR_SUCCESS           0
#define AIFFLIB_ERROR_FILE              1
#define AIFFLIB_ERROR_INVALID_PARAM     2
#define AIFFLIB_ERROR_ALREADY_INIT      3
#define AIFFLIB_ERROR_NOT_INITIALIZED   4
#define AIFFLIB_ERROR_PARSE_INFO        5
#define AIFFLIB_ERROR_DECODE            6

#ifdef __cplusplus
extern "C"
{
#endif

#include <Windows.h>
#include <stdint.h>

#define AIFFLIBAPI extern __declspec (dllexport)

DWORD AIFFLIBAPI AIFFLIB_Init(LPCWSTR aiffFile, LPBYTE lpbtChannels,
                   LPDWORD lpdwSampleRate, LPDWORD lpdwTotalSamples,
                   LPWORD lpwBitsPerSample, LPDWORD lpdwDuration,
                   LPDWORD lpdwBitrate);
DWORD AIFFLIBAPI AIFFLIB_Decode(LPBYTE lpDecodedData, LPDWORD lpdwDecoded);
DWORD AIFFLIBAPI AIFFLIB_Tell();
DWORD AIFFLIBAPI AIFFLIB_Seek(DWORD dwSample);
DWORD AIFFLIBAPI AIFFLIB_Flush();

#ifdef __cplusplus
}
#endif

#endif

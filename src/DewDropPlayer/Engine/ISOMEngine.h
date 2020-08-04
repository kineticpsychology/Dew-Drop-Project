/*******************************************************************************
*                                                                              *
* COPYRIGHT (C) Aug, 2020 | Polash Majumdar                                    *
*                                                                              *
* This file is part of the Dew Drop Player project. The file content comes     *
* "as is" without any warranty of definitive functionality. The author of the  *
* code is not to be held liable for any improper functionality, broken code,   *
* bug and otherwise unspecified error that might cause damage or might break   *
* the application where this code is imported. In accordance with the          *
* Zero-Clause BSD licence model of the Dew Drop project, the end-user/adopter  *
* of this code is hereby granted the right to use, copy, modify, and/or        *
* distribute this code with/without keeping this copyright notice.             *
*                                                                              *
* TL;DR - It's a free world. All of us should give back to the world that we   *
*         get so much from. I have tried doing my part by making this code     *
*         free (as in free beer). Have fun. Just don't vandalize the code      *
*         or morph it into a malicious one.                                    *
*                                                                              *
*******************************************************************************/

#ifndef _ISOMENGINE_H_
#define _ISOMENGINE_H_

#include "WaveOutEngine.h"
#include "ISOMToOggVorbis.h"
#include "codecs\include\bento4\Ap4.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/bento4.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/bento4.lib")
#endif

#define ISOM_SUBTYPE_AAC_MAIN       0x01
#define ISOM_SUBTYPE_AAC_LC         0x02
#define ISOM_SUBTYPE_AAC_SSR        0x03
#define ISOM_SUBTYPE_AAC_LTP        0x04
#define ISOM_SUBTYPE_HE_AAC         0x05
#define ISOM_SUBTYPE_HEv2_AAC       0x1D    // 29

// These codecs all fall under the MP4A (AP4_SAMPLE_FORMAT_MP4A)
// category. Need to tell them apart using these
#define ISOM_CODEC_STRING_AAC_MAIN  "mp4a.40.1"
#define ISOM_CODEC_STRING_AAC_LC    "mp4a.40.2"
#define ISOM_CODEC_STRING_AAC_SSR   "mp4a.40.3"
#define ISOM_CODEC_STRING_AAC_LTP   "mp4a.40.4"
#define ISOM_CODEC_STRING_FAAC_LC   "mp4a.40.2"
#define ISOM_CODEC_STRING_FAAC_HE   "mp4a.40.5"
#define ISOM_CODEC_STRING_FAAC_HEV2 "mp4a.40.29"
#define ISOM_CODEC_STRING_MPEG      "mp4a.6B"   // Covers MP2 and MP3
#define ISOM_CODEC_STRING_VORBIS    "mp4a.DD"

// These codecs can be told apart by virtue of each of them
// having their own GetFormat() types
#define ISOM_CODEC_STRING_AC3       "ac-3"
#define ISOM_CODEC_STRING_ALAC      "alac"
#define ISOM_CODEC_STRING_EC3       "ec-3"
#define ISOM_CODEC_STRING_DTS       "mp4a.A9"
#define ISOM_CODEC_STRING_MP3       ".mp3"


typedef class ISOMENGINE : public WAVEOUTENGINE
{
    protected:
        AP4_File                *_pISOMFile = NULL;
        AP4_Track               *_pTrack = NULL;
        AP4_SampleDescription   *_pSampleDescr = NULL;
        IAUDIOENGINE            *_pInternalEngine = NULL;
        LPDWORD                 _SampleTable = NULL;
        DWORD                   _dwSampleTableSize = 0;
        DWORD                   _dwTotalTracks = 0;
        DWORD                   _dwAudioStreamID = 0;
        UINT                    _uAACSubType = 0;
        LPISOMTOOGGVORBIS       _pOggVorbisPacker = NULL;
        const AP4_UI32          AP4_SAMPLE_FORMAT_MP3 = 0x2E6D7033;

        virtual BOOL    _GenerateAACData();
        virtual BOOL    _GenerateRawSampleData();
        virtual BOOL    _GenerateALACData();
        virtual BOOL    _GenerateVorbisData();

        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwSeconds);

    public:
                        ISOMENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        BYTE            Play();
        BYTE            Pause();
        BYTE            Resume();
        BYTE            Stop();
        DWORD           Tell();
        BYTE            SetVolume(WORD wLevel);
        BYTE            Unload();
                        ~ISOMENGINE();

} ISOMENGINE, *LPISOMENGINE;

#endif

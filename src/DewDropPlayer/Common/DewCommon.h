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

#ifndef _DEWCOMMON_H_
#define _DEWCOMMON_H_

#define MAX_CHAR_PATH   0x2000
#define EXINFO_TYP_SIZE MAX_PATH
#define EXINFO_VAL_SIZE 0x1000

#include <Windows.h>
#include <mmsystem.h>
#include <strsafe.h>
#include <math.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <stdio.h>

#pragma comment(lib, "winmm")
#pragma comment(lib, "rpcrt4")
#pragma comment(lib, "shlwapi")

// Custom event IDs and string identifiers
#define WM_DEWMSG_PLAYBACK_COMPLETE (WM_USER + 0x0FF) // 'OFF'. WARNING! Do NOT change the naming convention or put this constant in DewUICommon.h!
#define PREPEVENT                   L"DEWENGINE.PREPARE.EVENT"
#define DECODEEVENT                 L"DEWENGINE.DECODE.EVENT"
#define POLLEVENT                   L"DEWENGINE.POLL.EVENT"

// Decode error codes
#define DEWDEC_DECODE_OK            0x00 // No error
#define DEWDEC_DECODE_ERROR         0x01 // Some error
#define DEWDEC_DECODE_EOD           0x02 // End of data

#define DEWS_MEDIA_NONE             0x00
#define DEWS_MEDIA_LOADED           0x01
#define DEWS_MEDIA_PLAYING          0x02
#define DEWS_MEDIA_PAUSED           0x03
#define DEWS_MEDIA_STOPPED          0x04
#define DEWS_MEDIA_UNLOADED         0x05

#define DEWERR_SUCCESS              0x00
#define DEWERR_INVALID_PARAM        0x01
#define DEWERR_INVALID_FILE         0x02
#define DEWERR_FILE_READ            0x03
#define DEWERR_PARSE                0x04
#define DEWERR_MM_ACTION            0x05
#define DEWERR_ENGINE_STARTUP       0x06
#define DEWERR_ENGINE_SHUTDOWN      0x07
#define DEWERR_UNSUPPORTED          0x08
#define DEWERR_NOT_IMPLEMENTED      0x09

// Pure Audio Stream Formats
#define DEWMT_UNKNOWN               0x00
#define DEWMT_AAC                   0x01
#define DEWMT_AC3                   0x02
#define DEWMT_AIFF                  0x03
#define DEWMT_ALAC                  0x04
#define DEWMT_APE                   0x05
#define DEWMT_CDDA                  0x06
#define DEWMT_FLAC                  0x07
#define DEWMT_MIDI                  0x08
#define DEWMT_MPC                   0x09
#define DEWMT_MP3                   0x0A
#define DEWMT_OPUS                  0x0B
#define DEWMT_VORBIS                0x0C
#define DEWMT_WAV                   0x0D
#define DEWMT_WMA                   0x0E
#define DEWMT_WV                    0x0F

// Container Formats
#define DEWMT_OGG                   0x20
#define DEWMT_ISOM                  0x40
#define DEWMT_3GP                   0x60

// Combos
#define DEWMT_OGG_FLAC              0x27    // DEWMT_OGG | DEWMT_FLAC
#define DEWMT_OGG_OPUS              0x2B    // DEWMT_OGG | DEWMT_OPUS
#define DEWMT_OGG_VORBIS            0x2C    // DEWMT_OGG | DEWMT_VORBIS

#define DEWMT_ISOM_AAC              0x41    // DEWMT_ISOM | DEWMT_AAC
#define DEWMT_ISOM_AC3              0x42    // DEWMT_ISOM | DEWMT_AC3
#define DEWMT_ISOM_ALAC             0x44    // DEWMT_ISOM | DEWMT_ALAC
#define DEWMT_ISOM_MP3              0x4A    // DEWMT_ISOM | DEWMT_MP3
#define DEWMT_ISOM_VORBIS           0x4C    // DEWMT_ISOM | DEWMT_VORBIS

#define DEWMT_3GP_AAC               0x61    // DEWMT_3GP | DEWMT_AAC
#define DEWMT_3GP_AC3               0x62    // DEWMT_3GP | DEWMT_AC3
#define DEWMT_3GP_ALAC              0x64    // DEWMT_3GP | DEWMT_ALAC
#define DEWMT_3GP_MP3               0x6A    // DEWMT_3GP | DEWMT_MP3

#endif

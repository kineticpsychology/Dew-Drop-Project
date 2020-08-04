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

#include "IAudioEngine.h"

// Generate a temp file name. This will be useful for the MP3/OGG engines
void IAUDIOENGINE::_GenerateTempSrcFile()
{
    GUID        tmpId;
    wchar_t     *wsGUID;
    wchar_t     wsTempPath[255];

    if (UuidCreate(&tmpId) == ERROR_SUCCESS)
    {
        UuidToString(&tmpId, (RPC_WSTR*)&wsGUID);
        GetTempPath(255, wsTempPath);
        if (wsTempPath[lstrlen(wsTempPath) - 1] == L'\\')
            wsTempPath[lstrlen(wsTempPath) - 1] = L'\0';
        StringCchPrintf(_wsInternalTemp, MAX_CHAR_PATH, L"%s\\~{%s}", wsTempPath, wsGUID);
        RpcStringFree((RPC_WSTR*)&wsGUID);
    }
    return;
}

// Store the entire contents of the source file into the memory
// This way, we rely only on the memory and reduce the IO over the file
// + the file lock is released and the player continues to play without it :)
BOOL IAUDIOENGINE::_StoreRawInputData(LPCWSTR sourceFile)
{
    HANDLE      hSrcFile = NULL;

    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", sourceFile);
    hSrcFile = CreateFile(_wsSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSrcFile == NULL || hSrcFile == INVALID_HANDLE_VALUE) return FALSE;
    _dwSrcDataSize = GetFileSize(hSrcFile, NULL);
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
    SetFilePointer(hSrcFile, 0, NULL, FILE_BEGIN);
    ReadFile(hSrcFile, _lpEncodedSrcData, _dwSrcDataSize, &_dwSrcDataSize, NULL);
    CloseHandle(hSrcFile);
    hSrcFile = NULL;
    return TRUE;
}

// Janitor work. Free the memory holding the source file data
// This will have to be called from the child destructors as well
void IAUDIOENGINE::_Cleanup()
{
    if (_lpEncodedSrcData)
    {
        LocalFree(_lpEncodedSrcData);
        _lpEncodedSrcData = NULL;
    }
    if (_tiTagInfo.AlbumArt)
    {
        delete _tiTagInfo.AlbumArt;
        _tiTagInfo.AlbumArt = NULL;
    }
    if (_lpExInfo)
    {
        LocalFree(_lpExInfo);
        _lpExInfo = NULL;
    }
}

// Just initialize the "read-only" variables & initiate COM
IAUDIOENGINE::IAUDIOENGINE() : Status(_btStatus),
                               MediaType(_btMediaType),
                               Duration(_dwDuration),
                               Bitrate(_dwBitrate),
                               SampleRate(_dwSampleRate),
                               Channels(_btChannels),
                               ExtendedInfoCount(_dwExInfoLength),
                               ExtendedInfo(_lpExInfo),
                               SourceFileName(_wsSrcFile),
                               Library(_wsLibrary),
                               Tag(_tiTagInfo)
{
    _GenerateTempSrcFile();
    CoInitialize(NULL);
}

// Utility function to add the container mask
// bitwise-OR'd with the core audio format
void IAUDIOENGINE::SetContainerFormat(BYTE containerFormat)
{
    this->_btMediaType = (this->_btMediaType | containerFormat);
    return;
}

// DTOR(). Nothing much. Just cleanup
IAUDIOENGINE::~IAUDIOENGINE()
{ 
    // *** This method MUST be called in the child
    // (implementor) classes as well ***
    this->_Cleanup();
    CoUninitialize();
    return;
}

// One big lookup on the audio formats. Deliberately kept as static to get
// the 'on-the-fly' audio format string without bogging down the memory
// with yet another instance of the engine. Plus this is an abstract class anyways.
void IAUDIOENGINE::GetAudioFormatString(BYTE audioFormatCode, LPWSTR formatCodeString, const DWORD stringLength)
{
    if (!formatCodeString || stringLength <= 0) return;
    switch (audioFormatCode)
    {
        // Core Formats
        case DEWMT_AAC: StringCchPrintf(formatCodeString, stringLength, L"Advanced Audio Codec (AAC)"); break;
        case DEWMT_AC3: StringCchPrintf(formatCodeString, stringLength, L"AC3 (A/52) Audio"); break;
        case DEWMT_AIFF: StringCchPrintf(formatCodeString, stringLength, L"Aiff Audio"); break;
        case DEWMT_ALAC: StringCchPrintf(formatCodeString, stringLength, L"Apple (Lossless) Audio (ALAC)"); break;
        case DEWMT_APE: StringCchPrintf(formatCodeString, stringLength, L"Monkey's (Lossless) Audio (APE)"); break;
        case DEWMT_CDDA: StringCchPrintf(formatCodeString, stringLength, L"CD (Lossless) Audio"); break;
        case DEWMT_FLAC: StringCchPrintf(formatCodeString, stringLength, L"Flac (Lossless) Audio"); break;
        case DEWMT_MIDI: StringCchPrintf(formatCodeString, stringLength, L"Midi (Synth) Audio"); break;
        case DEWMT_MPC: StringCchPrintf(formatCodeString, stringLength, L"Musepack Audio"); break;
        case DEWMT_MP3: StringCchPrintf(formatCodeString, stringLength, L"MPEG Audio"); break;
        case DEWMT_OPUS: StringCchPrintf(formatCodeString, stringLength, L"Opus Audio"); break;
        case DEWMT_VORBIS: StringCchPrintf(formatCodeString, stringLength, L"Vorbis Audio"); break;
        case DEWMT_WAV: StringCchPrintf(formatCodeString, stringLength, L"WAV PCM (Lossless) Audio"); break;
        case DEWMT_WMA: StringCchPrintf(formatCodeString, stringLength, L"Windows Media Audio (ASF/WMA)"); break;
        case DEWMT_WV: StringCchPrintf(formatCodeString, stringLength, L"WavPack Audio"); break;

        // Container Formats
        case DEWMT_OGG_FLAC: StringCchPrintf(formatCodeString, stringLength, L"FLAC (Lossless) Audio [Ogg container]"); break;
        case DEWMT_OGG_OPUS: StringCchPrintf(formatCodeString, stringLength, L"OPUS Audio [Ogg container]"); break;
        case DEWMT_OGG_VORBIS: StringCchPrintf(formatCodeString, stringLength, L"Vorbis Audio [Ogg container]"); break;

        case DEWMT_ISOM_AAC: StringCchPrintf(formatCodeString, stringLength, L"AAC Audio [ISOM (MP4/M4A) container]"); break;
        case DEWMT_ISOM_AC3: StringCchPrintf(formatCodeString, stringLength, L"AC3 Audio [ISOM (MP4/M4A) container]"); break;
        case DEWMT_ISOM_ALAC: StringCchPrintf(formatCodeString, stringLength, L"ALAC (Lossless) Audio [ISOM (MP4/M4A) container]"); break;
        case DEWMT_ISOM_MP3: StringCchPrintf(formatCodeString, stringLength, L"MP3 Audio [ISOM (MP4/M4A) container]"); break;
        case DEWMT_ISOM_VORBIS: StringCchPrintf(formatCodeString, stringLength, L"Vorbis Audio [ISOM (MP4/M4A) container] - Experimental"); break;

        case DEWMT_3GP_AAC: StringCchPrintf(formatCodeString, stringLength, L"AAC Audio [3GP container]"); break;
        case DEWMT_3GP_AC3: StringCchPrintf(formatCodeString, stringLength, L"AC3 Audio [3GP container]"); break;
        case DEWMT_3GP_ALAC: StringCchPrintf(formatCodeString, stringLength, L"ALAC Audio [3GP container]"); break;
        case DEWMT_3GP_MP3: StringCchPrintf(formatCodeString, stringLength, L"MP3 Audio [3GP container]"); break;

        default: StringCchPrintf(formatCodeString, stringLength, L"Unknown or unsupported format"); break;
    }
    return;
}

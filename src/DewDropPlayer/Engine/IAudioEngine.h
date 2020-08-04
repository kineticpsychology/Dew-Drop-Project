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

#ifndef _IAUDIOENGINE_H_
#define _IAUDIOENGINE_H_

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif


#include "TagInfo.h"

#include "..\Common\DewCommon.h"

typedef struct _EXTENDEDINFO
{
        wchar_t         Type[EXINFO_TYP_SIZE];
        wchar_t         Value[EXINFO_VAL_SIZE];

        void            Set(LPCWSTR type, LPCWSTR value)
        {
            StringCchPrintf(Type, EXINFO_TYP_SIZE, type);
            StringCchPrintf(Value, EXINFO_VAL_SIZE, value);
            return;
        }
        void            SetType(LPCWSTR type)
        {
            StringCchPrintf(Type, EXINFO_TYP_SIZE, type);
            return;
        }
        void            SetValue(LPCWSTR value)
        {
            StringCchPrintf(Value, EXINFO_VAL_SIZE, value);
            return;
        }
} EXTENDEDINFO, *LPEXTENDEDINFO;

typedef class IAUDIOENGINE
{
    private:
        void                    _GenerateTempSrcFile(); // Use a temp location filename

    protected:
        wchar_t                 _wsInternalTemp[MAX_CHAR_PATH];
        HWND                    _hWndNotify = NULL;
        LPBYTE                  _lpEncodedSrcData = NULL;
        DWORD                   _dwSrcDataSize = 0;

        BYTE                    _btStatus = DEWS_MEDIA_NONE;
        BYTE                    _btMediaType = DEWMT_UNKNOWN;
        DWORD                   _dwDuration = 0;
        DWORD                   _dwBitrate = 0;
        DWORD                   _dwSampleRate = 0;
        BYTE                    _btChannels = 0;
        DWORD                   _dwExInfoLength = 0;
        LPEXTENDEDINFO          _lpExInfo = NULL;
        wchar_t                 _wsSrcFile[MAX_CHAR_PATH];
        wchar_t                 _wsLibrary[MAX_PATH];
        TAGINFO                 _tiTagInfo;

        virtual BOOL            _StoreRawInputData(LPCWSTR sourceFile);
        virtual void            _Cleanup();

    public:
        const BYTE&             Status;
        const BYTE&             MediaType;
        const DWORD&            Duration;
        const DWORD&            Bitrate;
        const DWORD&            SampleRate;
        const BYTE&             Channels;
        const DWORD&            ExtendedInfoCount;
        const LPEXTENDEDINFO&   ExtendedInfo;
        const wchar_t*          SourceFileName;
        const wchar_t*          Library;
        TAGINFO&                Tag;

                                IAUDIOENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile) = 0;
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize) = 0;
        virtual BYTE            Play() = 0;
        virtual BYTE            Pause() = 0;
        virtual BYTE            Resume() = 0;
        virtual BYTE            Stop() = 0;
        virtual BYTE            Seek(DWORD dwMS) = 0;
        virtual DWORD           Tell() = 0;
        virtual BYTE            SetVolume(WORD wLevel) = 0;
        virtual BYTE            Unload() = 0;
        virtual void            SetContainerFormat(BYTE containerFormat);
        virtual                 ~IAUDIOENGINE();

        static void             GetAudioFormatString(BYTE audioFormatCode, LPWSTR formatCodeString, const DWORD stringLength);

} IAUDIOENGINE, *LPIAUDIOENGINE;

#endif

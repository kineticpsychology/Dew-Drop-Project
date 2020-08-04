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

#ifndef _DEWSTARTUP_H_
#define _DEWSTARTUP_H_

#define DEWSTARTUP_STATUS_UNKNOWN       0x00
#define DEWSTARTUP_STATUS_CONTINUE      0x01
#define DEWSTARTUP_STATUS_IMMEDIATE     0x02
#define DEWSTARTUP_STATUS_CLOSE         0x03
#define DEWSTARTUP_STATUS_ERROR         0xFF

#include "DewUICommon.h"

// Single instance. Do NOT inherit! This is NOT Thread safe!
typedef class DEWSTARTUP
{
    private:
        HANDLE              _hMap = NULL;
        HANDLE              _hMutex = NULL;
        HANDLE              _hEvtEnqueue = NULL;
        BOOL                _bOriginalInstance = FALSE;
        BYTE                _btStatus = DEWSTARTUP_STATUS_UNKNOWN;
        LPMAPCONTENT        _lpMapContent = NULL;
        wchar_t             _wsProgramName[MAX_PATH];
        LPFILELIST          _lpArgList = NULL;
        UINT                _nArgListCount = 0;

        static DWORD WINAPI _TrdWaitForQueuingFiles(LPVOID lpv);
        void                _SyncEnqueueArgs();
        void                _ShowDefaultHelp();
        void                _ShowEnqueueInProgressError();

    public:
        const BYTE&         Status;
                            DEWSTARTUP();
                            ~DEWSTARTUP();
} DEWSTARTUP, *LPDEWSTARTUP;

#endif

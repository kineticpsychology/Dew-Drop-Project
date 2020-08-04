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

#include "DewStartup.h"

// The '();' part is totally unnecessary, but it will drastically improve the readability below
#define DEF_HELP_OPER();   this->_ShowDefaultHelp(); _btStatus = _bOriginalInstance ? DEWSTARTUP_STATUS_CONTINUE : DEWSTARTUP_STATUS_CLOSE; return;

DWORD WINAPI DEWSTARTUP::_TrdWaitForQueuingFiles(LPVOID lpv)
{
    LPDEWSTARTUP        lpStartup;

    lpStartup = (LPDEWSTARTUP)lpv;
    lpStartup->_SyncEnqueueArgs();
    return 0;
}

void DEWSTARTUP::_SyncEnqueueArgs()
{
    UINT    nIndex;

    if (!_lpMapContent || !_lpArgList || _nArgListCount == 0) return;

    // Wait for the DEWWINDOW to trigger the event for playlisy enqueuing completion
    // NOTE: Use _hEvtEnqueue over to ensure _lpMapContent->hEvtStartEnqueue is the same event handle
    WaitForSingleObject(_lpMapContent->hEvtStartEnqueue, INFINITE);

    for (nIndex = 0; nIndex < _nArgListCount - 1; nIndex++)
    {
        // IMPORTANT: Just to be sure, flush out the media path address space
        //            before populating with any data. This will handle any missed '\0's
        ZeroMemory((_lpMapContent->wsMediaPath), MAX_CHAR_PATH * sizeof(wchar_t));

        // Copy the target file/folder path to the map
        CopyMemory((_lpMapContent->wsMediaPath), (_lpArgList[nIndex].wsFileName), sizeof(wchar_t) * lstrlen(_lpArgList[nIndex].wsFileName));

        // And ask window to enqueue it
        SendMessage(_lpMapContent->hWnd, WM_DEWMSG_ENQUEUE, DEWUI_ENQUEUE_SEQ_MORE, 0);
    }

    // IMPORTANT: Just to be sure, flush out the media path address space
    //            before populating with any data. This will handle any missed '\0's
    ZeroMemory((_lpMapContent->wsMediaPath), MAX_CHAR_PATH * sizeof(wchar_t));

    // Copy the target file/folder path to the map
    CopyMemory((_lpMapContent->wsMediaPath), (_lpArgList[nIndex].wsFileName), sizeof(wchar_t) * lstrlen(_lpArgList[nIndex].wsFileName));

    // And ask window to enqueue it (send SEQ_LAST to indicate the last item)
    SendMessage(_lpMapContent->hWnd, WM_DEWMSG_ENQUEUE, DEWUI_ENQUEUE_SEQ_LAST, 0);

    return;
}

void DEWSTARTUP::_ShowDefaultHelp()
{
    wchar_t     wsHelp[MAX_CHAR_PATH] { 0 };

    StringCchPrintf(wsHelp, MAX_CHAR_PATH, L"%s Usage help:\n" \
                    L"%s [switch] <file/folder 1> <file/folder 2> <file/folder 3>...\n\n" \
                    L"Switches (optional, case insensitive):\n" \
                    L"/p or /play or -p or -play: Play (file) or enqueue and play (folder)\n" \
                    L"/e or /enqueue or -e or -enqueue: Enqueue files/folders without playing\n" \
                    L"No switch: same as /p or -p switch", _wsProgramName, _wsProgramName);
    MessageBox(NULL, wsHelp, _wsProgramName, MB_OK | MB_ICONINFORMATION);
    return;
}

void DEWSTARTUP::_ShowEnqueueInProgressError()
{
    const wchar_t     *wsError = L"An error occurred. It appears another enqueuing operation " \
                                 L"in Dew Drop Player is already under progress. Please wait for the " \
                                 L"operation to complete and retry after some time.";

    MessageBox(NULL, wsError, _wsProgramName, MB_OK | MB_ICONEXCLAMATION);
    return;
}

DEWSTARTUP::DEWSTARTUP() : Status(_btStatus)
{
    DWORD       dwError = 0;
    DWORD       dwGen = 0;
    LPWSTR      *ppArgs = NULL;
    int         iArgs = 0, iIdx, iOffset;
    wchar_t     wsAppPath[MAX_CHAR_PATH] { 0 };

    ZeroMemory(_wsProgramName, MAX_PATH * sizeof(wchar_t));
    GetModuleFileName(NULL, wsAppPath, MAX_CHAR_PATH);
    for (iIdx = lstrlen(wsAppPath) - 1; iIdx >= 0; iIdx--)
        if (wsAppPath[iIdx] == L'\\')
            break;
    iIdx++;
    CopyMemory(_wsProgramName, &(wsAppPath[iIdx]), (lstrlen(wsAppPath) - iIdx) * sizeof(wchar_t));

    // Start with no API errors.
    SetLastError(0x00);

    // Start with error status. We will set the code only after everything runs successfully
    _btStatus = DEWSTARTUP_STATUS_ERROR;

    #pragma region MUTEX SECTION
    // Create the unique mutex
    _hMutex = CreateMutex(NULL, TRUE, DEWUI_MUTEX);
    dwError = GetLastError();

    // If not already running/successful, something fucked up. Bail!
    if (dwError != ERROR_ALREADY_EXISTS && dwError != ERROR_SUCCESS)
        return;
    #pragma endregion


    #pragma region MAP INITIATION SECTION

    // This is the original instance
    if (dwError == ERROR_SUCCESS)
    {
        _bOriginalInstance = TRUE;
        // Reserve space for the DewUI HWND, followed by wide char of MAX_CHAR_PATH
        // We will queue the file/path into this folder
        dwGen = sizeof(MAPCONTENT);
        _hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwGen, DEWUI_MAP);
        if (_hMap)
        {
            _lpMapContent = (LPMAPCONTENT)MapViewOfFile(_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (!_lpMapContent)
                return;
            ZeroMemory(_lpMapContent, sizeof(MAPCONTENT));
        }
        else
        {
            return;
        }
    }
    // Alternate instance
    else
    {
        _bOriginalInstance = FALSE;
        // Read the map
        _hMap = OpenFileMapping(GENERIC_READ | GENERIC_WRITE, FALSE, DEWUI_MAP);
        if (_hMap)
        {
            _lpMapContent = (LPMAPCONTENT)MapViewOfFile(_hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
            if (!_lpMapContent)
                return;

            // Additional check. Make sure the event is NULL.
            // If it is not NULL, it means another instance of Startup
            // has requested for queueing and is currently operational.
            if (_lpMapContent->hEvtStartEnqueue != NULL &&
                _lpMapContent->hEvtStartEnqueue != INVALID_HANDLE_VALUE)
            {
                this->_ShowEnqueueInProgressError();
                return;
            }
        }
        else
        {
            return;
        }
    }

    #pragma endregion


    #pragma region CMDLINE SECTION

    ppArgs = CommandLineToArgvW(GetCommandLine(), &iArgs);

    // To remove error check clutter below, check all the files
    // for valid paths and existences
    for (iOffset = 1; iOffset < iArgs; iOffset++)
    {
        // Check all the file attributes starting from the first argument
        dwError = GetFileAttributes(ppArgs[iOffset]);
        if (dwError == INVALID_FILE_ATTRIBUTES)
        {
            if (iOffset != 1) // If it is not the first arg, it is definitely an issue
            {
                DEF_HELP_OPER();
            }
            else // If it is the first arg
            {
                // and is not one of the switches
                if (lstrcmpi(ppArgs[1], L"/e") &&
                    lstrcmpi(ppArgs[1], L"-e") &&
                    lstrcmpi(ppArgs[1], L"/enqueue") &&
                    lstrcmpi(ppArgs[1], L"-enqueue"))
                {
                    DEF_HELP_OPER();
                }
            }
        }
        // Steer clear of the '.' & '..' notations as well
        else if (!lstrcmpi(ppArgs[iOffset], L".") || !lstrcmpi(ppArgs[iOffset], L".."))
        {
            DEF_HELP_OPER();
        }
    }

    // By this point, it is guaranteed, that the arguments are in order
    // So, no further sanity check will be needed

    // Args = 0
    if (iArgs < 2)
    {
        _lpMapContent->btCmdlineOpt = DEWUI_CMDLINE_OPT_ENQUEUE;

        // Open up the application as usual, for the first instance, bail otherwise
        _btStatus = _bOriginalInstance ? DEWSTARTUP_STATUS_CONTINUE : DEWSTARTUP_STATUS_CLOSE;
        return;
    }

    // Args = 1
    else if (iArgs == 2)
    {
        // The 'only' arg is in fact, a switch. That would be invalid option
        if ((!lstrcmpi(ppArgs[1], L"/e") ||
            !lstrcmpi(ppArgs[1], L"-e") ||
            !lstrcmpi(ppArgs[1], L"/enqueue") ||
            !lstrcmpi(ppArgs[1], L"-enqueue")))
        {
            DEF_HELP_OPER();
        }
        else // The only arg is a file/dir. So we need to play it immediately
        {
            iOffset = 1;
            _lpMapContent->btCmdlineOpt = DEWUI_CMDLINE_OPT_PLAY;

            if (_bOriginalInstance) // If this is the first instance, set the special status
            {
                _btStatus = DEWSTARTUP_STATUS_IMMEDIATE;
            }
            else // Otherwise, set the regular continuation status & flush the existing playlist
            {
                SendMessage(_lpMapContent->hWnd, WM_DEWMSG_FLUSH_PLAYLIST, 0, 0);
                _btStatus = DEWSTARTUP_STATUS_CONTINUE;
            }
        }
    }

    // Args > 1
    else
    {
        iOffset = 1;
        // Start with immediate play
        _lpMapContent->btCmdlineOpt = DEWUI_CMDLINE_OPT_PLAY;

        // Play option. Increment the index
        if (!lstrcmpi(ppArgs[1], L"/e") ||
            !lstrcmpi(ppArgs[1], L"-e") ||
            !lstrcmpi(ppArgs[1], L"/enqueue") ||
            !lstrcmpi(ppArgs[1], L"-enqueue"))
        {
            iOffset = 2; // Increment the offset to skip the switch arg (ppArgs[1])
            _lpMapContent->btCmdlineOpt = DEWUI_CMDLINE_OPT_ENQUEUE;
        }
        else
        {
            SendMessage(_lpMapContent->hWnd, WM_DEWMSG_FLUSH_PLAYLIST, 0, 0);
        }
        _btStatus = DEWSTARTUP_STATUS_CONTINUE;
    }

    // If the code has reached here, it means that things will have to be queued
    // Dump the args into the stack
    _lpArgList = (LPFILELIST)LocalAlloc(LPTR, (iArgs - iOffset) * sizeof(FILELIST));
    for (iIdx = iOffset; iIdx < iArgs; iIdx++)
        StringCchPrintf((_lpArgList[iIdx - iOffset].wsFileName), MAX_CHAR_PATH, ppArgs[iIdx]);
    _nArgListCount = (iArgs - iOffset);

    #pragma endregion


    #pragma region EVENT SECTION

    // A map is available, with an event slot available.
    // Make sure to create an event, which the window can signal
    _hEvtEnqueue = CreateEvent(NULL, TRUE, FALSE, DEWUI_ENQUEUE_EVENT);
    if (_hEvtEnqueue == NULL || _hEvtEnqueue == INVALID_HANDLE_VALUE)
    {
        _btStatus = DEWSTARTUP_STATUS_ERROR;
        return;
    }
    // Event is created. Update it to the map, so that DEWWINDOW
    // can signal the event, once the enqueue process is over
    _lpMapContent->hEvtStartEnqueue = _hEvtEnqueue;

    #pragma endregion

    // All set for synchronous wait for DEWWINDOW to update/set the event
    // We will wait for it via:
    // 1. Separate thread for original instance (so that DEWWINDOW can get
    //    created and the event can get triggered) OR
    // 2. Main application thread for any other instance (since a thread
    //    will not be able to block the main thread and will get terminated
    //    immediately).
    if (_bOriginalInstance)
    {
        CreateThread(NULL, 0, _TrdWaitForQueuingFiles, this, 0, NULL);
    }
    else
    {
        SetEvent(_lpMapContent->hEvtStartEnqueue);
        this->_SyncEnqueueArgs();
        _btStatus = DEWSTARTUP_STATUS_CLOSE;
    }

    return;
}

// Cleanup
DEWSTARTUP::~DEWSTARTUP()
{
    // Unmap the shared memory and close the map file handle
    if (_hMap)
    {
        if (_lpMapContent)
        {
            UnmapViewOfFile(_lpMapContent);
            _lpMapContent = NULL;
        }
        CloseHandle(_hMap);
        _hMap = NULL;
    }

    // Close the mutex object
    if (_hMutex) { CloseHandle(_hMutex); _hMutex = NULL; }

    // Close out the enqueue event
    if (_hEvtEnqueue) { CloseHandle(_hEvtEnqueue); _hEvtEnqueue = NULL; }

    // Release any file list memory that was reserved
    if (_lpArgList) LocalFree(_lpArgList);
    _lpArgList = NULL;
    return;
}

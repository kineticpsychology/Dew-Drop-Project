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

#include "MIDIEngine.h"

void MIDIENGINE::_GetDSVersion()
{
    //MAX_PATH
    //_wsLibrary
    DWORD               dwVerInfoSize = 0, dwGenSize = 0;
    LPBYTE              lpVersionData = NULL, lpOffset = NULL;
    VS_FIXEDFILEINFO    *pVFFInfo = NULL;
    OLECHAR*            wsMIDICLSID = NULL;
    wchar_t             wsMIDIRegKey[MAX_PATH];
    HKEY                hkRegMidi = NULL;
    wchar_t*            lpDLLFile = NULL;

    StringFromCLSID(CLSID_AVIMIDIRender, &wsMIDICLSID);
    if (lstrlen((wchar_t*)wsMIDICLSID) > 0)
    {
        StringCchPrintf(wsMIDIRegKey, MAX_PATH, L"CLSID\\%s\\InprocServer32", (wchar_t*)wsMIDICLSID);
        RegOpenKeyEx(HKEY_CLASSES_ROOT, wsMIDIRegKey, 0, KEY_READ, &hkRegMidi);
        if (hkRegMidi && hkRegMidi != INVALID_HANDLE_VALUE)
        {
            dwGenSize = 0;
            RegQueryValue(hkRegMidi, NULL, NULL, (PLONG)&dwGenSize);
            if (dwGenSize > 0)
            {
                lpDLLFile = (wchar_t*)LocalAlloc(LPTR, dwGenSize);
                RegQueryValue(hkRegMidi, NULL, lpDLLFile, (PLONG)&dwGenSize);
            }
            RegCloseKey(hkRegMidi);
        }
        CoTaskMemFree(wsMIDICLSID);
    }

    if (lpDLLFile)
    {
        dwGenSize = 0;
        dwVerInfoSize = GetFileVersionInfoSize(lpDLLFile, &dwGenSize);
        if (dwVerInfoSize > 0)
        {
            lpVersionData = (LPBYTE)LocalAlloc(LPTR, dwVerInfoSize);
            GetFileVersionInfo(lpDLLFile, 0, dwVerInfoSize, lpVersionData);
            dwGenSize = 0;
            VerQueryValue(lpVersionData, L"\\", (LPVOID*)&lpOffset, (PUINT)&dwGenSize);
            if (dwGenSize)
            {
                pVFFInfo = (VS_FIXEDFILEINFO*)lpOffset;
                StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows DirectShow Filter (product version %u.%u.%u.%u)",
                    (pVFFInfo->dwProductVersionMS >> 16),
                    (pVFFInfo->dwProductVersionMS >> 0) & 0xFFFF,
                    (pVFFInfo->dwProductVersionLS >> 16),
                    (pVFFInfo->dwProductVersionLS >> 0) & 0xFFFF);
            }
            LocalFree(lpVersionData);
        }
        LocalFree(lpDLLFile);
    }
    return;
}

// This is a polling thread. This is different from the play thread
// from the waveout engine. IMediaControl->Run() is async. So no
// buffer driven event loop is needed for packet-by-packiet data filling
// Instead a parallel thread is needed to keep track of whether the
// playback has finished. Dually, we'll also utilize this thread
// to keep track of the current position. We'll simply send it when Tell() is called.
DWORD MIDIENGINE::_TrdPoll(LPVOID lpv)
{
    LPMIDIENGINE        lpEngine = NULL;
    OAFilterState       oaFilterState;
    HRESULT             hr = S_OK;
    BOOL                bPlaybackComplete = FALSE;
    LONGLONG            pSeekPos = 0;

    lpEngine = (LPMIDIENGINE)lpv;
    // Keep looping every 10ms
    do
    {
        hr = lpEngine->_pIMC->GetState(INFINITE, &oaFilterState);
        // This can happen if Stop() was invoked.
        if (oaFilterState == State_Stopped) break;
        hr = lpEngine->_pIMS->GetCurrentPosition(&(lpEngine->_llCurr));
        if (lpEngine->_llCurr >= lpEngine->_llTotal)
        {
            bPlaybackComplete = TRUE;
            break;
        }
        Sleep(lpEngine->_dwPollFreq);
    } while (SUCCEEDED(hr));

    // Simulate all the actions from Stop().
    // Cannot call Stop as it would wait for _hPoll and a deadlock would occur.
    if (bPlaybackComplete)
    {
        if (lpEngine->_hWndNotify != NULL && lpEngine->_hWndNotify != INVALID_HANDLE_VALUE)
            PostMessage(lpEngine->_hWndNotify, WM_DEWMSG_PLAYBACK_COMPLETE, 0, 0);
        if (lpEngine->_pIMC) hr = lpEngine->_pIMC->Stop();
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        hr = lpEngine->_pIMS->SetPositions(&pSeekPos, AM_SEEKING_AbsolutePositioning,
                                           NULL, AM_SEEKING_NoPositioning);
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        lpEngine->_btStatus = DEWS_MEDIA_STOPPED;
    }
    // Reset the current position, clear the thread (we're done with the thread anyways)
    lpEngine->_llCurr = 0;
    lpEngine->_hTrdPoll = NULL;
    //Signal event that Thread execution is complete (needed by Stop() function)
    SetEvent(lpEngine->_hPoll);
    return 0;
}

// Not a fucking clue what is going on here. Simple rip-off form the net
IPin* MIDIENGINE::_GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
    BOOL            bFound = FALSE;
    IEnumPins       *pEnum;
    IPin            *pPin;
    PIN_DIRECTION   PinDirThis;
    HRESULT         hr = S_OK;

    if (!pFilter) return NULL;

    hr = pFilter->EnumPins(&pEnum);
    if (!SUCCEEDED(hr)) return NULL;

    while (pEnum->Next(1, &pPin, 0) == S_OK)
    {
        pPin->QueryDirection(&PinDirThis);
        bFound = (PinDir == PinDirThis);
        if (bFound) break;
    }
    pEnum->Release();
    return (bFound ? pPin : NULL);
}

// Manually parse the file to get those "Fancy" information
void MIDIENGINE::_GetExInfo()
{
    HANDLE      hMidi = NULL;
    WORD        wTracks = 0, wFormat = 0;
    DWORD       dwGen = 0;
    wchar_t     wsValue[EXINFO_VAL_SIZE];

    hMidi = CreateFile(_wsSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hMidi == NULL || hMidi == INVALID_HANDLE_VALUE) return;
    SetFilePointer(hMidi, 0x08, NULL, FILE_BEGIN);
    ReadFile(hMidi, &wFormat, 2, &dwGen, NULL);
    ReadFile(hMidi, &wTracks, 2, &dwGen, NULL);
    CloseHandle(hMidi);

    wFormat = ((wFormat << 0x08) | (wFormat >> 0x08));
    wTracks = ((wTracks << 0x08) | (wTracks >> 0x08));

    _dwExInfoLength = 2;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"Midi Format");
    switch (wFormat)
    {
    case 1: _lpExInfo[0].SetValue(L"Format 1 (Single Track)"); break;
    case 2: _lpExInfo[0].SetValue(L"Format 2 (Multiple Simultaneous Tracks)"); break;
    case 3: _lpExInfo[0].SetValue(L"Format 3 (Multiple Independent Tracks"); break;
    default: _lpExInfo[0].SetValue(L"Unknown");
    }
    _lpExInfo[1].SetType(L"Track Chunks");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", wTracks);
    _lpExInfo[1].SetValue(wsValue);
    return;
}

// Release all the COMs that were intialized
void MIDIENGINE::_Cleanup()
{
    this->Stop();
    CloseHandle(_hPoll);
    if (_pFileOut)
    {
        //pIGB->Disconnect(pFileOut);
        _pFileOut->Release();
        _pFileOut = NULL;
    }

    if (_pIPFileFilter)
    {
        //pIGB->RemoveFilter(pIPFileFilter);
        _pIPFileFilter->Release();
        _pFileOut = NULL;
    }

    if (_pMIDIIn) _pMIDIIn->Release();

    if (_pMIDIRenderer) _pMIDIRenderer->Release();

    if (_pIMS) _pIMS->Release();
    if (_pIMC) _pIMC->Release();
    if (_pIBA) _pIBA->Release();
    if (_pIGB) _pIGB->Release();

    // Do not call this. Other COMs might be in use, which would
    // break if this is called while they are active
    //CoUninitialize();
    return;
}

// CTOR(). Initialize COM and create all the Interface pointers
MIDIENGINE::MIDIENGINE()
{
    HRESULT     hr = S_OK;

    _btMediaType = DEWMT_MIDI;
    hr = CoInitialize(NULL);

    // Typical COM process. One global method to get the Interface pointer
    // and then a bunch of QueryInterface() calls to check the support
    // and get the implementor pointer
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (LPVOID*)&_pIGB);
    if (hr != S_OK) return;

    // IBasicAudio (Volume)
    hr = _pIGB->QueryInterface(IID_IBasicAudio, (LPVOID*)&_pIBA);
    if (hr != S_OK) return;

    // IMediaControl (Pause, resume, stop)
    hr = _pIGB->QueryInterface(IID_IMediaControl, (LPVOID*)&_pIMC);
    if (hr != S_OK) return;

    // IMediaSeeking (Seek/Tell)
    hr = _pIGB->QueryInterface(IID_IMediaSeeking, (LPVOID*)&_pIMS);
    if (hr != S_OK) return;

    // Prepare the renderers
    //hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER,
    //                      IID_IBaseFilter, (LPVOID*)&_pDSRenderer);
    //if (_pDSRenderer) _pDSoundIn = this->_GetPin(_pDSRenderer, PINDIR_INPUT);
    hr = CoCreateInstance(CLSID_AVIMIDIRender, NULL, CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter, (LPVOID*)&_pMIDIRenderer);
    if (_pMIDIRenderer) _pMIDIIn = this->_GetPin(_pMIDIRenderer, PINDIR_INPUT);

    // DirectSound Filter (Not needed... just a handy reference)
    //hr = pIGB->AddFilter(pDSRenderer, L"DSoundFilter");
    //if (hr != S_OK) return;
    // Add the MIDI DirectSound Filter
    hr = _pIGB->AddFilter(_pMIDIRenderer, L"MIDIFilter");
    if (hr != S_OK) return;

    if (!_pIGB || !_pIBA || !_pIMC || !_pIMS || !_pMIDIRenderer || !_pMIDIIn)
        return;

    _hPoll = CreateEvent(NULL, TRUE, FALSE, POLLEVENT);
    this->_GetDSVersion();
    _bInitialized = TRUE;
    return;
}

// After initializing the COM pointers, use IPin* to connect filter with the midi file
BYTE MIDIENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    HRESULT     hr = S_OK;

    if (!_bInitialized) return DEWERR_ENGINE_STARTUP;
    if (!notificationWindow || notificationWindow == INVALID_HANDLE_VALUE || !srcFile) return DEWERR_INVALID_PARAM;

    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    hr = _pIGB->AddSourceFilter(_wsSrcFile, _wsSrcFile, &_pIPFileFilter);
    if (hr != S_OK) return DEWERR_ENGINE_STARTUP;

    if (_pFileOut) _pFileOut->Release();

    _pFileOut = _GetPin(_pIPFileFilter, PINDIR_OUTPUT);
    if (!_pFileOut) return FALSE;

    if (_pMIDIIn) hr = _pIGB->Connect(_pFileOut, _pMIDIIn);
    if (hr != S_OK) return FALSE;

    _pIMS->GetDuration(&_llTotal);
    _dwDuration = (DWORD)(_llTotal / 10000);
    _llCurr = 0;

    // Other typical information do not apply
    _dwBitrate = 0;
    _btChannels = 0;

    // Get MIDI specific info
    this->_GetExInfo();
    
    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Not implemented
BYTE MIDIENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Invoke the IMediaControl->Run() and keep polling for monitoring duration/status changes
BYTE MIDIENGINE::Play()
{
    LONGLONG        pSeekPos = 0;
    HRESULT         hr = S_OK;

    if (_btStatus == DEWS_MEDIA_LOADED || _btStatus == DEWS_MEDIA_STOPPED)
    {
        hr = _pIMS->SetPositions(&pSeekPos, AM_SEEKING_AbsolutePositioning,
                                 NULL, AM_SEEKING_NoPositioning);
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        hr = _pIMC->Run();
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        if (_hTrdPoll)
        {
            TerminateThread(_TrdPoll, 0x00);
            _hTrdPoll = NULL;
        }
        ResetEvent(_hPoll);
        _hTrdPoll = CreateThread(NULL, 0, _TrdPoll, this, 0, NULL);
        _btStatus = DEWS_MEDIA_PLAYING;
    }
    return DEWERR_SUCCESS;
}

// Pause with IMediaControl
BYTE MIDIENGINE::Pause()
{
    HRESULT     hr = S_OK;

    if (_btStatus == DEWS_MEDIA_PLAYING && _pIMC)
    {
        hr = _pIMC->Pause();
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        _btStatus = DEWS_MEDIA_PAUSED;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Resume (Play) with IMediaControl
BYTE MIDIENGINE::Resume()
{
    HRESULT     hr = S_OK;

    if (_btStatus == DEWS_MEDIA_PAUSED && _pIMC)
    {
        hr = _pIMC->Run();
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        _btStatus = DEWS_MEDIA_PLAYING;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Stop with IMediaControl
BYTE MIDIENGINE::Stop()
{
    LONGLONG        pSeekPos = 0;
    HRESULT         hr = S_OK;

    if ((_btStatus == DEWS_MEDIA_PLAYING || _btStatus == DEWS_MEDIA_PAUSED) && _pIMC && _pIMS)
    {
        hr = _pIMC->Stop();
        // Wait for the Polling thread to complete
        WaitForSingleObject(_hPoll, INFINITE);
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        // Seek back to position 0
        hr = _pIMS->SetPositions(&pSeekPos, AM_SEEKING_AbsolutePositioning,
                                 NULL, AM_SEEKING_NoPositioning);
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        _btStatus = DEWS_MEDIA_STOPPED;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Seek with IMediaSeeking
BYTE MIDIENGINE::Seek(DWORD dwMS)
{
    LONGLONG    llpSeekPos;
    DWORD       dwPos;
    HRESULT     hr = S_OK;

    if (_btStatus == DEWS_MEDIA_PAUSED || _btStatus == DEWS_MEDIA_PLAYING)
    {
        if (!_pIMS) return DEWERR_MM_ACTION;

        // Seek in multiplier of ms, further multiplied by 10000 (LONGLONG)
        dwPos = dwMS;
        if (dwMS == 0) return DEWERR_SUCCESS;
        llpSeekPos = (dwMS >= _dwDuration ? _dwDuration : dwPos) * 10000;
        hr = _pIMS->SetPositions(&llpSeekPos, AM_SEEKING_AbsolutePositioning, NULL,
                                AM_SEEKING_NoPositioning);
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// We could've used IMediaSeeking. Bth that's a 
// COM call again. Instead, simply borrow the value
// from the already running (and monitoring) polling thread
DWORD MIDIENGINE::Tell()
{
    return (DWORD) (_llCurr / 10000);
}

// THIS DOESN'T WORK!
// Log (base 2) is already setup
//
BYTE MIDIENGINE::SetVolume(WORD wLevel)
{
    double      fPercent, fLog;
    DWORD       dwVol = 0;
    LONG        lVol;
    HRESULT     hr = S_OK;

    if (!_pIBA) return DEWERR_MM_ACTION;

    // Map down wLevel to range of 0 to 100
    if (wLevel == 0xFFFF)
        dwVol = 100;
    else
        dwVol = (DWORD)((double)wLevel / (double)0xFFFF * 100.00);

    fPercent = (double)dwVol*0.01;
    //Just ensure that the value stays within -9.9999 and 0.0
    // That way, when multiplied by 1000, it will come under the range
    // of -10000 to 0. Just ensure that we do not try to get log2(0)
    // it will be infinite
    if (fPercent <= 0.0009766) fPercent = 0.0009766;
    fLog = (double)log2(fPercent);
    lVol = long(fLog*1000.0);

    hr = _pIBA->put_Volume(lVol);
    if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;

    return DEWERR_SUCCESS;
}

// Nothing notable. Just ensure that Stop() is triggered
BYTE MIDIENGINE::Unload()
{
    // Nothing else to do. _pFileOut will be auto-flushed druing Load() call
    this->Stop();
    return DEWERR_SUCCESS;
}

// Housekeeping
MIDIENGINE::~MIDIENGINE()
{
    this->_Cleanup();
    return;
}

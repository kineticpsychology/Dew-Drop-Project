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

#include "WaveOutEngine.h"

// Call the actually implemented WAVProc in the subsequent child classes
void CALLBACK WAVEOUTENGINE::_WAVHandler(HWAVEOUT hwo, UINT nMsg, DWORD_PTR dwRefData,
                                        DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    LPWAVEOUTENGINE     lpEngine;

    lpEngine = (LPWAVEOUTENGINE)dwRefData;
    lpEngine->_WAVProc(hwo, nMsg, dwParam1, dwParam2);
    return;
}

// Automatically called by the static function WAVHandler
void WAVEOUTENGINE::_WAVProc(HWAVEOUT hwo, UINT nMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    MMRESULT    mmr = 0;

    switch (nMsg)
    {
        case WOM_OPEN:
             SetEvent(this->_hEvtPrepare);
             return;
        case WOM_DONE:
             // If data is still present get & set the ID of the block (set by the 'Play' function).
             if (!_bNoMoreData)
                _dwCurrentIndex = ((LPWAVEHDR)dwParam1)->dwUser;
             // Signal the awaiting playback thread that decoding & playback can start
             // This is irrespective of whether data is present. The playback thread will handle this
             SetEvent(this->_hEvtDecode);
             
             return;
        default:
             return;
    }
    return;
}

// Eternal thread that will keep calling the internal _Decode function
// This will be spawned by Play() and terminated by Stop();
DWORD WINAPI WAVEOUTENGINE::_TrdPlay(LPVOID lpv)
{
    LPWAVEOUTENGINE     lpEngine = (LPWAVEOUTENGINE)lpv;
    MMRESULT            mmr = MMSYSERR_NOERROR;
    BYTE                decodeRet = 0;

    do
    {
        WaitForSingleObject(lpEngine->_hEvtDecode, INFINITE);
        // Failsafe. Before attempting to decode, check if end-of-data
        // has already been reached. In that case, we are done here
        if (lpEngine->_bNoMoreData) break;
        ResetEvent(lpEngine->_hEvtDecode);
        // This variable will mostly be false. So this is just another small statement.
        // It will only momentarily be turned on when a seek happens (for a fraction of a second)
        // This is to stop messing the decoder engines decoding activity while there is a
        // displacement with the sample location. It might not affect really quick/simple decoders
        // like .wav, but can very well affect decoders like WMA, FLAC, APE etc.
        // Lastly, this is kinda risk free as this stoppage is happening in a thread
        // which can be killed at any given point of time
        while (lpEngine->_bStopDecoding);
        decodeRet = lpEngine->_Decode();
        if (decodeRet == DEWDEC_DECODE_ERROR) return 0xDEAD;
        mmr = waveOutWrite(lpEngine->_hWO, &(lpEngine->_blocks[lpEngine->_dwCurrentIndex]), sizeof(WAVEHDR));
        if (mmr != MMSYSERR_NOERROR) return 0xDEAD;
    } while (decodeRet != DEWDEC_DECODE_EOD);

    lpEngine->_bNoMoreData = TRUE;
    lpEngine->_hTrdPlay = NULL;

    lpEngine->Stop();
    DestroyWindow(lpEngine->_hWndNotify);
    if (lpEngine->_hWndNotify != NULL && lpEngine->_hWndNotify != INVALID_HANDLE_VALUE)
        SendMessage(lpEngine->_hWndNotify, WM_DEWMSG_PLAYBACK_COMPLETE, 0, 0);

    return DEWERR_SUCCESS;

}

// Clean out the reserved memories for the encoded & decoded data streams
void WAVEOUTENGINE::_Cleanup()
{
    this->Stop();
    if (_lpDecodedData)
    {
        LocalFree(_lpDecodedData);
        _lpDecodedData = NULL;
    }
    if (_lpOverflowData)
    {
        LocalFree(_lpOverflowData);
        _lpOverflowData = NULL;
    }
    if (_blocks)
    {
        LocalFree(_blocks);
        _blocks = NULL;
    }
    if (_hEvtPrepare)
    {
        CloseHandle(_hEvtPrepare);
        _hEvtPrepare = NULL;
    }
    if (_hEvtDecode)
    {
        CloseHandle(_hEvtDecode);
        _hEvtDecode = NULL;
    }
    return;
}

// Set a default CHUNKSIZE and CHUNKCOUNT
// which can be overridden if required (or maybe NOT!)
// Also, create the event for continuous decoding & playback
WAVEOUTENGINE::WAVEOUTENGINE()
{
    // 0x8000 x 0x02 <- Original Setting
    CHUNKSIZE = 0x8000;
    CHUNKCOUNT = 0x02;

    _lpDecodedData = (LPBYTE)LocalAlloc(LPTR, CHUNKSIZE * CHUNKCOUNT);
    _blocks = (WAVEHDR*)LocalAlloc(LPTR, sizeof(WAVEHDR) * CHUNKCOUNT);
    _hEvtPrepare = CreateEvent(NULL, TRUE, FALSE, PREPEVENT);
    _hEvtDecode = CreateEvent(NULL, TRUE, FALSE, DECODEEVENT);
    return;
}

// Decode the initial CHUNKSIZE * CHUNKCOUNT bytes of data
// prepare the block headers and write out the media to get the thing rolling
// WAVProc will continue handling the rest
BYTE WAVEOUTENGINE::Play()
{
    UINT        index;
    BYTE        decodeRet = 0;
    MMRESULT    mmr = MMSYSERR_NOERROR;

    if (_btStatus != DEWS_MEDIA_LOADED && _btStatus != DEWS_MEDIA_STOPPED) return DEWERR_MM_ACTION;

    _bNoMoreData = FALSE;
    mmr = waveOutOpen(&_hWO, WAVE_MAPPER, &_wfex, (DWORD)_WAVHandler, (DWORD)this, CALLBACK_FUNCTION);
    if (mmr != MMSYSERR_NOERROR) return DEWERR_ENGINE_STARTUP;
    WaitForSingleObject(_hEvtPrepare, INFINITE);

    for (index = 0; index < CHUNKCOUNT; index++)
    {
        _blocks[index].lpData = (LPSTR)&(_lpDecodedData[index * CHUNKSIZE]);
        _blocks[index].dwBufferLength = CHUNKSIZE;
        _blocks[index].dwUser = index;
        _blocks[index].dwFlags = WHDR_DONE;
        _dwCurrentIndex = index;
        decodeRet = this->_Decode();
        if (decodeRet == DEWDEC_DECODE_ERROR) return DEWERR_ENGINE_STARTUP;
        mmr = waveOutPrepareHeader(_hWO, &_blocks[index], sizeof(WAVEHDR));
        if (mmr != MMSYSERR_NOERROR) return DEWERR_ENGINE_STARTUP;
        if (decodeRet == DEWDEC_DECODE_EOD) break;
    }

    _dwCurrentIndex = 0;
    ResetEvent(_hEvtDecode);
    if (decodeRet == DEWDEC_DECODE_OK)
        _hTrdPlay = CreateThread(NULL, 0, _TrdPlay, this, 0, NULL);

    for (index = 0; index < CHUNKCOUNT; index++)
    {
        mmr = waveOutWrite(_hWO, &_blocks[index], sizeof(WAVEHDR));
        if (mmr != MMSYSERR_NOERROR) return DEWERR_ENGINE_STARTUP;
    }

    _btStatus = DEWS_MEDIA_PLAYING;
    return DEWERR_SUCCESS;
}

// Generic Pause with waveOutPause()
BYTE WAVEOUTENGINE::Pause()
{
    MMRESULT    mmr = 0;
    if (_hWO == NULL || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;

    // Trigger only if the media is playing
    if (_btStatus == DEWS_MEDIA_PLAYING)
    {
        mmr = waveOutPause(_hWO);
        if (mmr != 0) return DEWERR_MM_ACTION;
        _btStatus = DEWS_MEDIA_PAUSED;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Generic Pause with waveOutRestart()
BYTE WAVEOUTENGINE::Resume()
{
    MMRESULT    mmr = 0;

    if (_hWO == NULL || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;

    // Trigger only if the media is paused
    if (_btStatus == DEWS_MEDIA_PAUSED)
    {
        mmr = waveOutRestart(_hWO);
        if (mmr != 0) return DEWERR_MM_ACTION;
        _btStatus = DEWS_MEDIA_PLAYING;
        return DEWERR_SUCCESS;
    }

    return DEWERR_MM_ACTION;
}

// Stop method. Force unload the blocks (by un-preparing the header)
// and resetting the waveOut
BYTE WAVEOUTENGINE::Stop()
{
    UINT        nIndex;
    MMRESULT    mmr = MMSYSERR_NOERROR;

    if (_hWO == NULL || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;

    // Stop can happen only if media is playing/paused
    if (_btStatus == DEWS_MEDIA_PLAYING || _btStatus == DEWS_MEDIA_PAUSED)
    {
        // If media is paused, resume & stop it.
        if (_btStatus == DEWS_MEDIA_PAUSED) this->Resume();

        // Stop the event monitoring play thread
        if (_hTrdPlay)
        {
            TerminateThread(_hTrdPlay, 0x00);
            _hTrdPlay = NULL;
        }
        // Reset the monitoring events
        ResetEvent(_hEvtPrepare);
        ResetEvent(_hEvtDecode);

        // Flush out any decoded data. This will introduce silence
        // even if the device keeps playing
        ZeroMemory(_lpDecodedData, CHUNKSIZE * CHUNKCOUNT);
        for (nIndex = 0; nIndex < CHUNKCOUNT; nIndex++)
        {
            // Unprepare the headers
            mmr = waveOutUnprepareHeader(_hWO, &(_blocks[nIndex]), sizeof(WAVEHDR));
            // Ignore it as the thread has been terminated already ;)
            if (mmr != 0 && mmr != WAVERR_STILLPLAYING) return DEWERR_MM_ACTION;
        }
        _bNoMoreData = TRUE;
        _dwCurrentPos = 0;

        // Set the position to 0 BEFORE setting the status. This will make sure
        // that the Seek works (and does not ignore becuse the status is stopped)
        this->Seek(0);
        // Close the entire engine. A subsequent Play() will re-intiate the engine
        mmr = waveOutReset(_hWO);
        if (mmr != MMSYSERR_NOERROR) return DEWERR_MM_ACTION;

        mmr = waveOutClose(_hWO);
        if (mmr != MMSYSERR_NOERROR) return DEWERR_MM_ACTION;

        _hWO = NULL;
        _btStatus = DEWS_MEDIA_STOPPED;
        return DEWERR_SUCCESS;
    }

    return DEWERR_MM_ACTION;
}

// WARNING: Engine implementor classes must not override this function!
BYTE WAVEOUTENGINE::Seek(DWORD dwMS)
{
    BYTE    btSeekRet = DEWERR_MM_ACTION;

    // Stop the decoding. For many decoders, this creates a problem
    // because seeking while decoding, throws off the decoder from 
    // its decoding sample position and causes a crash.
    _bStopDecoding = TRUE;
    // Flush out the current buffers
    ZeroMemory(_lpDecodedData, CHUNKSIZE * CHUNKCOUNT);
    _dwOverflow = 0; // Just ensure no trailing pieces are carried along
    // Get the return value
    btSeekRet = this->_Seek(dwMS);
    // No matter of the return, flip back the decoding switch
    _bStopDecoding = FALSE;
    // And then send across the result of the original seek
    return btSeekRet;
}

// Expects volume in the range of 0x0000 - 0xFFFF (WORD)
BYTE WAVEOUTENGINE::SetVolume(WORD wLevel)
{
    MMRESULT    mmr = MMSYSERR_NOERROR;
    if (_hWO == NULL || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;
    mmr = waveOutSetVolume(_hWO, (DWORD)MAKELONG(wLevel, wLevel));
    if (mmr != MMSYSERR_NOERROR) return DEWERR_MM_ACTION;
    return DEWERR_SUCCESS;
}

// Stop & reset everything. Complete the blocks and shutdown the waveOut engine
// and also unload the decoded file data to release the blocked memoty
BYTE WAVEOUTENGINE::Unload()
{
    BYTE        btRet = DEWERR_SUCCESS;
    MMRESULT    mmr = 0;

    // To be called from each Implementor! The cleanup needed for the memories created in Lod() function.
    // In addition to whatever the specific decoder does, the engine
    // needs to be closed out, if media is already playing/paused
    if (_btStatus == DEWS_MEDIA_PLAYING || _btStatus == DEWS_MEDIA_PAUSED)
        btRet = this->Stop();
    if (btRet != DEWERR_SUCCESS) return DEWERR_ENGINE_SHUTDOWN;
    return DEWERR_SUCCESS;
}

// Housekeeping & cleanup
WAVEOUTENGINE::~WAVEOUTENGINE()
{
    this->_Cleanup();
    return;
}

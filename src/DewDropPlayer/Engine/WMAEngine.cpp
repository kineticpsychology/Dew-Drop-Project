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

#include "WMAEngine.h"

// Cleanups
void WMAENGINE::_Cleanup()
{
    if (_pSyncReader) _pSyncReader->Close();
    SafeRelease(&_pSyncReader);
    SafeFree(&_lpDecodedChunk);
    return;
}

// Main decoding logic
BYTE WMAENGINE::_Decode()
{
    HRESULT     hr = S_OK;
    QWORD       qnSampleTime = 0, qnSampleDuration = 0;
    DWORD       dwGSNFlags = 0, dwOutputNum = 0;
    WORD        wStreamNum = 0;
    INSSBuffer  *pSampleBuff = NULL;
    LPBYTE      lpWAVBuff = NULL;
    DWORD       dwDecoded = 0, dwCumulDecoded = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
    {
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
        SafeFree(&_lpDecodedChunk);
    }

    // Same elongated COM handling.
    do
    {
        hr = _pSyncReader->GetNextSample(_wStreamIdx, &pSampleBuff, &qnSampleTime,
                                         &qnSampleDuration, &dwGSNFlags,
                                         &dwOutputNum, &wStreamNum);
        // Keep updating the current time variable
        _dwCurrTime = (DWORD)((double)qnSampleTime / 10000);
        
        if (SUCCEEDED(hr))
        {
            pSampleBuff->GetBufferAndLength(&lpWAVBuff, &dwDecoded);
            if (dwDecoded == 0)
            {
                _bNoMoreData = TRUE;
                SafeRelease(&pSampleBuff);
                return DEWDEC_DECODE_EOD;
            }
            // Same code to check and handle overflow.
            // Check the MP3/OPUS/VORBIS Engine implementaions to understand how this is handled
            if ((dwCumulDecoded + dwDecoded) >= CHUNKSIZE)
            {
                _dwOverflow = (dwCumulDecoded + dwDecoded) - CHUNKSIZE;
                _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, _dwOverflow);
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), lpWAVBuff, CHUNKSIZE - dwCumulDecoded);
                CopyMemory(_lpDecodedChunk, &(lpWAVBuff[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
                SafeRelease(&pSampleBuff);
                return DEWDEC_DECODE_OK;
            }
            else
            {
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), lpWAVBuff, dwDecoded);
                dwCumulDecoded += dwDecoded;
                SafeRelease(&pSampleBuff);
            }
        }
        // Any other error - bail.
        else if (hr == NS_E_NO_MORE_SAMPLES)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        // The above block could be combined here. But still kept the
        // 'No more samples' as a separate case.
        else
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
    } while (SUCCEEDED(hr));

    return DEWDEC_DECODE_ERROR;
}

// Go through another round of DLL/COM chain to get the extra metadata
void WMAENGINE::_SetExtendedInfo()
{
    IWMHeaderInfo2      *pHdrInfo2 = NULL;
    DWORD               dwCodecInfoCount = 0, dwIdx;
    HRESULT             hr = S_OK;
    WORD                sizeName = 0;
    WORD                sizeDesc = 0;
    WORD                sizeInfo = 0;
    WMT_CODEC_INFO_TYPE codecInfoType;
    wchar_t             *buffName = NULL, *buffDesc = NULL;
    LPBYTE              buffInfo = NULL;
    wchar_t             wsValue[EXINFO_VAL_SIZE] { 0 };

    _dwExInfoLength = 3;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"WM Stream Number");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _wStreamIdx);
    _lpExInfo[0].SetValue(wsValue);

    _lpExInfo[1].SetType(L"WM Output Number");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _dwOutNum);
    _lpExInfo[1].SetValue(wsValue);

    _lpExInfo[2].SetType(L"WM Codec (Encoder)");

    hr = _pSyncReader->QueryInterface(IID_IWMHeaderInfo2, (LPVOID*)&pHdrInfo2);
    if (pHdrInfo2 && SUCCEEDED(hr))
    {
        hr = pHdrInfo2->GetCodecInfoCount(&dwCodecInfoCount);
        if (SUCCEEDED(hr))
        {
            for (dwIdx = 0; dwIdx < dwCodecInfoCount; dwIdx++)
            {
                hr = pHdrInfo2->GetCodecInfo(dwIdx, &sizeName, NULL, &sizeDesc,
                    NULL, &codecInfoType, &sizeInfo, NULL);
                if (SUCCEEDED(hr) && codecInfoType == WMT_CODECINFO_AUDIO)
                {
                    buffName = (wchar_t*)LocalAlloc(LPTR, (sizeName + 1) * sizeof(wchar_t));
                    buffDesc = (wchar_t*)LocalAlloc(LPTR, (sizeDesc + 1) * sizeof(wchar_t));
                    buffInfo = (LPBYTE)LocalAlloc(LPTR, sizeInfo);

                    hr = pHdrInfo2->GetCodecInfo(dwIdx, &sizeName, buffName, &sizeDesc,
                        buffDesc, &codecInfoType, &sizeInfo, buffInfo);
                    if (lstrlen(buffDesc) <= 0 && lstrlen(buffName) > 0 && SUCCEEDED(hr))
                    {
                        StringCchPrintf(wsValue, EXINFO_VAL_SIZE, buffName);
                    }
                    else if (lstrlen(buffDesc) > 0 && lstrlen(buffName) <= 0 && SUCCEEDED(hr))
                    {
                        StringCchPrintf(wsValue, EXINFO_VAL_SIZE, buffDesc);
                    }
                    else if (lstrlen(buffName) > 0 && lstrlen(buffDesc) > 0 && SUCCEEDED(hr))
                    {
                        StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%s (%s)", buffName, buffDesc);
                    }
                    _lpExInfo[2].SetValue(wsValue);
                    SafeFree(&buffInfo);
                    SafeFree(&buffDesc);
                    SafeFree(&buffName);
                }
            }
        }
    }
    SafeRelease(&pHdrInfo2);
    return;
}

// Use the COM-based function to seek to the required position
BYTE WMAENGINE::_Seek(DWORD dwMS)
{
    HRESULT     hr;
    if (!_pSyncReader) return DEWERR_MM_ACTION;

    // Don't Question. Just seek to the start
    if (_bStopInitiatedSeek)
    {
        _bStopInitiatedSeek = FALSE;
        ZeroMemory(_lpDecodedData, CHUNKSIZE * CHUNKCOUNT);
        _dwCurrTime = 0;
        hr = _pSyncReader->SetRange(0, 0);
        if (!SUCCEEDED(hr)) return DEWERR_MM_ACTION;
        _bNoMoreData = TRUE;
        return DEWERR_SUCCESS;
    }

    _dwCurrTime = _dwDuration;
    if (_dwDuration == 0) return DEWERR_SUCCESS;
    if (dwMS <= 0 || dwMS >= _dwDuration - 1) return DEWERR_SUCCESS;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        hr = _pSyncReader->SetRange(dwMS * 10000, 0);
        if (SUCCEEDED(hr))
        {
            _dwCurrTime = dwMS;
            return DEWERR_SUCCESS;
        }
    }
    return DEWERR_MM_ACTION;

}

// Basic information setup and media type
// We'll still setup the library name/version from the 'Load' function.
// In the meanwhile, this string will serve as a 'generic' stop-gap
WMAENGINE::WMAENGINE()
{
    _btMediaType = DEWMT_WMA;
    CoInitialize(NULL);
    StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK");
}

// Main load. Please do yourself a favor & go over the MSDN docs to understand
// the flow below. I won't comment on anything else but will definitely say
// that MSDN is "THE BEST" documentation I've seen as a developer!
BYTE WMAENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    const DWORD         MAX_STREAM_COUNT = 0x40;
    IWMProfile          *pWMProfile = NULL;
    IWMStreamConfig     *pStreamConfig = NULL;
    IWMMediaProps       *pMediaProps = NULL;
    IWMOutputMediaProps *pOutMediaProps = NULL;
    IWMHeaderInfo       *pHeaderInfo = NULL;
    WM_MEDIA_TYPE       *pMediaType = NULL;
    DWORD               dwMediaSize = 0;
    WORD                wIdx = 0;
    LPBYTE              lpBuffer = NULL;
    HRESULT             hr = S_OK;
    GUID                streamType { 0 };
    BOOL                bCompressed = FALSE;
    WORD                wAttribStream = 0, wAttribSize = 0;
    WMT_ATTR_DATATYPE   attrDataType;
    LPBYTE              lpAttrData = NULL;
    BYTE                nError;
    MMRESULT            mmr = 0;
    WMT_VERSION         dwLibVer = WMT_VER_4_0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    hr = WMCreateSyncReader(NULL, 0, &_pSyncReader);
    if (FAILED(hr)) return DEWERR_ENGINE_STARTUP;
    
    hr = _pSyncReader->QueryInterface(IID_IWMProfile, (LPVOID*)&pWMProfile);
    if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
    hr = _pSyncReader->Open(srcFile);
    if (FAILED(hr)) { nError = DEWERR_FILE_READ; goto __ctor_done; }

    hr = pWMProfile->GetVersion(&dwLibVer);
    if (SUCCEEDED(hr))
    {
        switch(dwLibVer)
        {
            case WMT_VER_4_0:
                 StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK version 4");
                 break;
            case WMT_VER_7_0:
                 StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK version 7");
                 break;
            case WMT_VER_8_0:
                 StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK version 8");
                 break;
            case WMT_VER_9_0:
                 StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK version 9.0 / 9.5");
                 break;
            default:
                 StringCchPrintf(_wsLibrary, MAX_PATH, L"Windows Media Format SDK");
                 break;
        }
    }
    // Set the error code. We'll reset it only if a WMMEDIATYPE_Audio is found
    nError = DEWERR_INVALID_FILE;
    for (wIdx = 1; wIdx <= MAX_STREAM_COUNT; wIdx++)
    {

        hr = pWMProfile->GetStreamByNumber(wIdx, &pStreamConfig);
        if (FAILED(hr)) { nError = DEWERR_INVALID_FILE; goto __ctor_done; }
        hr = pStreamConfig->GetStreamType(&streamType);
        if (FAILED(hr)) { nError = DEWERR_INVALID_FILE; goto __ctor_done; }

        if (streamType == WMMEDIATYPE_Audio)
        {
            hr = pStreamConfig->QueryInterface(IID_IWMMediaProps, (LPVOID*)&pMediaProps);
            if (FAILED(hr)) { nError = DEWERR_INVALID_FILE; goto __ctor_done; }
            hr = pMediaProps->GetMediaType(NULL, &dwMediaSize);
            if (FAILED(hr)) { nError = DEWERR_INVALID_FILE; goto __ctor_done; }

            lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwMediaSize);
            pMediaType = reinterpret_cast<WM_MEDIA_TYPE*>(lpBuffer);
            hr = pMediaProps->GetMediaType(pMediaType, &dwMediaSize);
            if (FAILED(hr)) { nError = DEWERR_INVALID_FILE; goto __ctor_done; }
            if (pMediaType->formattype == WMFORMAT_WaveFormatEx)
            {
                CopyMemory(&_wfex, pMediaType->pbFormat, sizeof(WAVEFORMATEX));
                _wStreamIdx = wIdx;
            }
            SafeFree(&lpBuffer);

            hr = _pSyncReader->SetReadStreamSamples(_wStreamIdx, bCompressed);
            if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
            hr = _pSyncReader->GetReadStreamSamples(_wStreamIdx, &bCompressed);
            if (bCompressed) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }

            _pSyncReader->GetOutputNumberForStream(_wStreamIdx, &_dwOutNum);

            _wfex.cbSize = 0;
            _wfex.wFormatTag = WAVE_FORMAT_PCM;
            _wfex.wBitsPerSample = 16;
            _wfex.nBlockAlign = (_wfex.wBitsPerSample * _wfex.nChannels)/8;
            _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

            hr = _pSyncReader->GetOutputProps(_dwOutNum, &pOutMediaProps);
            if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
            hr = pOutMediaProps->GetMediaType(NULL, &dwMediaSize);
            lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwMediaSize);
            pMediaType = reinterpret_cast<WM_MEDIA_TYPE*>(lpBuffer);
            hr = pOutMediaProps->GetMediaType(pMediaType, &dwMediaSize);
            if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
            if (pMediaType->formattype == WMFORMAT_WaveFormatEx)
            {
                CopyMemory(pMediaType->pbFormat, &_wfex, sizeof(WAVEFORMATEX));
                hr = pOutMediaProps->SetMediaType(pMediaType);
                if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
                hr = _pSyncReader->SetOutputProps(_dwOutNum, pOutMediaProps);
                if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
                SafeFree(&lpBuffer);

                hr = _pSyncReader->QueryInterface(IID_IWMHeaderInfo, (LPVOID*)&pHeaderInfo);
                if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
                hr = pHeaderInfo->GetAttributeByName(&wAttribStream, g_wszWMDuration, &attrDataType, NULL, &wAttribSize);
                if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
                if (attrDataType == WMT_TYPE_QWORD && wAttribSize == sizeof(QWORD))
                {
                    lpAttrData = (LPBYTE)LocalAlloc(LPTR, wAttribSize);
                    hr = pHeaderInfo->GetAttributeByName(&wAttribStream, g_wszWMDuration, &attrDataType, lpAttrData, &wAttribSize);
                    _dwDuration = (DWORD)((*((QWORD*)lpAttrData)) / 10000);
                }

                hr = pStreamConfig->GetBitrate(&_dwBitrate);
                hr = _pSyncReader->GetMaxOutputSampleSize(_dwOutNum, &_dwMaxOverflow);
                if (FAILED(hr)) { nError = DEWERR_ENGINE_STARTUP; goto __ctor_done; }
                _btChannels = (BYTE)_wfex.nChannels;
                _dwSampleRate = _wfex.nSamplesPerSec;

                _SetExtendedInfo();
                nError = DEWERR_SUCCESS;
                break;
            }

        }
    }

__ctor_done:
    SafeFree(&lpAttrData);
    SafeRelease(&pHeaderInfo);
    SafeFree(&lpBuffer);
    SafeRelease(&pOutMediaProps);
    SafeRelease(&pMediaProps);
    SafeRelease(&pStreamConfig);
    SafeRelease(&pWMProfile);

    if (nError == DEWERR_SUCCESS)
    {
        mmr = waveOutOpen(&_hWO, WAVE_MAPPER, &_wfex, (DWORD_PTR)_WAVHandler, (DWORD_PTR)this, CALLBACK_FUNCTION);
        if (mmr != 0) return DEWERR_ENGINE_STARTUP;
        _btStatus = DEWS_MEDIA_LOADED;
        _bStopInitiatedSeek = FALSE;
    }
    return nError;

}

// We won't support loading wma from memory
BYTE WMAENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}


// Forcibly drag the internal decoder head to the beginning.
BYTE WMAENGINE::Stop()
{
    _bStopInitiatedSeek = TRUE;
    return WAVEOUTENGINE::Stop();
}

// Simply provide the variable that is keeping track of the time.
DWORD WMAENGINE::Tell()
{
    return _dwCurrTime;
}

// Housekeeping and cleanup
WMAENGINE::~WMAENGINE()
{
    this->_Cleanup();
    return;
}

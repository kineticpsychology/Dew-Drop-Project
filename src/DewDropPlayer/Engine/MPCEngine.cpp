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

#include "MPCEngine.h"

// I don't know whatever the fuck is happening here. Ripped stratight
// from the library code (libwavformat/output.c)
// Looks like float is being split into two halves based on signed integer
void MPCENGINE::_Float32ToInt16(float const *pSampleBuffer, LPBYTE lpOut, UINT nSampleCount)
{
    UINT        n;
    for (n = 0; n < nSampleCount; n++)
    {
        int tmp = (int)(pSampleBuffer[n] * 0x8000);
        if (tmp < -0x8000) tmp = -0x8000;
        else if (tmp > 0x7FFF) tmp = 0x7FFF;
        *(lpOut++) = (BYTE)((UINT)tmp);
        *(lpOut++) = (BYTE)((UINT)tmp >> 8);
    }
    return;
}

// Release memory and shutdown the demuxer/reader
void MPCENGINE::_Cleanup()
{
    if (_lpDecodedChunk)
    {
        LocalFree(_lpDecodedChunk);
        _lpDecodedChunk = NULL;
    }
    if (_pMpcDemux) mpc_demux_exit(_pMpcDemux);
    mpc_reader_exit_stdio(&_mpcReader);
    if (_fpMPC) fclose(_fpMPC);
    return;
}

// The core decoding
BYTE MPCENGINE::_Decode()
{
    mpc_status      mpcStatus;
    mpc_frame_info  frame;
    DWORD           dwDecoded = 0, dwLastPos = 0;
    UINT            nIdx = 0, nTargetBytes = 0;
    LPBYTE          lpSampleMap = NULL;
    DWORD           dwToTransfer = 0;
    
    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    // Copy whatever was leftover from the last iteration
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    if (_dwOverflow > 0 && _lpDecodedChunk)
    {
        dwLastPos = _dwOverflow;
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
    }
    
    // Keep parsing frames & their samples CHUNKSIZE overflows
    while (MPC_TRUE)
    {
        frame.buffer = _sampleBuffer;
        mpcStatus = mpc_demux_decode(_pMpcDemux, &frame);

        // End of stream. Bail.
        if (frame.bits == -1)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        
        // Samples are counted once (irrespective of channel count)
        // Loop over each sample of each frame.
        for (nIdx = 0; nIdx < frame.samples * _btChannels; nIdx += MPC_BUFF_SAMPLE_SIZE)
        {
            _Float32ToInt16(&_sampleBuffer[nIdx], (LPBYTE)_iDecodedBuff, MPC_BUFF_SAMPLE_SIZE);
            lpSampleMap = (LPBYTE)_iDecodedBuff;
            // Keep copying the decoded bytes into the decoding chunk
            // Do not worry about overflow right now. We'll do it after all the
            // samples of the current frame are decoded.
            // Pick the lower of total samples decoded so far vs  the target for this frame
            nTargetBytes = frame.samples * _btChannels * _nBytesPerSample;
            dwToTransfer = ((nIdx + MPC_BUFF_SAMPLE_SIZE) * _nBytesPerSample > nTargetBytes ? nTargetBytes - (nIdx * _nBytesPerSample) : MPC_DECODED_BUFF_SIZE);
            CopyMemory(&(_lpDecodedChunk[dwDecoded]), lpSampleMap, dwToTransfer);
            dwDecoded += dwToTransfer;
        }

        // Overflow has happened
        if ((dwDecoded + dwLastPos) >= CHUNKSIZE)
        {
            // Calculate the 'Overflow' size
            _dwOverflow = (dwDecoded + dwLastPos) - CHUNKSIZE;
            // Copy whatever can fit into the decoding block within CHUNKSIZE
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwLastPos]), _lpDecodedChunk, (CHUNKSIZE - dwLastPos));
            // Keep the remaining stored in the decoding chunk for the next iteration
            CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwLastPos]), _dwOverflow);
            return DEWDEC_DECODE_OK;
        }
    }
    return DEWDEC_DECODE_EOD;
}

// Simple seeking based on the library provided method
BYTE MPCENGINE::_Seek(DWORD dwMS)
{
    mpc_status      mpcStatus = MPC_STATUS_OK;
    double          dSeekSample = 0;

    // Do not allow any seekin unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        if (dwMS > _dwDuration)
            dwMS = _dwDuration;
        if (_dwDuration == 0) return DEWERR_SUCCESS;
        
        if (dwMS == _dwDuration)
            dSeekSample = (double)_nTotalSamples;
        else
            dSeekSample = ((double)_nTotalSamples * (double)dwMS) / (double)_dwDuration;
        mpcStatus = mpc_demux_seek_sample(_pMpcDemux, (mpc_uint64_t)dSeekSample);
        if (mpcStatus != MPC_STATUS_OK) return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Some basic initialization
MPCENGINE::MPCENGINE()
{
    ZeroMemory(&_mpcReader, sizeof(mpc_reader));
    _btMediaType = DEWMT_MPC;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"musepack (libmpcdec) version r475");
    // One frame cannot have more that 4608 (MPC_DECODER_BUFFER_LENGTH) bytes decoded.
    // So the absolute max of overflow that can happen is CHUNKSIZE + 4608
    _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, CHUNKSIZE + MPC_DECODER_BUFFER_LENGTH);
}

// We'll support loading only through file
BYTE MPCENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    mpc_status      mpcStatus;
    mpc_streaminfo  mpcSI { 0 };
    wchar_t         wsValue[EXINFO_VAL_SIZE];
    DWORD           dwLen;
    MMRESULT        mmr = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;

    // Overloaded call with source media data bytes is not supported
    _bOverloadedLoadFunctionCall = FALSE;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, srcFile);

    if (_wfopen_s(&_fpMPC, _wsSrcFile, L"rb")) return DEWERR_FILE_READ;
    mpcStatus = mpc_reader_init_stdio_stream(&_mpcReader, _fpMPC);
    if (mpcStatus != MPC_STATUS_OK) return DEWERR_FILE_READ;

    _pMpcDemux = mpc_demux_init(&_mpcReader);
    if (!_pMpcDemux) return DEWERR_FILE_READ;
    mpc_demux_get_info(_pMpcDemux, &mpcSI);

    _dwSampleRate = mpcSI.sample_freq;
    _btChannels = mpcSI.channels;
    _dwBitrate = (DWORD)mpcSI.average_bitrate;
    _nTotalSamples = mpcSI.samples;
    _dwDuration = (DWORD)((float)mpcSI.samples/((float)mpcSI.sample_freq/1000.0f));

    // Setup the WAAVEFORMATEX structure
    _wfex.cbSize = 0;
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.wBitsPerSample = 16; // Hard-set the bit depth to 16
    _wfex.nChannels = _btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;
    _nBytesPerSample = _wfex.wBitsPerSample / 8; // Bytes per sample. Needed while decoding

    // We have a lot fo metadata for this :)
    _dwExInfoLength = 9;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"Stream Version");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.stream_version);
    _lpExInfo[0].SetValue(wsValue);

    _lpExInfo[1].SetType(L"Gapless");
    _lpExInfo[1].SetValue(((mpcSI.is_true_gapless == 1) ? L"Yes" : L"No"));

    _lpExInfo[2].SetType(L"Encoder Version");
    dwLen = lstrlenA(mpcSI.encoder) > (EXINFO_VAL_SIZE - 3) ? (EXINFO_VAL_SIZE - 1) : lstrlenA(mpcSI.encoder);
    MultiByteToWideChar(CP_UTF8, 0, mpcSI.encoder, dwLen, wsValue, dwLen);
    wsValue[dwLen] = L'\0';
    _lpExInfo[2].SetValue(wsValue);

    _lpExInfo[3].SetType(L"Encoder Profile");
    dwLen = lstrlenA(mpcSI.profile_name) > 61 ? 61 : lstrlenA(mpcSI.profile_name);
    MultiByteToWideChar(CP_UTF8, 0, mpcSI.profile_name, dwLen, wsValue, dwLen);
    wsValue[dwLen] = L'\0';
    _lpExInfo[3].SetValue(wsValue);

    _lpExInfo[4].SetType(L"Replay Gain (Title)");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.gain_title);
    _lpExInfo[4].SetValue(wsValue);

    _lpExInfo[5].SetType(L"Replay Gain (Album)");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.gain_album);
    _lpExInfo[5].SetValue(wsValue);

    _lpExInfo[6].SetType(L"Peak Loudness (Title)");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.peak_title);
    _lpExInfo[6].SetValue(wsValue);

    _lpExInfo[7].SetType(L"Peak Loudness (Album)");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.peak_album);
    _lpExInfo[7].SetValue(wsValue);

    _lpExInfo[8].SetType(L"Maximum Bands");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", mpcSI.max_band);
    _lpExInfo[8].SetValue(wsValue);

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// This mode of media loading is not implemented
BYTE MPCENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// This is a bad implementation. Feel free to introduce anything fancy
DWORD MPCENGINE::Tell()
{
    // Get the total bits so far and divide by the bitrate :P
    if (_pMpcDemux)
        return (DWORD)((float)mpc_demux_pos(_pMpcDemux) / ((float)_dwBitrate / 1000.0f));
    else
        return 0;
}

// Housekeeping
MPCENGINE::~MPCENGINE()
{
    this->_Cleanup();
    return;
}

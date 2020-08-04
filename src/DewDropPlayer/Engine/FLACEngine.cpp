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

#include "FLACEngine.h"

#pragma region Stub methods required for dabbling (IsLoadable() methods)
// These are temporarilty declared. Will be called only when trying to
// open/test files
static FLAC__StreamDecoderWriteStatus _tmpWriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
static void _tmpErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
static void _tmpMetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);

static FLAC__StreamDecoderWriteStatus _tmpWriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data) { return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE; }
static void _tmpErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) { return; }
static void _tmpMetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) { return; }

#pragma endregion

#pragma region Main Static Callbacks

// Part 1 of amin decoding logic
// Called back everytime a FLAC__stream_decode_single() is called
FLAC__StreamDecoderWriteStatus FLACENGINE::_writeCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    LPFLACENGINE    lpFlacEngine;
    size_t          iIdxBlock, iIdChannel;
    LPBYTE          lpIndicator = NULL;

    // Bring out the class pointer
    lpFlacEngine = (LPFLACENGINE)client_data;
    lpFlacEngine->_dwDecodedChunk = 0;
    // The array is set in sets of channels
    for (iIdxBlock = 0; iIdxBlock < frame->header.blocksize; iIdxBlock++)
    {
        for (iIdChannel = 0; iIdChannel < lpFlacEngine->_btChannels; iIdChannel++)
        {
            lpIndicator = (LPBYTE)&(buffer[iIdChannel][iIdxBlock]);
            lpIndicator += (lpFlacEngine->_wBitsPerSample - lpFlacEngine->_ENGINE_BITS_PER_SAMPLE) / 8;
            CopyMemory(&(lpFlacEngine->_lpDecodedChunk[(iIdxBlock * lpFlacEngine->_ENGINE_BITS_PER_SAMPLE / 8 * lpFlacEngine->_btChannels) + (iIdChannel * lpFlacEngine->_ENGINE_BITS_PER_SAMPLE / 8)]),
                       lpIndicator, lpFlacEngine->_ENGINE_BITS_PER_SAMPLE / 8);
        }
    }

    lpFlacEngine->_dwDecodedChunk = frame->header.blocksize * lpFlacEngine->_btChannels * lpFlacEngine->_ENGINE_BITS_PER_SAMPLE / 8;

    // Needed for being able to track the rough sample, so we can tell the duration covered so far
    if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER)
        lpFlacEngine->_u64CurrentSample = frame->header.number.sample_number;
    // Just a final check to see if the stream has completed.
    // If so, set the data end marker.
    if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
        lpFlacEngine->_bNoMoreData = TRUE;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// There's really nothing that can be done at this point
void FLACENGINE::_errorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    return;
}

// Collect the WAVEFORMATEX's data
void FLACENGINE::_metadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
    LPFLACENGINE        lpFlacEngine = NULL;
    lpFlacEngine = (LPFLACENGINE)client_data;

    // Grab everything from the STREAMINFO structure except the bitrate
    //Bitrate will be calculated in the main Load method
    lpFlacEngine->_wBitsPerSample = metadata->data.stream_info.bits_per_sample;
    lpFlacEngine->_btChannels = metadata->data.stream_info.channels;
    lpFlacEngine->_dwSampleRate = metadata->data.stream_info.sample_rate;
    lpFlacEngine->_u64TotalSamples = metadata->data.stream_info.total_samples;
    lpFlacEngine->_dwMaxDecodedSize = (metadata->data.stream_info.max_blocksize) *
                                     (lpFlacEngine->_btChannels) *
                                     (lpFlacEngine->_wBitsPerSample) / 8;
    lpFlacEngine->_dwDuration = (DWORD)round(((double)lpFlacEngine->_u64TotalSamples) / ((double)lpFlacEngine->_dwSampleRate/1000.0f));
    lpFlacEngine->_lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, lpFlacEngine->_dwMaxDecodedSize);
    return;
}

#pragma endregion

// Flush out everything (decoder, memories, file pointers)
void FLACENGINE::_Cleanup()
{
    if (_lpDecodedChunk)
    {
        LocalFree(_lpDecodedChunk);
        _lpDecodedChunk = NULL;
    }
    if (_pFlacDec)
    {
        FLAC__stream_decoder_finish(_pFlacDec);
        FLAC__stream_decoder_flush(_pFlacDec);
        FLAC__stream_decoder_reset(_pFlacDec);
        FLAC__stream_decoder_delete(_pFlacDec);
        _pFlacDec = NULL;
    }
    if (_fpFlac)
    {
        fclose(_fpFlac);
        _fpFlac = NULL;
    }
    return;
};

// Part 2 of decoding 
BYTE FLACENGINE::_Decode()
{
    DWORD       dwCumulDecoded = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData)
    {
        _u64CurrentSample = 0;
        return DEWDEC_DECODE_EOD;
    }
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
    {
        // Now there might be a case where _dwOverflow > CHUNKSIZE (if so defined!)
        if (_dwOverflow > CHUNKSIZE)
        {
            // Even after copying over CHUNKSIZE, we are still left with data!
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, CHUNKSIZE);
            // We are now left with _dwOverflow - CHUNKSIZE data
            CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE]), (_dwOverflow - CHUNKSIZE));
            _dwOverflow -= CHUNKSIZE;
            // Let's move to the next iteration, to handle that trailing data
            return DEWDEC_DECODE_OK;
        }
        else
        {
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
        }
    }
    dwCumulDecoded = _dwOverflow;
    
    // Decode one frame at a time and keep continuing
    // until the CHUNKSIZE fills/just spills
    do
    {
        if (!FLAC__stream_decoder_process_single(_pFlacDec))
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        if (FLAC__stream_decoder_get_state(_pFlacDec) == FLAC__STREAM_DECODER_END_OF_STREAM)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        // Callback will populate dwDecodedChunk amount of data into lpDecodedChunk
        if ((dwCumulDecoded + _dwDecodedChunk) > CHUNKSIZE)
        {

            // Keep the overflow amount in the lpDecodedChunk buffer
            _dwOverflow = (dwCumulDecoded + _dwDecodedChunk) - CHUNKSIZE;
            if (_dwOverflow > 0)
            {
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, CHUNKSIZE - dwCumulDecoded);
                CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
            }
            return DEWDEC_DECODE_OK;
        }
        else
        {
            _dwOverflow = 0;
            // Keep dumping the data into the lpDecodedData buffer until
            // it fills up to the brim or overflows
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, _dwDecodedChunk);
            dwCumulDecoded += _dwDecodedChunk;
        }
        _dwDecodedChunk = 0;
    } while (TRUE);
    return DEWDEC_DECODE_ERROR;
}

// Use the library-provided method to seek
BYTE FLACENGINE::_Seek(DWORD dwMS)
{
    FLAC__uint64    u64SeekSample = 0;
    double          dDuration = 0, dSeekTime = 0, dSeekSample = 0;

    // Do not allow any seekin unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwMS;

        // Fgure out the absolte sample to seek to
        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
        {
            u64SeekSample = _u64TotalSamples;
        }
        else
        {
            dSeekSample = (dSeekTime * (double)_u64TotalSamples) / dDuration;
            u64SeekSample = (FLAC__uint64)dSeekSample;
            if (u64SeekSample > _u64TotalSamples) u64SeekSample = _u64TotalSamples;
        }
        if (!FLAC__stream_decoder_seek_absolute(_pFlacDec, u64SeekSample)) return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// Placeholder. For now assume the format is native FLAC
FLACENGINE::FLACENGINE()
{
    _btMediaType = DEWMT_FLAC;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libFLAC version 1.3.3");
}

// Static method to dabble and evaluate if file contains a native FLAC stream
BOOL FLACENGINE::IsLoadable(LPCWSTR testFile)
{
    FLAC__StreamDecoder     *tmpFlacDec = NULL;
    FILE                    *tmpFP = NULL;
    FLAC__StreamDecoderInitStatus tmpDecStatus;

    if (!testFile) return FALSE;
    tmpFlacDec = FLAC__stream_decoder_new();
    if (!tmpFlacDec) return FALSE;
    _wfopen_s(&tmpFP, testFile, L"rb");
    if (!tmpFP) return FALSE;
    tmpDecStatus = FLAC__stream_decoder_init_FILE(tmpFlacDec, tmpFP, _writeCallback, _metadataCallback, _errorCallback, NULL);
    FLAC__stream_decoder_delete(tmpFlacDec);
    fclose(tmpFP);
    return (tmpDecStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK);
}

// Static method to dabble and evaluate if an ogg container contains a FLAC stream
BOOL FLACENGINE::IsOggLoadable(LPCWSTR testFile)
{
    FLAC__StreamDecoder     *tmpFlacDec = NULL;
    FILE                    *tmpFP = NULL;
    FLAC__StreamDecoderInitStatus tmpDecStatus;

    if (!testFile) return FALSE;
    tmpFlacDec = FLAC__stream_decoder_new();
    if (!tmpFlacDec) return FALSE;
    _wfopen_s(&tmpFP, testFile, L"rb");
    if (!tmpFP) return FALSE;
    tmpDecStatus = FLAC__stream_decoder_init_ogg_FILE(tmpFlacDec, tmpFP, _writeCallback, _metadataCallback, _errorCallback, NULL);
    FLAC__stream_decoder_delete(tmpFlacDec);
    fclose(tmpFP);
    return (tmpDecStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK);
}

// Main Load method.
BYTE FLACENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    FLAC__StreamDecoderInitStatus       status;
    MMRESULT                            mmr = 0;
    DWORD                               dwSignature = 0;
    const DWORD                         FLAC_FLAC = 0x43614C66;
    const DWORD                         FLAC_OGG = 0x5367674F;
    DWORD                               dwSrcFileSize = 0;
    HANDLE                              hBRSrcFile = NULL;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    // Get the file size just to get the bitrate
    hBRSrcFile = CreateFile(_wsSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hBRSrcFile == NULL || hBRSrcFile == INVALID_HANDLE_VALUE) return DEWERR_FILE_READ;
    dwSrcFileSize = GetFileSize(hBRSrcFile, NULL);
    CloseHandle(hBRSrcFile);

    // Initialize decoder engine
    _pFlacDec = FLAC__stream_decoder_new();
    if (!_pFlacDec) return DEWERR_ENGINE_STARTUP;
    _wfopen_s(&_fpFlac, _wsSrcFile, L"rb");
    if (!_fpFlac) return DEWERR_FILE_READ;

    // Seek to the file's beginning to get the initial signature (4) bytes
    // re-use the FILE* pointer to reduce the IO
    _fseeki64(_fpFlac, 0, SEEK_SET);
    fread_s(&dwSignature, 4, 4, 1, _fpFlac);
    // After reading, make sure the file pointer is set back to the 0th offset!
    _fseeki64(_fpFlac, 0, SEEK_SET);

    // 'fLaC' - Open it in the traditional way
    if (dwSignature == FLAC_FLAC)
    {
        status = FLAC__stream_decoder_init_FILE(_pFlacDec, _fpFlac, _writeCallback, _metadataCallback, _errorCallback, this);
        if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) return DEWERR_ENGINE_STARTUP;
    }
    // 'OggS' - Open it assuming it to be an Ogg container
    else if (dwSignature == FLAC_OGG)
    {
        status = FLAC__stream_decoder_init_ogg_FILE(_pFlacDec, _fpFlac, _writeCallback, _metadataCallback, _errorCallback, this);
        if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) return DEWERR_ENGINE_STARTUP;
        
    }
    // Something else. We won't handle this
    else return DEWERR_UNSUPPORTED;

    // Gather metadata
    FLAC__stream_decoder_process_until_end_of_metadata(_pFlacDec);
    _dwBitrate = (DWORD)((double)dwSrcFileSize / (double)_dwDuration) * 8;
    
    // Prepare WAVEFORMATEX
    // No matter what, we will always prepare 16-bit stereo
    if (_wBitsPerSample == 16)
    {
        _wfex.wFormatTag = WAVE_FORMAT_PCM;
        _wfex.cbSize = 0;
        _wfex.nChannels = (WORD)_btChannels;
        _wfex.nSamplesPerSec = _dwSampleRate;
        _wfex.wBitsPerSample = _wBitsPerSample;
        _wfex.nBlockAlign = (_wfex.nChannels * _wBitsPerSample) / 8;
        _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;
    }
    else
    {
        _wfex.wFormatTag = WAVE_FORMAT_PCM;
        _wfex.cbSize = 0;
        _wfex.nChannels = (WORD)_btChannels;
        _wfex.nSamplesPerSec = _dwSampleRate;
        _wfex.wBitsPerSample = _ENGINE_BITS_PER_SAMPLE;
        _wfex.nBlockAlign = (_wfex.nChannels * _ENGINE_BITS_PER_SAMPLE) / 8;
        _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;
    }

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// This mode of loading is not supported
BYTE FLACENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Use the internal sample counter to try and predict the duration passed so far
DWORD FLACENGINE::Tell()
{
    if (_u64CurrentSample < 0)
        return (DWORD)0xFFFFFFFF; // INVALID_HANDLE_VALUE
    return (DWORD)(round(((double)_u64CurrentSample) / ((double)_dwSampleRate/1000.0f)));
}

// Housekeeping and cleanup
FLACENGINE::~FLACENGINE()
{
    this->_Cleanup();
    return;
}

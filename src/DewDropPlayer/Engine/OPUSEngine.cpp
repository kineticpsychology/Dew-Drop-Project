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

#include "OPUSEngine.h"

// Release the OggOpusFile pointer
void OPUSENGINE::_Cleanup()
{
    if (_pOOF)
    {
        op_free(_pOOF);
        _pOOF = NULL;
    }
    return;
}

// Main decoding logic
BYTE OPUSENGINE::_Decode()
{
    DWORD       dwCumulDecoded = 0, dwDecoded = 0;
    int         iIdSample, iSamples = 0;

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
        _dwOverflow = 0;
    }
    // Flush the decoding chunk just to be extra sure
    ZeroMemory(_lpDecodedChunk, OPUS_BUFF_SIZE_LE);
    do
    {
        // Keep getting the bitrate as it will vary
        _dwBitrate = op_bitrate_instant(_pOOF);
        // Flush out the sample receiver
        ZeroMemory(&_OpusSample, OPUS_BUFF_SIZE  * sizeof(opus_int16));
        // Force-demux to stereo. (Hint: that is why wfex has wChannels set to hard-coded 2
        iSamples = op_read_stereo(_pOOF, _OpusSample, sizeof(_OpusSample) / sizeof(*_OpusSample));
        if (iSamples > 0)
        {
            // The data needs to be repeated for both the channels
            // as the sample information is for one channel only
            for (iIdSample = 0; iIdSample < 2 * iSamples; iIdSample++) // *Always 2 channels. Hence the '2' multiplier
            {
                _OpusSampleLE[2 * iIdSample + 0] = (BYTE)(_OpusSample[iIdSample] & 0xFF);
                _OpusSampleLE[2 * iIdSample + 1] = (BYTE)(_OpusSample[iIdSample] >> 8 & 0xFF);
            }
            dwDecoded = sizeof(*_OpusSampleLE) * 4 * iSamples;
            if ((dwCumulDecoded + dwDecoded) >= CHUNKSIZE)
            {
                // Keep the overflow amount in the lpDecodedChunk buffer
                _dwOverflow = (dwCumulDecoded + dwDecoded) - CHUNKSIZE;
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _OpusSampleLE, CHUNKSIZE - dwCumulDecoded);
                CopyMemory(_lpDecodedChunk, &(_OpusSampleLE[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
                return DEWDEC_DECODE_OK;
            }
            else
            {
                // Not yet filled. Keep pouring the bytes into the main decoded buffer area
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _OpusSampleLE, dwDecoded);
                dwCumulDecoded += dwDecoded;
            }
        }
        else if (iSamples == 0) // End of stream
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        else // iSamples < 0 --> Something's not right. Bail.
        {
            return DEWDEC_DECODE_ERROR;
        }
    } while (TRUE);

    return DEWDEC_DECODE_ERROR;
}

// Calculate the sample position based on the required seek point in time
BYTE OPUSENGINE::_Seek(DWORD dwSeconds)
{
    ogg_uint64_t    u64SeekSample = 0;
    double          dDuration = 0, dSeekTime;

    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        if (!_pOOF) return DEWERR_MM_ACTION;
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwSeconds;

        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
        {
            u64SeekSample = _u64TotalSamples;
        }
        else
        {
            u64SeekSample = (DWORD)((dSeekTime * (double)_u64TotalSamples) / dDuration);
            u64SeekSample = u64SeekSample > _u64TotalSamples ? _u64TotalSamples : u64SeekSample;
        }
        if (op_pcm_seek(_pOOF, u64SeekSample) != 0) return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Nothing much. Just set the basic info
OPUSENGINE::OPUSENGINE()
{
    _btMediaType = DEWMT_OPUS;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"opusfile v 0.9 | libopus v 1.2.1");
    _bVBR = TRUE; // We'll always provide the dynamic bitrate
}

// There's no direct api to load a Unicode file.
// We'll do it the indirect way
BYTE OPUSENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    // Store the entire file into the internal storage
    if (!this->_StoreRawInputData(srcFile)) return DEWERR_FILE_READ;

    // Set the flag to indicate that we're going to make an overloaded call
    _bOverloadedLoadFunctionCall = TRUE;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);
    // Then call the actual function to load the media
    return this->Load(notificationWindow, _lpEncodedSrcData, _dwSrcDataSize);
}

// The 'Real' function
BYTE OPUSENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    int         iResult = 0;
    MMRESULT    mmr = 0;
    wchar_t     wsValue[EXINFO_VAL_SIZE];

    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcDataBytes) return DEWERR_INVALID_PARAM;
    if (dataSize <= 0x00) return DEWERR_INVALID_PARAM;

    // Set the notification window
    _hWndNotify = notificationWindow;

    // Proceed with lpEncodedSrcData population only if this function
    // has been called explicitly from outside (and not an overloaded call)
    if (!_bOverloadedLoadFunctionCall)
    {
        _dwSrcDataSize = dataSize;
        _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
        CopyMemory(_lpEncodedSrcData, srcDataBytes, _dwSrcDataSize);
    }
    _pOOF = op_open_memory(_lpEncodedSrcData, _dwSrcDataSize, &iResult);
    if (iResult != 0 || _pOOF == NULL) return DEWERR_ENGINE_STARTUP;

    _u64TotalSamples = op_pcm_total(_pOOF, -1); // Get the total samples in the stream
    _btChannels = op_channel_count(_pOOF, -1); // Get the channels for the entire stream
    _dwBitrate = op_bitrate(_pOOF, -1); // Get the average bitrate
    _dwDuration = (DWORD)((double)_u64TotalSamples / ((double)OPUS_OUT_FREQ/1000.0f));
    const OpusHead* oh = op_head(_pOOF, -1);
    _dwSampleRate = (DWORD)oh->input_sample_rate; // For metadata only. We'll stick to 48 KHz.

    _wfex.cbSize = 0;
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.wBitsPerSample = 16; // We'll go with 16 bits per sampls
    _wfex.nChannels = 2; // We'll always decode to stereo. So the channel count will always be 2
    _wfex.nSamplesPerSec = OPUS_OUT_FREQ; // Always decode to 48KHz
    _wfex.nBlockAlign = (_wfex.wBitsPerSample * _wfex.nChannels) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    // We have a good amoount of metadata for opus
    _dwExInfoLength = 8;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"Ogg/Opus Version");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u.%u", (((BYTE)oh->version) & 0x0F), (((BYTE)(oh->version) >> 4) & 0x0F));
    _lpExInfo[0].SetValue(wsValue);

    _lpExInfo[1].SetType(L"Link Count");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", op_link_count(_pOOF));
    _lpExInfo[1].SetValue(wsValue);

    _lpExInfo[2].SetType(L"Link Serial Number");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", op_serialno(_pOOF, -1));
    _lpExInfo[2].SetValue(wsValue);

    _lpExInfo[3].SetType(L"Link Serial Number");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", oh->stream_count);
    _lpExInfo[3].SetValue(wsValue);

    _lpExInfo[4].SetType(L"Coupled Stream Count");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", oh->coupled_count);
    _lpExInfo[4].SetValue(wsValue);

    _lpExInfo[5].SetType(L"Mapping");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", oh->mapping);
    _lpExInfo[5].SetValue(wsValue);

    _lpExInfo[6].SetType(L"Mapping Family");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", oh->mapping_family);
    _lpExInfo[6].SetValue(wsValue);

    _lpExInfo[7].SetType(L"Output Gain");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", oh->output_gain);
    _lpExInfo[7].SetValue(wsValue);

    // TagLib doesn't seem to play well with Base64 encoded image data
    // Particularly, it seems to be failing to get hold of the
    // METADATA_BLOCK_PICTURE tag. So we'll use the native opus_tag
    // to overcome this and keep the picture ready.
    // Taglib will simply skip the image data extraction
    const OpusTags *tags = op_tags(_pOOF, -1);
    for (int nooo = 0; nooo < tags->comments; nooo++)
    {
        if (!(opus_tagncompare("METADATA_BLOCK_PICTURE", 22, tags->user_comments[nooo])))
        {
            OpusPictureTag pic;
            opus_picture_tag_parse(&pic, tags->user_comments[nooo]);
            IStream *pImgStream = SHCreateMemStream(pic.data, pic.data_length);
            if (pImgStream)
            {
                this->_tiTagInfo.AlbumArt = Image::FromStream(pImgStream);
                pImgStream->Release();
                pImgStream = NULL;
            }
            opus_picture_tag_clear(&pic);
        }
    }

    // All set!
    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Backtrack from the current sample position to get the duration in seconds
DWORD OPUSENGINE::Tell()
{
    ogg_uint64_t    u64CurrSample = 0;

    if (!_pOOF) return 0;
    u64CurrSample = op_pcm_tell(_pOOF);
    return (DWORD)(((double)u64CurrSample * double(_dwDuration)) / ((double)_u64TotalSamples));
}

// Housekeeping & cleanup
OPUSENGINE::~OPUSENGINE()
{
    this->_Cleanup();
    return;
}

// Isolated static method quickly dabble a file to see if it is an OPUS file
BOOL OPUSENGINE::IsLoadable(LPCWSTR testFile)
{
    HANDLE      hTmpFile = NULL;
    BYTE        TmpData[512] { 0 };
    DWORD       dwTmpSize = 0;
    OggOpusFile *oof = NULL;
    int         iResult = 0;

    if (!testFile) return FALSE;
    hTmpFile = CreateFile(testFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTmpFile == NULL || hTmpFile == INVALID_HANDLE_VALUE) return FALSE;
    dwTmpSize = GetFileSize(hTmpFile, NULL);
    ReadFile(hTmpFile, TmpData, 512, &dwTmpSize, NULL);
    CloseHandle(hTmpFile);
    if (dwTmpSize < 512) return FALSE;

    oof = op_test_memory(TmpData, dwTmpSize, &iResult);
    if (iResult < 0 || oof == NULL) return FALSE;
    op_free(oof);
    oof = NULL;
    return TRUE;
}

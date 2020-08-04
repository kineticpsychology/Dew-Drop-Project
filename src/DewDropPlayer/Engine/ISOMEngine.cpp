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

#include "ISOMEngine.h"
#include "AACEngine.h"
#include "AC3Engine.h"
#include "ALACEngine.h"
#include "VORBISEngine.h"
#include "MP3Engine.h"

// Keep extracting the AAC data. However, each sample must precede with
// its decriptor (ADTS header).
BOOL ISOMENGINE::_GenerateAACData()
{
    AP4_Size                ap4Zero = 0;
    AP4_UI32                subFmt;
    BYTE                    adtsHeader[7];
    UINT                    freqIndex = 0, channelCount = 0, frameSize = 0;
    AP4_MemoryByteStream    *ap4Output;
    AP4_Sample              sample;
    AP4_DataBuffer          data;
    AP4_Ordinal             index = 0;
    AP4_UI32                nAACSampleRate;

    if (!_pSampleDescr) return FALSE;
    AP4_AudioSampleDescription* ap4AudioDescr = AP4_DYNAMIC_CAST(AP4_AudioSampleDescription, _pSampleDescr);
    if (!ap4AudioDescr) return FALSE;

    AP4_MpegSampleDescription *pMpegSampleDescr = AP4_DYNAMIC_CAST(AP4_MpegSampleDescription, _pSampleDescr);
    subFmt = pMpegSampleDescr->GetObjectTypeId();

    AP4_MpegAudioSampleDescription *pMpegAudDescr = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, _pSampleDescr);
    subFmt = pMpegAudDescr->GetMpeg4AudioObjectType();
    switch (subFmt)
    {
        case ISOM_SUBTYPE_AAC_LC: _uAACSubType = ISOM_SUBTYPE_AAC_LC - 1; break;
        case ISOM_SUBTYPE_AAC_MAIN: _uAACSubType = ISOM_SUBTYPE_AAC_MAIN - 1; break;
        case ISOM_SUBTYPE_AAC_SSR: _uAACSubType = ISOM_SUBTYPE_AAC_SSR - 1; break;
        case ISOM_SUBTYPE_AAC_LTP: _uAACSubType = ISOM_SUBTYPE_AAC_LTP - 1; break;
        case ISOM_SUBTYPE_HE_AAC: _uAACSubType = ISOM_SUBTYPE_AAC_LC - 1; break;
        case ISOM_SUBTYPE_HEv2_AAC: _uAACSubType = ISOM_SUBTYPE_AAC_LC - 1; break;
    }

    // All the tables can be found in the ADTS header reference
    // (see web link in the AACEngine.cpp)
    channelCount = ap4AudioDescr->GetChannelCount();
    nAACSampleRate = ap4AudioDescr->GetSampleRate();
    if (subFmt == ISOM_SUBTYPE_HE_AAC || subFmt == ISOM_SUBTYPE_HEv2_AAC)
        nAACSampleRate /= channelCount;
    switch (nAACSampleRate)
    {
        case 96000: freqIndex = 0; break;
        case 88200: freqIndex = 1; break;
        case 64000: freqIndex = 2; break;
        case 48000: freqIndex = 3; break;
        case 44100: freqIndex = 4; break;
        case 32000: freqIndex = 5; break;
        case 24000: freqIndex = 6; break;
        case 22050: freqIndex = 7; break;
        case 16000: freqIndex = 8; break;
        case 12000: freqIndex = 9; break;
        case 11025: freqIndex = 10; break;
        case 8000:  freqIndex = 11; break;
        case 7350:  freqIndex = 12; break;
        default:    freqIndex = 0;
    }
    
    // Create a memory stream and keep writing the ADTS & raw AAC sample data
    ap4Output = new AP4_MemoryByteStream();
    while (AP4_SUCCEEDED(_pTrack->ReadSample(index, sample, data)))
    {
        frameSize = sample.GetSize();
        adtsHeader[0] = 0xFF;
        adtsHeader[1] = 0xF1; // 0xF9 (MPEG2)
        adtsHeader[2] = (_uAACSubType << 0x06) | (freqIndex << 2) | (channelCount >> 2);
        adtsHeader[3] = ((channelCount & 0x3) << 6) | ((frameSize + 7) >> 11);
        adtsHeader[4] = ((frameSize + 7) >> 3) & 0xFF;
        adtsHeader[5] = (((frameSize + 7) << 5) & 0xFF) | 0x1F;
        adtsHeader[6] = 0xFC;
        
        ap4Output->Write(adtsHeader, 7);
        ap4Output->Write(data.GetData(), data.GetDataSize());
        index++;
    }
    AP4_LargeSize       ap4DecodedSize = 0;
    ap4Output->GetSize(ap4DecodedSize);
    _dwSrcDataSize = (DWORD)ap4DecodedSize;
    // Transfer the stream data into the 'lpEncodedSrcData' buffer
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
    CopyMemory(_lpEncodedSrcData, ap4Output->GetData(), _dwSrcDataSize);
    // and flush out the stream
    ap4Output->Flush();
    ap4Output->Release();
    return TRUE;
}

// Very similar and stripped down version of the AAC data generator.
// The difference is that no ADTS header is required for this.
// Applicable for .mp3/.ac3 tracks muxed into mp4
BOOL ISOMENGINE::_GenerateRawSampleData()
{
    AP4_MemoryByteStream    *ap4Output;
    AP4_Sample              sample;
    AP4_DataBuffer          data;
    AP4_Ordinal             index = 0;
    AP4_Result              hr;
    AP4_LargeSize           ap4DecodedSize = 0;

    if (!_pTrack) return FALSE;
    
    ap4Output = new AP4_MemoryByteStream();
    do
    {
        hr = _pTrack->ReadSample(index, sample, data);
        if (AP4_SUCCEEDED(hr))
            ap4Output->Write(data.GetData(), data.GetDataSize());
        index++;
        sample.Reset();
    } while (AP4_SUCCEEDED(hr));

    ap4Output->GetSize(ap4DecodedSize);
    _dwSrcDataSize = (DWORD)ap4DecodedSize;
    // Transfer the stream data into the 'lpEncodedSrcData' buffer
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
    CopyMemory(_lpEncodedSrcData, ap4Output->GetData(), _dwSrcDataSize);
    // and flush out the stream
    ap4Output->Flush();
    ap4Output->Release();
    return TRUE;
}

// Follow along the lines of the previous two functions
// However, since this is ALAC, we need to build up a sample lookup table
// which will be passed on to the ALACEngine class
BOOL ISOMENGINE::_GenerateALACData()
{
    AP4_MemoryByteStream    *ap4Output;
    AP4_Sample              sample;
    AP4_DataBuffer          data;
    AP4_Ordinal             index = 0;
    AP4_Result              hr;
    AP4_LargeSize           ap4DecodedSize = 0;
    DWORD                   dwSampleOffset = 0;
    AP4_AudioSampleDescription *pAudDescr = NULL;

    if (!_pTrack) return FALSE;

    ap4Output = new AP4_MemoryByteStream();
    do
    {
        hr = _pTrack->ReadSample(index, sample, data);
        if (AP4_SUCCEEDED(hr))
        {
            if (_dwSampleTableSize == 0)
                _SampleTable = (LPDWORD)LocalAlloc(LPTR, sizeof(DWORD));
            else
                _SampleTable = (LPDWORD)LocalReAlloc(_SampleTable, (_dwSampleTableSize + 1) * sizeof(DWORD), LHND);
            _SampleTable[_dwSampleTableSize] = dwSampleOffset;
            dwSampleOffset += data.GetDataSize();
            _dwSampleTableSize++;
            ap4Output->Write(data.GetData(), data.GetDataSize());
        }
        index++;
    } while (AP4_SUCCEEDED(hr));

    ap4Output->GetSize(ap4DecodedSize);
    _dwSrcDataSize = (DWORD)ap4DecodedSize;
    // Transfer the stream data into the 'lpEncodedSrcData' buffer
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
    CopyMemory(_lpEncodedSrcData, ap4Output->GetData(), _dwSrcDataSize);
    // and flush out the stream
    ap4Output->Flush();
    ap4Output->Release();

    // Collect these information as they too, will be needed by ALACEngine
    pAudDescr = AP4_DYNAMIC_CAST(AP4_AudioSampleDescription, _pSampleDescr);
    _btChannels = (BYTE)pAudDescr->GetChannelCount();
    _dwSampleRate = pAudDescr->GetSampleRate();
    _dwDuration = _pTrack->GetDurationMs();

    return TRUE;
}

// This is the lengthiest of all. Basically, we collect the sample descriptor
// that gives us the first 2 ogg page info. Once done, we package all the
// subsequent samples into ogg "pages". This could not be contained within
// a single function. So we have created a dedicated class for this.
// The 'ISOMToOggVorbis' class. In a nutshell, the class is an
// 'Ogg/Vorbis' extractor from the ISOM file.
// WARNING! This is NOT an established standard way. But the encoders (e.g. XMedia Recode)
//          and decoders (VLC) seem to agree on this.
BOOL ISOMENGINE::_GenerateVorbisData()
{
    AP4_Sample              currSample, prevSample;
    AP4_DataBuffer          currSampleData, prevSampleData;
    UINT                    nIndex, nTotalSamples;
    AP4_UI64                lastGranulePos;

    nTotalSamples = _pTrack->GetSampleCount();
    lastGranulePos = _pTrack->GetMediaDuration();

    // Note that we are sending the granule position with each iteration.
    // As per ogg specs, the grabule position AFTER the current block
    // should be represented. However, we don't have to worry. The packer
    // does exactly that. It picks up the current granule position and 
    // packs it in the PREVIOUS packet, when the segment table limit is hit.
    // Loop from 0 to nTotalSamples - 1, so that we can avoid an 'if' condition
    // while the loop is running. This will same some time.
    for (nIndex = 0; nIndex < nTotalSamples - 1; nIndex++)
    {
        _pTrack->ReadSample(nIndex, currSample, currSampleData);
        _pOggVorbisPacker->AddVorbisBlock((LPBYTE)(currSampleData.GetData()), currSampleData.GetDataSize(), currSample.GetDts());
        currSample.Reset();
    }
    // Now we simply read the last sample. This way, we get to
    _pTrack->ReadSample(nIndex, currSample, currSampleData);
    _pOggVorbisPacker->AddVorbisBlock((LPBYTE)(currSampleData.GetData()), currSampleData.GetDataSize(), lastGranulePos, TRUE);

    _pOggVorbisPacker->GetFullAudioTrack(&_lpEncodedSrcData, &_dwSrcDataSize);
    return (_dwSrcDataSize > 0);
}


// Done. Flush all the memory hogging classes
// and release the internal audio engine as well
void ISOMENGINE::_Cleanup()
{
    if (_pOggVorbisPacker)
    {
        delete _pOggVorbisPacker;
        _pOggVorbisPacker = NULL;
    }

    if (_pSampleDescr)
    {
        delete _pSampleDescr;
        _pSampleDescr = NULL;
    }
    if (_pInternalEngine)
    {
        _pInternalEngine->Unload();
        delete _pInternalEngine;
        _pInternalEngine = NULL;
    }
    return;
}

// Stub method strictly because C++ demands so from an 'instance-ready' class
BYTE ISOMENGINE::_Decode()
{
    // Nothing needs to be done;
    this->_btMediaType = DEWMT_UNKNOWN;
    return DEWDEC_DECODE_OK;
}

// Delegate 'Seek' to the internal engine (if available)
BYTE ISOMENGINE::_Seek(DWORD dwSeconds)
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Seek(dwSeconds);
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// We read the file. Collect the audio/video stream information,
// pick the first audio stream (if available). Once the audio stream
// is available, we analyze if it is one of AAC/MP3/AC3/ALAC. Other
// formats are not currently supported. If one of these 4 audio
// formats is discovered, create the respective engine and delegate all the
// engine-related code to the internal engine.
BYTE ISOMENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    AP4_ByteStream  *pISOMFileStream = NULL;
    AP4_Movie       *pISOMMovie = NULL;
    AP4_MpegSampleDescription *pVorbisSampleDescr = NULL;
    UINT            nTrackID = 0;
    BOOL            bAudioTrackFound = FALSE, bVorbisPacked = FALSE;
    BYTE            btEngineRet = DEWERR_ENGINE_STARTUP;
    wchar_t         wsValue[EXINFO_VAL_SIZE], wsCodecFormat[EXINFO_VAL_SIZE];
    DWORD           dwGenLen = 0, dwExInfoIndex = 0, dwOggDescrSize;
    AP4_String      codecString;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    // Set the notification window
    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    // Open the file with the static method
    // ** This method has been modified in the Bento4 library to accept wchar_t* (instead of char*)
    if (AP4_FileByteStream::Create(_wsSrcFile, AP4_FileByteStream::STREAM_MODE_READ, pISOMFileStream) != 0) return DEWERR_FILE_READ;
    _pISOMFile = new AP4_File(*pISOMFileStream, true);
    if (!_pISOMFile) return DEWERR_FILE_READ;

    // Get the movie pointer
    pISOMMovie = _pISOMFile->GetMovie();
    if (!pISOMMovie)
    {
        // Release all resources before returning
        pISOMFileStream->Flush();
        pISOMFileStream->Release();
        delete pISOMFileStream;
        pISOMFileStream = NULL;
        return DEWERR_FILE_READ;
    }

    AP4_List<AP4_Track>& isomTtracks = pISOMMovie->GetTracks();
    if (isomTtracks.ItemCount() <= 0)
    {
        // Release all resources before returning
        pISOMFileStream->Flush();
        pISOMFileStream->Release();
        delete pISOMFileStream;
        pISOMFileStream = NULL;
        return DEWERR_INVALID_FILE;
    }

    // Enumerate the tracks
    _dwTotalTracks = (DWORD)isomTtracks.ItemCount();
    for (nTrackID = 0; nTrackID < _dwTotalTracks; nTrackID++)
    {
        AP4_Track   *pCurrTrack = NULL;
        isomTtracks.Get(nTrackID, pCurrTrack);
        // Enumerate the audio tracks
        if (pCurrTrack->GetType() == AP4_Track::TYPE_AUDIO)
        {
            _pSampleDescr = pCurrTrack->GetSampleDescription(0);
            if (!_pSampleDescr)
            {
                // Release all resources before returning
                pISOMFileStream->Flush();
                pISOMFileStream->Release();
                delete pISOMFileStream;
                pISOMFileStream = NULL;
                return DEWERR_ENGINE_STARTUP;
            }

            // AC3
            if (_pSampleDescr->GetFormat() == AP4_SAMPLE_FORMAT_AC_3)
            {
                _dwAudioStreamID = nTrackID;
                bAudioTrackFound = TRUE;
                _pTrack = pCurrTrack;
                // Get the source data dump
                if (!this->_GenerateRawSampleData())
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return DEWERR_ENGINE_STARTUP;
                }
                _pInternalEngine = new AC3ENGINE();
                // And delegate the encoded source data to the AC3Engine
                btEngineRet = _pInternalEngine->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize);
                if (_lpEncodedSrcData)
                {
                    LocalFree(_lpEncodedSrcData);
                    _lpEncodedSrcData = NULL;
                }
                if (btEngineRet != DEWERR_SUCCESS)
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return btEngineRet;
                }
                StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AC3 (A/52) Stream");
                break;
            }
            // ALAC
            else if (_pSampleDescr->GetFormat() == AP4_SAMPLE_FORMAT_ALAC)
            {
                _dwAudioStreamID = nTrackID;
                bAudioTrackFound = TRUE;
                _pTrack = pCurrTrack;
                // Get the source data dump, sample lookup table and other
                // details that is needed by the ALACEngine
                if (!this->_GenerateALACData())
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return DEWERR_ENGINE_STARTUP;
                }
                _pInternalEngine = new ALACENGINE();
                btEngineRet = ((LPALACENGINE)_pInternalEngine)->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize, _SampleTable, _dwSampleTableSize, _btChannels, _dwSampleRate, _dwDuration);
                if (_lpEncodedSrcData)
                {
                    LocalFree(_lpEncodedSrcData);
                    _lpEncodedSrcData = NULL;
                }
                if (btEngineRet != DEWERR_SUCCESS)
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return btEngineRet;
                }
                StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"ALAC (Apple Lossless) Stream");
                break;
            }
            // MP3 (Note: This constant is custom defined in this class
            // It is not natively defined in Bento4
            else if (_pSampleDescr->GetFormat() == AP4_SAMPLE_FORMAT_MP3)
            {
                _dwAudioStreamID = nTrackID;
                bAudioTrackFound = TRUE;
                _pTrack = pCurrTrack;
                // Get the source data dump
                if (!this->_GenerateRawSampleData())
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return DEWERR_ENGINE_STARTUP;
                }
                _pInternalEngine = new MP3ENGINE();
                // And delegate the encoded source data to the MP3Engine
                btEngineRet = _pInternalEngine->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize);
                if (_lpEncodedSrcData)
                {
                    LocalFree(_lpEncodedSrcData);
                    _lpEncodedSrcData = NULL;
                }
                if (_SampleTable)
                {
                    LocalFree(_SampleTable);
                    _SampleTable = NULL;
                }
                if (btEngineRet != DEWERR_SUCCESS)
                {
                    // Release all resources before returning
                    pISOMFileStream->Flush();
                    pISOMFileStream->Release();
                    delete pISOMFileStream;
                    pISOMFileStream = NULL;
                    return btEngineRet;
                }
                StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"MPEG-1/2 (MP3) Stream");
                break;
            }
            // AAC/MP3/Vorbis
            else if (_pSampleDescr->GetFormat() == AP4_SAMPLE_FORMAT_MP4A)
            {
                _dwAudioStreamID = nTrackID;
                bAudioTrackFound = TRUE;
                _pTrack = pCurrTrack;

                AP4_String CodecStringIdentifier;
                _pSampleDescr->GetCodecString(CodecStringIdentifier);
                
                // AAC (LC/Main/LTP/SSR/HE/HEv2
                if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_MAIN) ||
                    !lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_LC) ||
                    !lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_SSR) ||
                    !lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_LTP) ||
                    !lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_FAAC_HE) ||
                    !lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_FAAC_HEV2) )
                {
                    // Get to work, preparing the entire AAC raw stream data
                    if (!this->_GenerateAACData())
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return DEWERR_ENGINE_STARTUP;
                    }
                    _pInternalEngine = new AACENGINE();
                    // And delegate the encoded source data to the AACEngine
                    btEngineRet = _pInternalEngine->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize);
                    if (_lpEncodedSrcData)
                    {
                        LocalFree(_lpEncodedSrcData);
                        _lpEncodedSrcData = NULL;
                    }
                    if (btEngineRet != DEWERR_SUCCESS)
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return btEngineRet;
                    }
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_MAIN))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (Main Profile)");
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_LC))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (LC Profile)");
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_SSR))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (SSR Profile)");
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_AAC_LTP))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (LTP Profile)");
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_FAAC_HE))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (HE AAC)");
                    if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_FAAC_HEV2))
                        StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"AAC Stream (HEv2 AAC)");

                }
                // MP2/MP3
                else if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_MPEG))
                {
                    if (!this->_GenerateRawSampleData())
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return DEWERR_ENGINE_STARTUP;
                    }

                    _pInternalEngine = new MP3ENGINE();
                    // And delegate the encoded source data to the AACEngine
                    btEngineRet = _pInternalEngine->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize);
                    if (_lpEncodedSrcData)
                    {
                        LocalFree(_lpEncodedSrcData);
                        _lpEncodedSrcData = NULL;
                    }
                    if (btEngineRet != DEWERR_SUCCESS)
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return btEngineRet;
                    }
                    StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"MPEG Layer 2/3");
                }
                // Vorbis (experimental)
                else if (!lstrcmpA(CodecStringIdentifier.GetChars(), ISOM_CODEC_STRING_VORBIS))
                {
                    // By now, it is guaranteed that this is a MPEG desciptor. So we will cast
                    // it and extract the descriptor info out of the sample descriptor.
                    // This will give us the 0x1E-sized 1st header info, the comment and the codebook block.
                    // We will use that it to initialize the Vorbis packer and then keep on adding the extracted samples
                    pVorbisSampleDescr = ((AP4_MpegSampleDescription*)_pSampleDescr);
                    dwOggDescrSize = pVorbisSampleDescr->GetDecoderInfo().GetDataSize();
                    _pOggVorbisPacker = new ISOMTOOGGVORBIS((const LPBYTE)(pVorbisSampleDescr->GetDecoderInfo().GetData()), dwOggDescrSize);
                    // Store the result of vorbis data generation in a variable since we will
                    // kill the '_pOggVorbisPacker' class, no matter what
                    bVorbisPacked = this->_GenerateVorbisData();
                    delete _pOggVorbisPacker; // Packing complete. The packer class is no longer needed
                    _pOggVorbisPacker = NULL;
                    if (!bVorbisPacked)
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return DEWERR_ENGINE_STARTUP;
                    }
                    _pInternalEngine = new VORBISENGINE();
                    // And delegate the encoded source data to the VORBISEngine
                    btEngineRet = _pInternalEngine->Load(_hWndNotify, _lpEncodedSrcData, _dwSrcDataSize);
                    if (_lpEncodedSrcData)
                    {
                        LocalFree(_lpEncodedSrcData);
                        _lpEncodedSrcData = NULL;
                    }
                    if (btEngineRet != DEWERR_SUCCESS)
                    {
                        // Release all resources before returning
                        pISOMFileStream->Flush();
                        pISOMFileStream->Release();
                        delete pISOMFileStream;
                        pISOMFileStream = NULL;
                        return btEngineRet;
                    }
                    
                    StringCchPrintf(wsCodecFormat, EXINFO_VAL_SIZE, L"Vorbis");
                }
                break;
            }
        }
    }

    // If engine was loaded with the audio stream, then we're game
    // Add the internal engine's metadata with this class's metadata (5)
    if (btEngineRet == DEWERR_SUCCESS && _pInternalEngine)
    {
        this->_dwBitrate = _pInternalEngine->Bitrate;
        this->_dwDuration = _pInternalEngine->Duration;
        this->_dwSampleRate = _pInternalEngine->SampleRate;
        this->_btChannels = _pInternalEngine->Channels;
        this->_btMediaType = (this->_btMediaType | _pInternalEngine->MediaType);

        _dwExInfoLength = 5 + _pInternalEngine->ExtendedInfoCount;
        _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

        _lpExInfo[0].SetType(L"Total Tracks");
        StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _dwTotalTracks);
        _lpExInfo[0].SetValue(wsValue);

        _lpExInfo[1].SetType(L"Active Track ID");
        StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _dwAudioStreamID);
        _lpExInfo[1].SetValue(wsValue);

        _lpExInfo[2].SetType(L"Active Track Name");
        dwGenLen = lstrlenA(_pTrack->GetTrackName());
        dwGenLen = dwGenLen > (EXINFO_VAL_SIZE - 2) ? EXINFO_VAL_SIZE - 2 : dwGenLen;
        MultiByteToWideChar(CP_UTF8, 0, _pTrack->GetTrackName(), dwGenLen, wsValue, dwGenLen);
        wsValue[dwGenLen] = L'\0';
        _lpExInfo[2].SetValue(wsValue);

        _lpExInfo[3].SetType(L"Audio Codec");
        _lpExInfo[3].SetValue(wsCodecFormat);

        _lpExInfo[4].SetType(L"Codec String");
        _pSampleDescr->GetCodecString(codecString);
        dwGenLen = codecString.GetLength() <= (EXINFO_VAL_SIZE - 2) ? codecString.GetLength() : (EXINFO_VAL_SIZE - 2);
        MultiByteToWideChar(CP_UTF8, 0, codecString.GetChars(), dwGenLen, wsValue, dwGenLen);
        wsValue[dwGenLen] = L'\0';
        _lpExInfo[4].SetValue(wsValue);

        for (dwExInfoIndex = 0; dwExInfoIndex < _pInternalEngine->ExtendedInfoCount; dwExInfoIndex++)
        {
            _lpExInfo[5 + dwExInfoIndex].SetType(_pInternalEngine->ExtendedInfo[dwExInfoIndex].Type);
            _lpExInfo[5 + dwExInfoIndex].SetValue(_pInternalEngine->ExtendedInfo[dwExInfoIndex].Value);
        }
    }

    // Release the resources
    pISOMFileStream->Flush();
    pISOMFileStream->Release();
    delete pISOMFileStream;
    pISOMFileStream = NULL;
    return btEngineRet;

}

// We won't support memory mode of file loading
BYTE ISOMENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// This engine is just a mask. The actual decoding will be done
// by the underlysing Audio engine. So we expose each of the public methods
// overriding them with the action of the underlying decoding engine
ISOMENGINE::ISOMENGINE()
{
    StringCchPrintf(_wsLibrary, MAX_PATH, L"Bento4 version 1-5-1-628 (Customized with unicode support)");
    return;
}

// Delegate 'Play' to the internal engine (if available)
BYTE ISOMENGINE::Play()
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Play();
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// Delegate 'Pause' to the internal engine (if available)
BYTE ISOMENGINE::Pause()
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Pause();
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// Delegate 'Resume' to the internal engine (if available)
BYTE ISOMENGINE::Resume()
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Resume();
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// Delegate 'Stop' to the internal engine (if available)
BYTE ISOMENGINE::Stop()
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Stop();
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// Delegate 'Tell' to the internal engine (if available)
DWORD ISOMENGINE::Tell()
{
    if (!_pInternalEngine) return (DWORD)INVALID_HANDLE_VALUE;
    _btStatus = _pInternalEngine->Status;
    return _pInternalEngine->Tell();
}

// Delegate 'SetVolume' to the internal engine (if available)
BYTE ISOMENGINE::SetVolume(WORD wLevel)
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->SetVolume(wLevel);
    _btStatus = _pInternalEngine->Status;
    return btEngineRet;
}

// Delegate 'Unload' to the internal engine (if available)
BYTE ISOMENGINE::Unload()
{
    BYTE    btEngineRet;
    if (!_pInternalEngine) return DEWERR_MM_ACTION;
    btEngineRet = _pInternalEngine->Unload();
    if (btEngineRet == DEWERR_SUCCESS)
        _btStatus = DEWS_MEDIA_NONE;
    return btEngineRet;
}

// Housekeeping and cleanup
ISOMENGINE::~ISOMENGINE()
{
    this->_Cleanup();
    return;
}

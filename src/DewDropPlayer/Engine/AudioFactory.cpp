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

#include "AudioFactory.h"

#include "AACEngine.h"
#include "AC3Engine.h"
#include "AIFFEngine.h"
#include "ALACEngine.h"
#include "APEEngine.h"
#include "CDDAEngine.h"
#include "FLACEngine.h"
#include "MP3Engine.h"
#include "ISOMEngine.h"
#include "MIDIEngine.h"
#include "MPCEngine.h"
#include "OPUSEngine.h"
#include "VORBISEngine.h"
#include "WAVEngine.h"
#include "WMAEngine.h"
#include "WVEngine.h"


#pragma region ** THE MAIN INTERNAL FUNCTION **

BYTE AUDIOFACTORY::_ParseMediaType(LPCWSTR audioFile)
{
    HANDLE      hAudioFile = NULL;
    DWORD       dwBlock1 = 0, dwBlock2 = 0, dwBlock3 = 0, dwBlock4 = 0;
    DWORD       dwOggSubID = 0, dwGen = 0;

    if (!audioFile) return DEWMT_UNKNOWN;

    hAudioFile = CreateFile(audioFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hAudioFile == NULL || hAudioFile == INVALID_HANDLE_VALUE) return DEWMT_UNKNOWN;
    // Read 0x10 (16) bytes in chunks of 4 DWORDs
    ReadFile(hAudioFile, &dwBlock1, 4, &dwGen, NULL);
    ReadFile(hAudioFile, &dwBlock2, 4, &dwGen, NULL);
    ReadFile(hAudioFile, &dwBlock3, 4, &dwGen, NULL);
    ReadFile(hAudioFile, &dwBlock4, 4, &dwGen, NULL);
    // We need additional blocks to check for, for Oggs. So pick those bytes up
    // before closing out the file
    if (dwBlock1 == sigOgg)
    {
        SetFilePointer(hAudioFile, OGG_SUBPACKET_START, NULL, FILE_BEGIN);
        ReadFile(hAudioFile, &dwOggSubID, 4, &dwGen, NULL);
    }

    CloseHandle(hAudioFile);

    // Enumerate the various conditions based on the extensions
    // Container formats:
    if (dwBlock1 == sigOgg)
    {
        if ((dwOggSubID & 0xFFFFFF00) == sigOggVorbis)
            return DEWMT_OGG_VORBIS;
        if ((dwOggSubID & 0xFFFFFF00) == sigOggFlac)
            return DEWMT_OGG_FLAC;
        if (dwOggSubID == sigOggOpus)
        {
            BYTE    btMediaFromExt = DEWMT_UNKNOWN;
            btMediaFromExt = AUDIOFACTORY::GetMediaTypeFromExtension(audioFile);
            if (btMediaFromExt == DEWMT_OGG) return DEWMT_OGG_OPUS;
            return DEWMT_OPUS;
        }
        return DEWMT_UNKNOWN;
    }

    if (dwBlock2 == sigFTYP)
    {
        if ( (dwBlock3 & 0x00FFFFFF) == sigMP4 ||
             (dwBlock3 & 0x00FFFFFF) == sigM4A ||
             (dwBlock3 & 0x00FFFFFF) == sigM4V ||
             dwBlock3 == sigISOM )
            return DEWMT_ISOM;
        if ((dwBlock3 & 0x00FFFFFF) == sig3GP)
            return DEWMT_3GP;
    }

    if ( (dwBlock1 & 0x0000FFFF) == sigAAC1 ||
         (dwBlock1 & 0x0000FFFF) == sigAAC2)
        return DEWMT_AAC;

    if ((dwBlock1 & 0x0000FFFF) == sigAC3)
        return DEWMT_AC3;

    if (dwBlock1 == sigAIFF)
        return DEWMT_AIFF;

    if ( (dwBlock1 & 0x00FFFFFF) == sigAPE)
        return DEWMT_APE;

    if (dwBlock2 == sigRIFF)
    {
        if (dwBlock3 == sigCDDA) return DEWMT_CDDA;
        if (dwBlock3 == sigWAV) return DEWMT_WAV;
    }

    if (dwBlock1 == sigFLAC) return DEWMT_FLAC;

    if (dwBlock1 == sigMIDI) return DEWMT_MIDI;

    if (dwBlock1 == sigMPC) return DEWMT_MPC;

    if ( (dwBlock1 & 0x0000FFFF) == sigMP31 ||
         (dwBlock1 & 0x00FFFFFF) == sigMP32 ||
         (dwBlock1 & 0x0000FFFF) == sigMP2)
        return DEWMT_MP3;

    if (dwBlock1 == sigWV) return DEWMT_WV;
    
    // Still here. In that case, go blindly with the file extension
    return AUDIOFACTORY::GetMediaTypeFromExtension(audioFile);
}

#pragma endregion


#pragma region Utility Functions

void AUDIOFACTORY::_DeriveFileFromPath(LPCWSTR wsPath, LPWSTR wsFileName, const DWORD dwFleNameLen)
{
    size_t      nIndex;

    ZeroMemory(wsFileName, dwFleNameLen * sizeof(wchar_t));
    for (nIndex = lstrlen(wsPath) - 1; nIndex >= 0; nIndex--)
    {
        if (wsPath[nIndex] == L'\\')
        {
            nIndex++;
            break;
        }
    }
    if (nIndex == 0) return;
    CopyMemory(wsFileName, &(wsPath[nIndex]), (lstrlen(wsPath) - nIndex) * sizeof(wchar_t));
    return;
}

#pragma endregion


#pragma region Album Art extractor functions

void AUDIOFACTORY::_GetAPEArt(const TagLib::APE::Tag *apeTag, Image** plpCoverArt)
{
    IStream         *pStream = NULL;
    DWORD           dwImgDataOffset = 0;
    LARGE_INTEGER   li_Seek;

    if (apeTag)
    {
        {
            if (apeTag->itemListMap().size() > 0)
            {
                TagLib::APE::ItemListMap apeTagMap = apeTag->itemListMap();
                for (TagLib::APE::ItemListMap::Iterator iter = apeTagMap.begin(); iter != apeTagMap.end(); iter++)
                {
                    if (!StrCmpI(iter->first.toCWString(), L"COVER ART (FRONT)") ||
                        !StrCmpI(iter->first.toCWString(), L"COVER ART (BACK)"))
                    {
                        /*
                        The Data typically precedes with the below combinations:
                        Cover Art (Back).jpg <- both jpg and jpeg files
                        Cover Art (Front).jpg <- both jpg and jpeg files
                        Cover Art (Back).png
                        Cover Art (Front).png
                        All of these follow a 0x00 code right after. So our calculation is:
                        the key string length + '.jpg' + 1 bte (for the 0x00 marker)
                        => the key string length + 4 + 1 (since both .jpg and .png are the same length)
                        => the key string length + 5
                        */
                        dwImgDataOffset = lstrlen(iter->first.toCWString()) + 5;

                        pStream = SHCreateMemStream((const BYTE*)&(iter->second.binaryData().data()[dwImgDataOffset]), iter->second.binaryData().size() - dwImgDataOffset);
                        if (pStream)
                        {
                            li_Seek.QuadPart = 0;
                            pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                            *plpCoverArt = Image::FromStream(pStream);
                            pStream->Release();
                            pStream = NULL;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void AUDIOFACTORY::_GetASFArt(const TagLib::ASF::Tag *asfTag, Image **plpCoverArt)
{
    IStream         *pStream = NULL;
    DWORD           dwImgSize = 0;
    LARGE_INTEGER   li_Seek;

    if (!asfTag->isEmpty())
    {
        TagLib::ASF::AttributeListMap asfTagMap = asfTag->attributeListMap();
        if (!asfTagMap.isEmpty())
        {
            for (TagLib::ASF::AttributeListMap::Iterator iter = asfTagMap.begin(); iter != asfTagMap.end(); iter++)
            {
                if (!StrCmpI(iter->first.toCWString(), L"WM/PICTURE"))
                {
                    if (iter->second.size() > 0)
                    {
                        TagLib::ASF::AttributeList asfAttr = iter->second;
                        TagLib::ASF::AttributeList::Iterator asfAttrFirst = asfAttr.begin();
                        dwImgSize = asfAttrFirst->toPicture().dataSize();
                        if (dwImgSize > 0)
                            pStream = SHCreateMemStream((const BYTE*)asfAttrFirst->toPicture().picture().data(), dwImgSize);

                        if (dwImgSize > 0 && pStream)
                        {
                            li_Seek.QuadPart = 0;
                            pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                            *plpCoverArt = Image::FromStream(pStream);
                            pStream->Release();
                            pStream = NULL;
                            return;
                        }
                    }
                }
            }
        }
    }
    return;
}

void AUDIOFACTORY::_GetFLACArt(TagLib::FLAC::File& flacFile, Image **plpCoverArt)
{
    IStream         *pStream = NULL;
    DWORD           dwImgSize = 0;
    LARGE_INTEGER   li_Seek;

    if (flacFile.pictureList().size() > 0)
    {
        TagLib::List<TagLib::FLAC::Picture*> flacArtList = flacFile.pictureList();
        if (flacArtList.size() > 0)
        {
            dwImgSize = flacArtList[0]->data().size();
            if (dwImgSize > 0)
                pStream = SHCreateMemStream((const BYTE*)flacArtList[0]->data().data(), dwImgSize);
            if (dwImgSize > 0 && pStream)
            {
                li_Seek.QuadPart = 0;
                pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                *plpCoverArt = Image::FromStream(pStream);
                pStream->Release();
                pStream = NULL;
                return;
            }
            return;
        }
    }
}

void AUDIOFACTORY::_GetID3V2Art(const TagLib::ID3v2::Tag *mpgid3v2Tag, Image **plpCoverArt)
{
    IStream         *pStream = NULL;
    DWORD           dwImgSize = 0;
    LARGE_INTEGER   li_Seek;

    if (mpgid3v2Tag)
    {
        TagLib::ID3v2::FrameList frameList = mpgid3v2Tag->frameList("APIC");
        if (frameList.isEmpty() == false)
        {
            dwImgSize = ((TagLib::ID3v2::AttachedPictureFrame *)frameList.front())->picture().size();
            if (dwImgSize > 0)
            {
                pStream = SHCreateMemStream((const BYTE*)((TagLib::ID3v2::AttachedPictureFrame *)frameList.front())->picture().data(), dwImgSize);
            }
            else
            {
                dwImgSize = ((TagLib::ID3v2::AttachedPictureFrame *)frameList.back())->picture().size();
                if (dwImgSize > 0)
                {
                    pStream = SHCreateMemStream((const BYTE*)((TagLib::ID3v2::AttachedPictureFrame *)frameList.back())->picture().data(), dwImgSize);
                }
            }
            if (dwImgSize > 0 && pStream)
            {
                li_Seek.QuadPart = 0;
                pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                *plpCoverArt = Image::FromStream(pStream);
                pStream->Release();
                pStream = NULL;
                return;
            }
        }
    }
    return;
}

void AUDIOFACTORY::_GetXiphCommentArt(TagLib::Ogg::XiphComment& xiphTag, Image **plpCoverArt)
{
    IStream         *pStream = NULL;
    DWORD           dwImgSize = 0;
    LARGE_INTEGER   li_Seek;

    if (xiphTag.pictureList().size() > 0)
    {
        TagLib::List<TagLib::FLAC::Picture*> flacArtList = xiphTag.pictureList();
        if (flacArtList.size() > 0)
        {
            dwImgSize = flacArtList[0]->data().size();
            if (dwImgSize > 0)
                pStream = SHCreateMemStream((const BYTE*)flacArtList[0]->data().data(), dwImgSize);
            if (dwImgSize > 0 && pStream)
            {
                li_Seek.QuadPart = 0;
                pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                *plpCoverArt = Image::FromStream(pStream);
                pStream->Release();
                pStream = NULL;
                return;
            }
            return;
        }
    }
}

#pragma endregion


#pragma region Format specific tag information functions

void AUDIOFACTORY::_GetAIFFTags(LPCWSTR wsAIFFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::RIFF::AIFF::File aiffFile(wsAIFFile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", aiffFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", aiffFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", aiffFile.tag()->album().toCWString());
    if (aiffFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", aiffFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", aiffFile.tag()->genre().toCWString());
    if (aiffFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", aiffFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    if (aiffFile.hasID3v2Tag())
    {
        TagLib::ID3v2::Tag *id3v2Tag = aiffFile.tag();;
        AUDIOFACTORY::_GetID3V2Art(id3v2Tag, plpCoverArt);
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsAIFFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetAPETags(LPCWSTR wsAPEFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                               LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::MPC::File apeFile(wsAPEFile);
    if (apeFile.hasAPETag())
    {
        StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", apeFile.tag()->title().toCWString());
        StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", apeFile.tag()->artist().toCWString());
        StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", apeFile.tag()->album().toCWString());
        if (apeFile.tag()->track() > 0)
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", apeFile.tag()->track());
        else
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
        StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", apeFile.tag()->genre().toCWString());
        if (apeFile.tag()->year() > 0)
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", apeFile.tag()->year());
        else
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");
    }

    if (apeFile.hasAPETag())
    {
        TagLib::APE::Tag *apeTag = apeFile.APETag();
        AUDIOFACTORY::_GetAPEArt(apeTag, plpCoverArt);
    }
    
    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsAPEFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetASFTags(LPCWSTR wsASFFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                               LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::ASF::File asfFile(wsASFFile);
    
    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", asfFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", asfFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", asfFile.tag()->album().toCWString());
    if (asfFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", asfFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", asfFile.tag()->genre().toCWString());
    if (asfFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", asfFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    TagLib::ASF::Tag *asfTag = asfFile.tag();
    AUDIOFACTORY::_GetASFArt(asfTag, plpCoverArt);

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsASFFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }
    
    return;
}

void AUDIOFACTORY::_GetFLACTags(LPCWSTR wsFLACFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::FLAC::File flacFile(wsFLACFile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", flacFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", flacFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", flacFile.tag()->album().toCWString());
    if (flacFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", flacFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", flacFile.tag()->genre().toCWString());
    if (flacFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", flacFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    AUDIOFACTORY::_GetFLACArt(flacFile, plpCoverArt);

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsFLACFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetISOMTags(LPCWSTR wsISOMFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
    LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    IStream         *pStream = NULL;
    LARGE_INTEGER   li_Seek{ 0 };

    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::MP4::File isomFile(wsISOMFile);
    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", isomFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", isomFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", isomFile.tag()->album().toCWString());
    if (isomFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", isomFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", isomFile.tag()->genre().toCWString());
    if (isomFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", isomFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    if (isomFile.hasMP4Tag())
    {
        TagLib::MP4::Tag *isomTag = isomFile.tag();
        TagLib::MP4::ItemListMap isomTagMap = isomTag->itemListMap();
        if (isomTagMap.size() > 0)
        {
            TagLib::MP4::Item isomCoverItem = isomTagMap["covr"];
            TagLib::MP4::CoverArtList isomCoverList = isomCoverItem.toCoverArtList();
            if (!isomCoverList.isEmpty())
            {
                // 'Front' cover will take precedence over 'Back' cover
                if (isomCoverList.front().data().size() > 0)
                {
                    pStream = SHCreateMemStream((BYTE*)isomCoverList.front().data().data(), isomCoverList.front().data().size());
                }
                else if (isomCoverList.back().data().size() > 0)
                {
                    pStream = SHCreateMemStream((BYTE*)isomCoverList.back().data().data(), isomCoverList.back().data().size());
                }
                if (pStream != NULL)
                {
                    li_Seek.QuadPart = 0;
                    pStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
                    *plpCoverArt = Image::FromStream(pStream);
                    pStream->Release();
                    pStream = NULL;
                }
            }
        }
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsISOMFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }
    return;
}

void AUDIOFACTORY::_GetOggOpusTags(LPCWSTR wsOggOpusFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                   LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::Ogg::Opus::File oggOpusFile(wsOggOpusFile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", oggOpusFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", oggOpusFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", oggOpusFile.tag()->album().toCWString());
    if (oggOpusFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", oggOpusFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", oggOpusFile.tag()->genre().toCWString());
    if (oggOpusFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", oggOpusFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    TagLib::Ogg::XiphComment *xiphTag = oggOpusFile.tag();
    if (xiphTag->fieldCount() > 0)
    {
        TagLib::Ogg::FieldListMap xiphTagMap = xiphTag->fieldListMap();
        if (!xiphTagMap.isEmpty())
        {
            AUDIOFACTORY::_GetXiphCommentArt(*xiphTag, plpCoverArt);
        }
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsOggOpusFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetOggVorbisTags(LPCWSTR wsVorbisFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                     LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::Ogg::Vorbis::File vorbisFile(wsVorbisFile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", vorbisFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", vorbisFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", vorbisFile.tag()->album().toCWString());
    if (vorbisFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", vorbisFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", vorbisFile.tag()->genre().toCWString());
    if (vorbisFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", vorbisFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    TagLib::Ogg::XiphComment *xiphTag = vorbisFile.tag();
    if (xiphTag->fieldCount() > 0)
    {
        TagLib::Ogg::FieldListMap xiphTagMap = xiphTag->fieldListMap();
        if (!xiphTagMap.isEmpty())
        {
            AUDIOFACTORY::_GetXiphCommentArt(*xiphTag, plpCoverArt);
        }
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsVorbisFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetOpusTags(LPCWSTR wsOpusFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    IStream         *pStream = NULL;
    LARGE_INTEGER   li_Seek { 0 };
    LPBYTE          lpImgData = NULL, lpDecData = NULL;

    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::Ogg::Opus::File opusFile(wsOpusFile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", opusFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", opusFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", opusFile.tag()->album().toCWString());
    if (opusFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", opusFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", opusFile.tag()->genre().toCWString());
    if (opusFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", opusFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    // Nothing needs to be done for extracting image.
    // That will be taken care of, in the dedicated engine itself

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsOpusFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetMPCTags(LPCWSTR wsMPCFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                               LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;


    TagLib::MPC::File mpcFile(wsMPCFile);
    if (mpcFile.hasAPETag() || mpcFile.hasID3v1Tag())
    {
        StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", mpcFile.tag()->title().toCWString());
        StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", mpcFile.tag()->artist().toCWString());
        StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", mpcFile.tag()->album().toCWString());
        if (mpcFile.tag()->track() > 0)
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", mpcFile.tag()->track());
        else
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
        StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", mpcFile.tag()->genre().toCWString());
        if (mpcFile.tag()->year() > 0)
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", mpcFile.tag()->year());
        else
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");
    }
    
    if (mpcFile.hasAPETag())
    {
        TagLib::APE::Tag *apeTag = mpcFile.APETag();
        AUDIOFACTORY::_GetAPEArt(apeTag, plpCoverArt);
    }
    
    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsMPCFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetMP3Tags(LPCWSTR wsMP3File, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                               LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::MPEG::File mpgFile(wsMP3File);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", mpgFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", mpgFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", mpgFile.tag()->album().toCWString());
    if (mpgFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", mpgFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", mpgFile.tag()->genre().toCWString());
    if (mpgFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", mpgFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    if (mpgFile.hasID3v2Tag())
    {
        TagLib::ID3v2:: Tag *mpgid3v2Tag = mpgFile.ID3v2Tag();
        AUDIOFACTORY::_GetID3V2Art(mpgid3v2Tag, plpCoverArt);
    }
    else if (mpgFile.hasAPETag()) // Has APE tag. Call the function to extract the album art the 'APE' way!
    {
        TagLib::APE::Tag *apeTag = mpgFile.APETag();
        AUDIOFACTORY::_GetAPEArt(apeTag, plpCoverArt);
    }
    else if (mpgFile.hasID3v1Tag()) // ID3v1: No support for album art
    {
        TagLib::ID3v1::Tag *mpgid3v1Tag = mpgFile.ID3v1Tag();
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsMP3File, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetWAVTags(LPCWSTR wsWAVile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                               LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::RIFF::WAV::File wavFile(wsWAVile);

    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", wavFile.tag()->title().toCWString());
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", wavFile.tag()->artist().toCWString());
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", wavFile.tag()->album().toCWString());
    if (wavFile.tag()->track() > 0)
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", wavFile.tag()->track());
    else
        StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", wavFile.tag()->genre().toCWString());
    if (wavFile.tag()->year() > 0)
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", wavFile.tag()->year());
    else
        StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");

    if (wavFile.hasID3v2Tag())
    {
        TagLib::ID3v2::Tag *id3v2Tag = wavFile.ID3v2Tag();
        AUDIOFACTORY::_GetID3V2Art(id3v2Tag, plpCoverArt);
    }

    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsWAVile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

void AUDIOFACTORY::_GetWVTags(LPCWSTR wsWVFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                              LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{

    ZeroMemory(wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;

    TagLib::WavPack::File wvFile(wsWVFile);
    if (wvFile.hasAPETag() || wvFile.hasID3v1Tag())
    {
        StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", wvFile.tag()->title().toCWString());
        StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", wvFile.tag()->artist().toCWString());
        StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", wvFile.tag()->album().toCWString());
        if (wvFile.tag()->track() > 0)
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%u", wvFile.tag()->track());
        else
            StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
        StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", wvFile.tag()->genre().toCWString());
        if (wvFile.tag()->year() > 0)
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%u", wvFile.tag()->year());
        else
            StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");
    }
    
    if (wvFile.hasAPETag())
    {
        TagLib::APE::Tag *apeTag = wvFile.APETag();
        AUDIOFACTORY::_GetAPEArt(apeTag, plpCoverArt);
    }
    if (lstrlen(wsTitle) <= 0)
    {
        AUDIOFACTORY::_DeriveFileFromPath(wsWVFile, wsTitle, TAG_DATA_LENGTH);
        *pbSimulatedTitle = TRUE;
    }
    else
    {
        *pbSimulatedTitle = FALSE;
    }

    return;
}

// All other formats
void AUDIOFACTORY::_GetDefaultTags(LPCWSTR wsFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                   LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle)
{
    ZeroMemory(wsArtist, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsAlbum, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsTrack, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsGenre, TAG_DATA_LENGTH * sizeof(wchar_t));
    ZeroMemory(wsYear, TAG_DATA_LENGTH * sizeof(wchar_t));
    *plpCoverArt = NULL;
    AUDIOFACTORY::_DeriveFileFromPath(wsFile, wsTitle, TAG_DATA_LENGTH);
    *pbSimulatedTitle = TRUE;
    return;
}

#pragma endregion


#pragma region Exposed (Public Functions)

BYTE AUDIOFACTORY::GetMediaTypeFromExtension(LPCWSTR wsPath)
{
    int         iDotPos = 0, iSlashPos = 0, iIndex;
    wchar_t     wsExtension[MAX_PATH] { 0 };
    
    if (!wsPath) return DEWMT_UNKNOWN;
    if (lstrlen(wsPath) <= 3) return DEWMT_UNKNOWN;
    for (iIndex = lstrlen(wsPath) - 1; iIndex >= 0; iIndex--)
    {
        if (wsPath[iIndex] == L'\\')
        {
            iSlashPos = iIndex;
            break;
        }
        if (wsPath[iIndex] == L'.' && iSlashPos == 0)
        {
            iDotPos = iIndex + 1;
            break;
        }
    }

    if (iDotPos <= 0) return DEWMT_UNKNOWN;
    CopyMemory(wsExtension, &(wsPath[iDotPos]), (lstrlen(wsPath) - iDotPos)*sizeof(wchar_t));

    if (!StrCmpI(wsExtension, L"AAC")) return DEWMT_AAC;
    if (!StrCmpI(wsExtension, L"AC3")) return DEWMT_AC3;
    if (!StrCmpI(wsExtension, L"AIFF") || !StrCmpI(wsExtension, L"AIF")) return DEWMT_AIFF;
    if (!StrCmpI(wsExtension, L"APE")) return DEWMT_APE;
    if (!StrCmpI(wsExtension, L"CDA")) return DEWMT_CDDA;
    if (!StrCmpI(wsExtension, L"FLAC")) return DEWMT_FLAC;
    if (!StrCmpI(wsExtension, L"MID") || !StrCmpI(wsExtension, L"MIDI")) return DEWMT_MIDI;
    if (!StrCmpI(wsExtension, L"MPC")) return DEWMT_MPC;
    if (!StrCmpI(wsExtension, L"MP3")) return DEWMT_MP3;
    if (!StrCmpI(wsExtension, L"MP2")) return DEWMT_MP3;
    if (!StrCmpI(wsExtension, L"OPUS")) return DEWMT_OPUS;
    if (!StrCmpI(wsExtension, L"WAV")) return DEWMT_WAV;
    if (!StrCmpI(wsExtension, L"ASF") || !StrCmpI(wsExtension, L"WMA")) return DEWMT_WMA;
    if (!StrCmpI(wsExtension, L"WV")) return DEWMT_WV;
    if (!StrCmpI(wsExtension, L"OGG") || !StrCmpI(wsExtension, L"OGA")) return DEWMT_OGG_VORBIS;
    if (!StrCmpI(wsExtension, L"MP4") || !StrCmpI(wsExtension, L"M4A") ||
        !StrCmpI(wsExtension, L"M4V"))
        return DEWMT_ISOM;
    if (!StrCmpI(wsExtension, L"3GP")) return DEWMT_3GP;
    return DEWMT_UNKNOWN;
}

bool AUDIOFACTORY::GetAudioEngine(LPCWSTR audioFile, LPIAUDIOENGINE *plpEngine, BYTE mediaType)
{
    BYTE    btMediaType;
    wchar_t wsTitle[TAG_DATA_LENGTH], wsArtist[TAG_DATA_LENGTH],
            wsAlbum[TAG_DATA_LENGTH], wsTrack[TAG_DATA_LENGTH],
            wsGenre[TAG_DATA_LENGTH], wsYear[TAG_DATA_LENGTH];
    Image*  pAlbumArt = NULL;
    BOOL    bSimulatedTitle;
    
    *plpEngine = NULL;
    btMediaType = (mediaType == DEWMT_UNKNOWN ? (AUDIOFACTORY::_ParseMediaType(audioFile)) : mediaType);
    if (btMediaType == DEWMT_UNKNOWN) return false;

    switch (btMediaType)
    {
        case DEWMT_AAC:
        {    
             *plpEngine = new AACENGINE();
             AUDIOFACTORY::_GetDefaultTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_AC3:
        {
             *plpEngine = new AC3ENGINE();
             AUDIOFACTORY::_GetDefaultTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_AIFF:
        {
             *plpEngine = new AIFFENGINE();
             AUDIOFACTORY::_GetAIFFTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_APE:
        {
             *plpEngine = new APEENGINE();
             AUDIOFACTORY::_GetAPETags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_CDDA:
        {
             *plpEngine = new CDDAENGINE();
             AUDIOFACTORY::_DeriveFileFromPath(audioFile, wsTitle, TAG_DATA_LENGTH);
             (*plpEngine)->Tag.Populate(wsTitle, L"", L"", L"", L"", L"", NULL, TRUE);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_FLAC:
        {
             *plpEngine = new FLACENGINE();
             AUDIOFACTORY::_GetFLACTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_MIDI:
        {
             // No Tags for MIDI
             *plpEngine = new MIDIENGINE();
             AUDIOFACTORY::_DeriveFileFromPath(audioFile, wsTitle, TAG_DATA_LENGTH);
             (*plpEngine)->Tag.Populate(wsTitle, L"", L"", L"", L"", L"", NULL, TRUE);
             return true;
        }
        case DEWMT_MPC:
        {
             *plpEngine = new MPCENGINE();
             AUDIOFACTORY::_GetMPCTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_MP3:
        {
            *plpEngine = new MP3ENGINE();
            AUDIOFACTORY::_GetMP3Tags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
            (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
            if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
            return true;
        }
        case DEWMT_OPUS:
        {
            *plpEngine = new OPUSENGINE();
            AUDIOFACTORY::_GetOpusTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
            (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
            if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
            return true;
        }
        case DEWMT_WAV:
        {
             *plpEngine = new WAVENGINE();
             AUDIOFACTORY::_GetWAVTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_WMA:
        {
             *plpEngine = new WMAENGINE();
             AUDIOFACTORY::_GetASFTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_WV:
        {
             *plpEngine = new WVENGINE();
             AUDIOFACTORY::_GetWVTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             return true;
        }
        case DEWMT_OGG_VORBIS:
        {
             *plpEngine = new VORBISENGINE();
             AUDIOFACTORY::_GetOggVorbisTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             (*plpEngine)->SetContainerFormat(DEWMT_OGG);
             return true;
        }
        case DEWMT_OGG_FLAC:
        {
             *plpEngine = new FLACENGINE();
             AUDIOFACTORY::_GetFLACTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             (*plpEngine)->SetContainerFormat(DEWMT_OGG);
             return true;
        }
        case DEWMT_OGG_OPUS:
        {
             *plpEngine = new OPUSENGINE();
             AUDIOFACTORY::_GetOggOpusTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
             (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
             if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
             (*plpEngine)->SetContainerFormat(DEWMT_OGG);
             return true;
        }
        case DEWMT_ISOM:
        {
            *plpEngine = new ISOMENGINE();
            AUDIOFACTORY::_GetISOMTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
            (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
            if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
            (*plpEngine)->SetContainerFormat(DEWMT_ISOM);
            return true;
        }
        case DEWMT_3GP:
        {
            *plpEngine = new ISOMENGINE();
            AUDIOFACTORY::_GetISOMTags(audioFile, wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, &pAlbumArt, &bSimulatedTitle);
            (*plpEngine)->Tag.Populate(wsTitle, wsArtist, wsAlbum, wsTrack, wsGenre, wsYear, pAlbumArt, bSimulatedTitle);
            if (pAlbumArt) { delete pAlbumArt; pAlbumArt = NULL; }
            (*plpEngine)->SetContainerFormat(DEWMT_3GP);
            return true;
        }
    }
    return false;
}

bool AUDIOFACTORY::GetAudioPrelimInfo(LPCWSTR audioFile, LPWSTR audioTitle, const DWORD& audioTitleLength, LPDWORD pDuration, LPBYTE pAudioType)
{
    if (!audioFile || !audioTitle || !pDuration || !pAudioType) return false;

    *pDuration = 0;
    *pAudioType = _ParseMediaType(audioFile);

    if (*pAudioType == DEWMT_UNKNOWN) return false;
    switch(*pAudioType)
    {
        case DEWMT_AIFF:
        {
             TagLib::RIFF::AIFF::File aiffFile(audioFile);
             StringCchPrintf(audioTitle, audioTitleLength, L"%s", aiffFile.tag()->title().toCWString());
             if (aiffFile.audioProperties())
                *pDuration = (DWORD)(aiffFile.audioProperties()->lengthInMilliseconds());
             break;
        }
        case DEWMT_APE:
        {
             TagLib::MPC::File apeFile(audioFile);
             StringCchPrintf(audioTitle, audioTitleLength, L"%s", apeFile.tag()->title().toCWString());
             if (apeFile.audioProperties())
                *pDuration = (DWORD)(apeFile.audioProperties()->lengthInMilliseconds());
             break;
        }
        case DEWMT_FLAC:
        case DEWMT_OGG_FLAC:
        {
             TagLib::FLAC::File flacFile(audioFile);
             StringCchPrintf(audioTitle, audioTitleLength, L"%s", flacFile.tag()->title().toCWString());
             if (flacFile.audioProperties())
                *pDuration = (DWORD)(flacFile.audioProperties()->lengthInMilliseconds());
             break;
        }
        case DEWMT_MIDI:
        {
            AUDIOFACTORY::_DeriveFileFromPath(audioFile, audioTitle, audioTitleLength);
            *pDuration = 0;
            /**/ // TODO: This piece of code handles the times. However,
            //            it is excruciatingly slow. Do we want to keep
            //            the duration field empty, even when 'Deep Scan'
            //            is on? If we're okay, then the code below can be scrapped
            /*wchar_t     wsSyntax[MAX_CHAR_PATH]{ 0 };
            wchar_t     wsLength[256]{ 0 };

            StringCchPrintf(wsSyntax, MAX_CHAR_PATH, L"open \"%s\" type sequencer alias midiAudio", audioFile);
            if (mciSendString(wsSyntax, NULL, 0, NULL) == ERROR_SUCCESS)
            {
                if (mciSendString(L"set midiAudio time format milliseconds", NULL, 0, NULL) == ERROR_SUCCESS)
                    if (mciSendString(L"status midiAudio length", wsLength, 256, NULL) == ERROR_SUCCESS)
                        *pDuration = (DWORD)_wtol(wsLength);
                mciSendString(L"close midiAudio", NULL, 0, NULL);
            }*/
            break;
        }
        case DEWMT_MPC:
        {
            TagLib::MPC::File mpcFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", mpcFile.tag()->title().toCWString());
            if (mpcFile.audioProperties())
                *pDuration = (DWORD)(mpcFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_MP3:
        {
            TagLib::MPEG::File mpgFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", mpgFile.tag()->title().toCWString());
            if (mpgFile.audioProperties())
                *pDuration = (DWORD)(mpgFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_OPUS:
        case DEWMT_OGG_OPUS:
        {
            TagLib::Ogg::Opus::File opusFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", opusFile.tag()->title().toCWString());
            if (opusFile.audioProperties())
                *pDuration = (DWORD)(opusFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_OGG_VORBIS:
        {
             TagLib::Ogg::Vorbis::File vorbisFile(audioFile);
             StringCchPrintf(audioTitle, audioTitleLength, L"%s", vorbisFile.tag()->title().toCWString());
             if (vorbisFile.audioProperties())
                *pDuration = (DWORD)(vorbisFile.audioProperties()->lengthInMilliseconds());
             break;
        }
        case DEWMT_WAV:
        {
            TagLib::RIFF::WAV::File wavFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", wavFile.tag()->title().toCWString());
            if (wavFile.audioProperties())
                *pDuration = (DWORD)(wavFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_WMA:
        {
            TagLib::ASF::File asfFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", asfFile.tag()->title().toCWString());
            if (asfFile.audioProperties())
                *pDuration = (DWORD)(asfFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_WV:
        {
            TagLib::WavPack::File wvFile(audioFile);
            StringCchPrintf(audioTitle, audioTitleLength, L"%s", wvFile.tag()->title().toCWString());
            if (wvFile.audioProperties())
                *pDuration = (DWORD)(wvFile.audioProperties()->lengthInMilliseconds());
            break;
        }
        case DEWMT_ISOM:
        case DEWMT_ISOM_AAC:
        case DEWMT_ISOM_AC3:
        case DEWMT_ISOM_ALAC:
        case DEWMT_ISOM_MP3:
        case DEWMT_3GP:
        case DEWMT_3GP_AAC:
        case DEWMT_3GP_AC3:
        case DEWMT_3GP_ALAC:
        case DEWMT_3GP_MP3:
        {
             TagLib::MP4::File isomFile(audioFile);
             StringCchPrintf(audioTitle, audioTitleLength, L"%s", isomFile.tag()->title().toCWString());
             if (isomFile.audioProperties())
                *pDuration = (DWORD)(isomFile.audioProperties()->lengthInMilliseconds());
             break;
        }
        default:
        {
            AUDIOFACTORY::_DeriveFileFromPath(audioFile, audioTitle, audioTitleLength);
            break;
        }
    }

    if (lstrlen(audioTitle) <= 0)
        AUDIOFACTORY::_DeriveFileFromPath(audioFile, audioTitle, audioTitleLength);

    return true;
}

#pragma endregion

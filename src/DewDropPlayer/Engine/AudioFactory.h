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

#ifndef _AUDIOFACTORY_H_
#define _AUDIOFACTORY_H_

#define TAGLIB_STATIC
#define _BE2LE_DW(X)            (((X << 24) & 0xFF000000) | ((X << 8) & 0xFF0000) | ((X >> 8) & 0xFF00) | ((X >> 24) & 0xFF))
#define OGG_SUBPACKET_START     0x1C

#include "IAudioEngine.h"
#include "TagInfo.h"

// Generic/Common Tag Headers
#include "tag\include\tag.h"
#include "tag\include\fileref.h"
#include "tag\include\tpropertymap.h"

// ID3v1/2/MPEG Tag headers
#include "tag\include\id3v1tag.h"
#include "tag\include\id3v2tag.h"
#include "tag\include\id3v2frame.h"
#include "tag\include\mpegfile.h"
#include "tag\include\attachedpictureframe.h"

// Ape Tag Headers
#include "tag\include\apefile.h"
#include "tag\include\apetag.h"

// ASF/WMA Tag Headers
#include "tag\include\asffile.h"
#include "tag\include\asftag.h"

// RIFF/WAV/AIFF Family Tag Headers
#include "tag\include\rifffile.h"
#include "tag\include\wavfile.h"
#include "tag\include\aifffile.h"

// MP4 Tag Headers
#include "tag\include\mp4tag.h"
#include "tag\include\mp4file.h"

// WV Tag Headers
#include "tag\include\wavpackproperties.h"
#include "tag\include\wavpackfile.h"

// Ogg Common Tag Headers
#include "tag\include\oggfile.h"
#include "tag\include\xiphcomment.h"

// Opus/Ogg Opus Tag Headers
#include "tag\include\opusfile.h"

// Flac/Ogg Flac Tag Headers
#include "tag\include\flacfile.h"
#include "tag\include\flacpicture.h"

// Ogg/Vorbis Tag Headers
#include "tag\include\vorbisfile.h"

// MPC Tag Headers
#include "tag\include\mpcfile.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/tag/lib/Debug/tag.lib")
#else
#pragma comment(lib, "./Engine/tag/lib/Release/tag.lib")
#endif
#pragma comment(lib, "gdiplus")
#pragma comment(lib, "shlwapi")

// This class is meant to be an 'on-the-fly' and lightweight class
// So the entire class is static. Should not contain "instantiable" methods
// so that this class can be entirely used without any instance creation,
// reducing memory footprint
// **** Since the entire class is static pay attention to two majot items ****
//      to prevent data corruption/incorrect data carry overs:
// 1. No static member variables (m_xxx...)
// 2. No static variables inside the static methods
typedef class AUDIOFACTORY
{
    private:
        // File signature IDs
        enum
        {
            sigOgg = 0x5367674F, // 'OggS', // 0 - 3 (block 1)

            sigFTYP = 0x70797466, // 'ftyp', // 4 - 7 (block 2)
            sigISOM = 0x6D6F7369, // 'isom', // 8 - B (block 3)
            sigMP4 = 0x34706D, // 'mp4', // 8 - B (block 3)
            sigM4A = 0x41344D, // 'M4A', // 8 - B (block 3)
            sigM4V = 0x56344D, // 'M4V', // 8 - B (block 3)

            sig3GP = 0x706733, // '3gp', // 8 - B (block 3)

            sigAAC1 = 0xF1FF, // 'ÿñ',  // FF F1 (Big-Endian) | 0 - 1 (block 1)
            sigAAC2 = 0xF1F9, // 'ùÿ',  // F9 F1 (Big-Endian) | 0 - 1 (block 1)

            sigAC3 = 0x770B, // 0B 77 (Big-Endian) | 0 - 1 (block 1)

            sigAIFF = 0x4D524F46, // 'FORM', // 0 - 3 (block 1)

            sigAPE = 0x43414D, // 'MAC', // 0 - 2 (block 1)

            sigRIFF = 0x46464952, // 'RIFF', // 0 - 3 (block 1)
            sigCDDA = 0x41444443, // 'CDDA', // 8 - B (block 3)

            sigFLAC = 0x43614C66, //'fLaC', // 0 - 3 (block 1)

            sigMIDI = 0x6468544D, // 'MThd', // 0 - 3 (block 1)

            sigMPC = 0x4B43504D, // 'MPCK', // 0 - 3 (block 1)

            sigMP2 = 0xFDFF, // FF FD (Big-Endian)
            sigMP31 = 0xFBFF, // FF FB (Big-Endian), for older ID3v1 based MP3s | 0 - 1 (block 1)
            sigMP32 = 0x334449, // 'ID3', // 0 - 1 (block 1)

            sigWAV = 0x45564157, // 'WAVE', // 8 - B (block 3)

            sigWMA_Block1 = 0x75B22630, // 0 - 3 (block 1)
            sigWMA_Block2 = 0x11CF668E, // 4 - 7 (block 2)
            sigWMA_Block3 = 0xAA00D9A6, // 8 - B (block 3)
            sigWMA_Block4 = 0x6CCE6200, // C - F (block 4)

            sigWV = 0x6B707677, // 'wvpk' // 0 - 3 (block 1)

            sigOggVorbis = 0x726F7600, // 'vor'
            sigOggFlac = 0x414C4600, // 'FLA'
            sigOggOpus = 0x7375704F // 'Opus'

        };

        // ***** THIS IS THE SINGLE-MOST IMPORTANT FUNCTION IN THIS CLASS ******
        static BYTE         _ParseMediaType(LPCWSTR audioFile);
                             
        static void         _DeriveFileFromPath(LPCWSTR wsPath, LPWSTR wsFileName, const DWORD dwFleNameLen);

                             
        static void         _GetAPEArt(const TagLib::APE::Tag *apeTag, Image** plpCoverArt);
        static void         _GetASFArt(const TagLib::ASF::Tag *asfTag, Image **plpCoverArt);
        static void         _GetFLACArt(TagLib::FLAC::File& flacFile, Image **plpCoverArt);
        static void         _GetID3V2Art(const TagLib::ID3v2::Tag *mpgid3v2Tag, Image **plpCoverArt);
        static void         _GetXiphCommentArt(TagLib::Ogg::XiphComment& xiphTag, Image **plpCoverArt);
                             
        static void         _GetAIFFTags(LPCWSTR wsAIFFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                         LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetAPETags(LPCWSTR wsAPEFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                        LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetASFTags(LPCWSTR wsASFFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                        LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetFLACTags(LPCWSTR wsFLACFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                         LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetISOMTags(LPCWSTR wsISOMFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                         LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetOggOpusTags(LPCWSTR wsOggOpusFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                            LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetOggVorbisTags(LPCWSTR wsVorbisFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                              LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetOpusTags(LPCWSTR wsOpusFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                         LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetMPCTags(LPCWSTR wsMPCFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                        LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetMP3Tags(LPCWSTR wsMP3File, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                        LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetWAVTags(LPCWSTR wsWAVile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                        LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);
        static void         _GetWVTags(LPCWSTR wsWVFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                       LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);

        static void         _GetDefaultTags(LPCWSTR wsFile, LPWSTR wsTitle, LPWSTR wsArtist, LPWSTR wsAlbum,
                                            LPWSTR wsTrack, LPWSTR wsGenre, LPWSTR wsYear, Image** plpCoverArt, LPBOOL pbSimulatedTitle);

    public:
        static BYTE         GetMediaTypeFromExtension(LPCWSTR wsPath);
        static bool         GetAudioEngine(LPCWSTR audioFile, LPIAUDIOENGINE *plpEngine, BYTE mediaType = DEWMT_UNKNOWN);
        static bool         GetAudioPrelimInfo(LPCWSTR audioFile, LPWSTR audioTitle, const DWORD& audioTitleLength, LPDWORD pDuration, LPBYTE pAudioType);

}AUDIOFACTORY, *LPAUDIOFACTORY;


#endif

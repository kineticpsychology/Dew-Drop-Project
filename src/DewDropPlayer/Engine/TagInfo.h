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

#ifndef _TAGINFO_H_
#define _TAGINFO_H_

#ifndef UNICODE
#define UNICODE
#endif

#define TAG_DATA_LENGTH     512

#include "../UI/DewUICommon.h"

using namespace Gdiplus;

typedef class TAGINFO
{
    private:
        wchar_t         wsTitle[TAG_DATA_LENGTH];
        wchar_t         wsArtist[TAG_DATA_LENGTH];
        wchar_t         wsAlbum[TAG_DATA_LENGTH];
        wchar_t         wsTrack[TAG_DATA_LENGTH];
        wchar_t         wsGenre[TAG_DATA_LENGTH];
        wchar_t         wsYear[TAG_DATA_LENGTH];
        Image           *pAlbumArt = NULL;
        BOOL            bSimulatedTitle = TRUE;

    public:
        const wchar_t*  Title;
        const wchar_t*  Artist;
        const wchar_t*  Album;
        const wchar_t*  Track;
        const wchar_t*  Genre;
        const wchar_t*  Year;
        Image*          &AlbumArt;
        const BOOL&     SimulatedTitle;
                        TAGINFO();
        void            Populate(LPCWSTR title, LPCWSTR artist, LPCWSTR album,
                                 LPCWSTR track, LPCWSTR GENRE, LPCWSTR year,
                                 Image* albumArt, BOOL simulatedTitle);
                        ~TAGINFO();

} TAGINFO, *LPTAGINFO;

#endif


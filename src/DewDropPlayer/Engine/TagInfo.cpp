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

#include "TagInfo.h"

TAGINFO::TAGINFO() : Title(wsTitle), Artist(wsArtist), Album(wsAlbum),
                    Track(wsTrack), Genre(wsGenre), Year(wsYear),
                    AlbumArt(pAlbumArt), SimulatedTitle(bSimulatedTitle)
{
    pAlbumArt = NULL;
    // Blank-ify all the fields
    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"");
    StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"");
    bSimulatedTitle = TRUE;

    return;
}

void TAGINFO::Populate(LPCWSTR title, LPCWSTR artist, LPCWSTR album, LPCWSTR track, LPCWSTR genre, LPCWSTR year, Image * albumArt, BOOL simulatedTitle)
{
    StringCchPrintf(wsTitle, TAG_DATA_LENGTH, L"%s", title);
    StringCchPrintf(wsArtist, TAG_DATA_LENGTH, L"%s", artist);
    StringCchPrintf(wsAlbum, TAG_DATA_LENGTH, L"%s", album);
    StringCchPrintf(wsTrack, TAG_DATA_LENGTH, L"%s", track);
    StringCchPrintf(wsGenre, TAG_DATA_LENGTH, L"%s", genre);
    StringCchPrintf(wsYear, TAG_DATA_LENGTH, L"%s", year);
    if (albumArt)
        pAlbumArt = albumArt->Clone();
    bSimulatedTitle = simulatedTitle;
    return;
}

TAGINFO::~TAGINFO()
{
    if (pAlbumArt)
    {
        delete pAlbumArt;
        pAlbumArt = NULL;
    }
    return;
}

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

#ifndef _DEWPLAYLISTPARSER_H_
#define _DEWPLAYLISTPARSER_H_

#include "DewUICommon.h"

#define DEW_PLFILE_EOL_WINDOWS          0x00
#define DEW_PLFILE_EOL_UNIX             0x01
#define DEW_PLFILE_EOL_MAC              0x02
#define DEW_PLFILE_EOL_INVALID          0xFF

#define DEW_PLFILE_ENCODING_ANSI        0x00
#define DEW_PLFILE_ENCODING_UTF8        0x01
#define DEW_PLFILE_ENCODING_UNICODE     0x02

#define DEW_PLFILE_TYPE_UNSUPPORTED     0x00
#define DEW_PLFILE_TYPE_M3U             0x01
#define DEW_PLFILE_TYPE_PLS             0x02
#define DEW_PLFILE_TYPE_CUE             0x03

// Please do NOT inherit
typedef class DEWPLAYLISTPARSER
{
    private:
        LPFILELIST          _lpLines = NULL;
        LPFILELIST          _lpFiles = NULL;
        UINT                _nLineCount = 0;
        UINT                _nFileCount = 0;
        BYTE                _btEOLType = 0;
        BYTE                _btEncType = 0;
        BYTE                _btPlaylistType;
        BOOL                _bError = FALSE;

        void                _FlushLines();
        void                _FlushFiles();
        BOOL                _PopulateM3UFiles(LPCWSTR wsPlaylistFile, LPFILELIST lpHeaderPos);
        BOOL                _PopulatePLSFiles(LPCWSTR wsPlaylistFile, LPFILELIST lpHeaderPos);
        BOOL                _PopulateCUEFiles(LPCWSTR wsPlaylistFile);

    public:
        const BYTE&         EncodingType;
        const BYTE&         EOLType;
        const BYTE&         PlaylistType;
        const BOOL&         HasError;
        const UINT&         FileCount;
        const LPFILELIST&   Files;

                            DEWPLAYLISTPARSER();
                            DEWPLAYLISTPARSER(LPCWSTR wsPlaylistFile);
        void                ParsePlaylistFile(LPCWSTR wsPlaylistFile);
        void                Reset();
                            ~DEWPLAYLISTPARSER();
} DEWPLAYLISTPARSER, *LPDEWPLAYLISTPARSER;

#endif // _DEWPLAYLISTPARSER_H_

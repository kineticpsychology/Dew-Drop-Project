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

#include "DewPlaylistParser.h"

void DEWPLAYLISTPARSER::_FlushLines()
{
    LPFILELIST      lpCurr = NULL;
    if (_lpLines)
    {
        do
        {
            lpCurr = _lpLines;
            _lpLines = _lpLines->next;
            LocalFree(lpCurr);
        } while(_lpLines);
    }
    _nLineCount = 0;
    return;
}

void DEWPLAYLISTPARSER::_FlushFiles()
{
    LPFILELIST      lpCurr = NULL;

    if (_lpFiles)
    {
        do
        {
            lpCurr = _lpFiles;
            _lpFiles = _lpFiles->next;
            LocalFree(lpCurr);
        } while(_lpFiles);
    }
    _nFileCount = 0;
}

BOOL DEWPLAYLISTPARSER::_PopulateM3UFiles(LPCWSTR wsPlaylistFile, LPFILELIST lpHeaderPos)
{
    LPFILELIST      lpLineCurr = NULL, lpFileCurr = NULL, lpFilePrev = NULL;
    wchar_t         wsPlaylistPath[MAX_CHAR_PATH] { 0 }, wsFile[MAX_CHAR_PATH];
    UINT            nIndex;
    DWORD           dwAttr;

    this->_FlushFiles();
    _nFileCount = 0;

    if (!wsPlaylistFile)
    {
        return FALSE;
    }

    // It is assumed that wsPlaylistFile will be a valid path to a valid playlist file
    // Do NOT call from anywhere other than CTOR()!
    CopyMemory(wsPlaylistPath, wsPlaylistFile, lstrlen(wsPlaylistFile) * sizeof(wchar_t));
    for (nIndex = (UINT)lstrlen(wsPlaylistPath) - 1; nIndex; nIndex--)
    {
        if (wsPlaylistPath[nIndex] == L'\\')
        {
            wsPlaylistPath[nIndex] = L'\0';
            break;
        }
    }

    lpLineCurr = ((lpHeaderPos == NULL) ? _lpLines : lpHeaderPos);

    for (; lpLineCurr; lpLineCurr = lpLineCurr->next)
    {
        // Needs to be a valid file entry. This means it cannot be zero length
        // and cannot start with a '#' (which is a M3U directive)
        if (lstrlen(lpLineCurr->wsFileName) > 0 &&
            lpLineCurr->wsFileName[0] != L'#')
        {
            // Before inserting, make sure it is a valid file
            ZeroMemory(wsFile, MAX_CHAR_PATH * sizeof(wchar_t));

            if (StrStrI((lpLineCurr->wsFileName), L":")) // Absolute path
                CopyMemory(wsFile, (lpLineCurr->wsFileName), lstrlen((lpLineCurr->wsFileName)) * sizeof(wchar_t));
            else // Relative Path
                StringCchPrintf(wsFile, MAX_CHAR_PATH, L"%s\\%s", wsPlaylistPath, (lpLineCurr->wsFileName));
            dwAttr = GetFileAttributes(wsFile);
            // Do not add to the list if the file is invalid or a directory
            if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                continue;

            lpFileCurr = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST));
            CopyMemory((lpFileCurr->wsFileName), wsFile, lstrlen(wsFile) * sizeof(wchar_t));
            lpFileCurr->next = NULL;
            _nFileCount++;
            if (!_lpFiles)
                _lpFiles = lpFileCurr;
            else
                lpFilePrev->next = lpFileCurr;
            lpFilePrev = lpFileCurr;

            // Check if path is relative
        }
    }
    // All done!
    return TRUE;
}

BOOL DEWPLAYLISTPARSER::_PopulatePLSFiles(LPCWSTR wsPlaylistFile, LPFILELIST lpHeaderPos)
{
    LPFILELIST      lpLineCurr = NULL, lpFileCurr = NULL, lpFilePrev = NULL;
    wchar_t         wsPlaylistPath[MAX_CHAR_PATH] { 0 }, wsFile[MAX_CHAR_PATH];
    UINT            nIndex;
    DWORD           dwAttr;
    wchar_t         wsKeyword[16];

    this->_FlushFiles();
    _nFileCount = 0;

    if (!wsPlaylistFile)
    {
        return FALSE;
    }

    // It is assumed that wsPlaylistFile will be a valid path to a valid playlist file
    // Do NOT call from anywhere other than CTOR()!
    CopyMemory(wsPlaylistPath, wsPlaylistFile, lstrlen(wsPlaylistFile) * sizeof(wchar_t));
    for (nIndex = (UINT)lstrlen(wsPlaylistPath) - 1; nIndex; nIndex--)
    {
        if (wsPlaylistPath[nIndex] == L'\\')
        {
            wsPlaylistPath[nIndex] = L'\0';
            break;
        }
    }

    // Since this is like an ini file, the FileXX cannot exceed
    // the total line numbers - 1 (exclude the '[playlist]' header)
    for (nIndex = 0; nIndex < _nLineCount-1; nIndex++)
    {
        StringCchPrintf(wsKeyword, 16, L"File%u=", nIndex + 1);
        lpLineCurr = ((lpHeaderPos == NULL) ? _lpLines : lpHeaderPos);
        for (; lpLineCurr; lpLineCurr = lpLineCurr->next)
        {
            if (lstrlen(lpLineCurr->wsFileName) > lstrlen(wsKeyword))
            {
                // Starts with "FileXX="
                if ((lpLineCurr->wsFileName) == StrStrI(lpLineCurr->wsFileName, wsKeyword))
                {
                    // Before inserting, make sure it is a valid file
                    ZeroMemory(wsFile, MAX_CHAR_PATH * sizeof(wchar_t));

                    if (StrStrI((lpLineCurr->wsFileName), L":")) // Absolute path
                        CopyMemory(wsFile, &(lpLineCurr->wsFileName[lstrlen(wsKeyword)]),
                                   (lstrlen(lpLineCurr->wsFileName) - lstrlen(wsKeyword)) * sizeof(wchar_t));
                    else // Relative Path
                        StringCchPrintf(wsFile, MAX_CHAR_PATH, L"%s\\%s", wsPlaylistPath, &(lpLineCurr->wsFileName[lstrlen(wsKeyword)]));
                    dwAttr = GetFileAttributes(wsFile);
                    // Do not add to the list if the file is invalid or a directory
                    if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                        continue;

                    lpFileCurr = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST));
                    CopyMemory((lpFileCurr->wsFileName), wsFile, lstrlen(wsFile) * sizeof(wchar_t));
                    lpFileCurr->next = NULL;
                    _nFileCount++;
                    if (!_lpFiles)
                        _lpFiles = lpFileCurr;
                    else
                        lpFilePrev->next = lpFileCurr;
                    lpFilePrev = lpFileCurr;
                    break;
                }
            }
        }
    }

    return TRUE;
}

BOOL DEWPLAYLISTPARSER::_PopulateCUEFiles(LPCWSTR wsPlaylistFile)
{
    LPFILELIST      lpLineCurr = NULL, lpFileCurr = NULL, lpFilePrev = NULL;
    wchar_t         wsPlaylistPath[MAX_CHAR_PATH] { 0 }, wsFile[MAX_CHAR_PATH];
    UINT            nIndex;
    DWORD           dwAttr;
    wchar_t         wsFilePart[MAX_CHAR_PATH];

    this->_FlushFiles();
    _nFileCount = 0;

    if (!wsPlaylistFile)
        return FALSE;

    // It is assumed that wsPlaylistFile will be a valid path to a valid playlist file
    // Do NOT call from anywhere other than CTOR()!
    CopyMemory(wsPlaylistPath, wsPlaylistFile, lstrlen(wsPlaylistFile) * sizeof(wchar_t));
    for (nIndex = (UINT)lstrlen(wsPlaylistPath) - 1; nIndex; nIndex--)
    {
        if (wsPlaylistPath[nIndex] == L'\\')
        {
            wsPlaylistPath[nIndex] = L'\0';
            break;
        }
    }

    for (lpLineCurr = _lpLines; lpLineCurr; lpLineCurr = lpLineCurr->next)
    {
        if (lstrlen(lpLineCurr->wsFileName) > 7) // The texts 'FILE', two '"' and a whitespace
        {
            if ((lpLineCurr->wsFileName) == StrStrI(lpLineCurr->wsFileName, L"FILE") && // Line starts with 'FILE'
                StrStrI(lpLineCurr->wsFileName, L"\"") < StrRStrI(lpLineCurr->wsFileName, NULL, L"\"")) // There are two '"'s
            {
                ZeroMemory(wsFilePart, MAX_PATH * sizeof(wchar_t));
                CopyMemory(wsFilePart, &(lpLineCurr->wsFileName[(StrStrI(lpLineCurr->wsFileName, L"\"")) - (lpLineCurr->wsFileName) + 1]),
                           (StrRStrI(lpLineCurr->wsFileName, NULL, L"\"") - StrStrI(lpLineCurr->wsFileName, L"\"") - 1) * sizeof(wchar_t));
                if (StrStrI(lpLineCurr->wsFileName, L":")) // Absolute Path
                {
                    CopyMemory(wsFile, wsFilePart, MAX_CHAR_PATH * sizeof(wchar_t));
                }
                else
                {
                    StringCchPrintf(wsFile, MAX_CHAR_PATH, L"%s\\%s", wsPlaylistPath, wsFilePart);
                }

                dwAttr = GetFileAttributes(wsFile);
                // Do not add to the list if the file is invalid or a directory
                if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                    continue;

                lpFileCurr = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST));
                CopyMemory((lpFileCurr->wsFileName), wsFile, lstrlen(wsFile) * sizeof(wchar_t));
                lpFileCurr->next = NULL;
                _nFileCount++;
                if (!_lpFiles)
                    _lpFiles = lpFileCurr;
                else
                    lpFilePrev->next = lpFileCurr;
                lpFilePrev = lpFileCurr;
            }
        }
    }
    return TRUE;
}

DEWPLAYLISTPARSER::DEWPLAYLISTPARSER() :
EncodingType(_btEncType), EOLType (_btEOLType), PlaylistType(_btPlaylistType),
HasError (_bError), FileCount(_nFileCount), Files(_lpFiles)
{} // NOP

DEWPLAYLISTPARSER::DEWPLAYLISTPARSER(LPCWSTR wsPlaylistFile) :
EncodingType(_btEncType), EOLType (_btEOLType), PlaylistType(_btPlaylistType),
HasError (_bError), FileCount(_nFileCount), Files(_lpFiles)
{
    this->ParsePlaylistFile(wsPlaylistFile);
    return;
}

void DEWPLAYLISTPARSER::ParsePlaylistFile(LPCWSTR wsPlaylistFile)
{
    HANDLE      hFile = NULL;
    DWORD       dwAttr = 0, dwRawSize = 0;
    LPBYTE      lpData;
    wchar_t     *wsData = NULL;
    UINT        nOffset = 0x00, nIndex = 0;
    LPFILELIST  lpPrev = NULL, lpCurr = NULL;
    BOOL        bPlaylistHeaderFound = FALSE;
    FILE        *fpPLFile = NULL;

    #pragma region RESET SECTION

    this->Reset();

    #pragma endregion

    #pragma region ENCODING DETECTION

    // wsPlaylistFile must be absolute
    dwAttr = GetFileAttributes(wsPlaylistFile);
    if (dwAttr == INVALID_FILE_ATTRIBUTES) return; // Cannot be invalid path
    if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return; // Cannot be directory

    hFile = CreateFile(wsPlaylistFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return;
    dwRawSize = GetFileSize(hFile, NULL);
    // File cannot be lesser than 4 bytes
    if (dwRawSize < 4) { CloseHandle(hFile); return; }
    lpData = (LPBYTE)LocalAlloc(LPTR, dwRawSize);
    ReadFile(hFile, lpData, dwRawSize, &dwRawSize, NULL);
    CloseHandle(hFile);

    // UNICODE BOM
    if (lpData[0x00] == 0xFF && lpData[0x01] == 0xFE)
    {
        _btEncType = DEW_PLFILE_ENCODING_UNICODE;
        nOffset = 0x02; // Data starts from 2th index
    }
    // UTF-8 BOM
    else if (lpData[0x00] == 0xEF && lpData[0x01] == 0xBB && lpData[0x02] == 0xBF)
    {
        _btEncType = DEW_PLFILE_ENCODING_UTF8;
        nOffset = 0x03; // Data starts from 3th index
    }

    #pragma endregion

    #pragma region EOL DETECTION

    _btEOLType = DEW_PLFILE_EOL_WINDOWS;
    for (nIndex = nOffset; nIndex < dwRawSize; nIndex++)
    {
        if (_btEncType != DEW_PLFILE_ENCODING_UNICODE && nIndex < (dwRawSize - 1) &&
            lpData[nIndex] == 0x0D && lpData[nIndex+1] == 0x0A) // Windows
        {
            _btEOLType = DEW_PLFILE_EOL_WINDOWS;
            break;
        }
        else if (_btEncType == DEW_PLFILE_ENCODING_UNICODE && nIndex <= (dwRawSize - 4) &&
                 lpData[nIndex] == 0x0D && lpData[nIndex+1] == 0x00 &&
                 lpData[nIndex+2] == 0x0A && lpData[nIndex+3] == 0x00) // Windows
        {
            _btEOLType = DEW_PLFILE_EOL_WINDOWS;
            break;
        }
        else if (lpData[nIndex] == 0x0A) // Unix
        {
            _btEOLType = DEW_PLFILE_EOL_UNIX;
            break;
        }
        else if (lpData[nIndex] == 0x0D) // MAC
        {
            _btEOLType = DEW_PLFILE_EOL_MAC;
            break;
        }

    }

    #pragma endregion

    // If there are no new-lines, then it is an invalid file
    // Technically, it could be a valid file, but not a valid Playlist file
    if (_btEOLType == DEW_PLFILE_EOL_INVALID) return;

    #pragma region LINE PARSING AND POPULATION

    LocalFree(lpData);

    wsData = (wchar_t*)LocalAlloc(LPTR, dwRawSize * sizeof(wchar_t));
    fpPLFile = NULL;
    _wfopen_s(&fpPLFile, wsPlaylistFile, L"rt, ccs=UTF-8");
    if (fpPLFile)
    {
        fread(wsData, dwRawSize * sizeof(wchar_t), 1, fpPLFile);
        fclose(fpPLFile);
        fpPLFile = NULL;
    }

    nOffset = 0;
    for (nIndex = 0; nIndex < (UINT)lstrlen(wsData); nIndex++)
    {
        if (wsData[nIndex] == L'\n')
        {
            _nLineCount++;
            lpCurr = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST));
            CopyMemory((lpCurr->wsFileName), &(wsData[nOffset]), (nIndex - nOffset)*sizeof(wchar_t));
            nOffset = nIndex + 1;
            lpCurr->next = NULL;
            if (!_lpLines)
                _lpLines = lpCurr;
            else
                lpPrev->next = lpCurr;
            lpPrev = lpCurr;
        }
    }
    // Last stop-check in cases where the '\n' is not the last data
    // In that case, the last line would also have to be included
    if (nIndex == (UINT)lstrlen(wsData) && wsData[nIndex] != L'\n')
    {
        _nLineCount++;
        lpCurr = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST));
        CopyMemory((lpCurr->wsFileName), &(wsData[nOffset]), (nIndex - nOffset)*sizeof(wchar_t));
        nOffset = nIndex + 1;
        lpCurr->next = NULL;
        if (!_lpLines)
            _lpLines = lpCurr;
        else
            lpPrev->next = lpCurr;
        lpPrev = lpCurr;
    }
    LocalFree(wsData);
    wsData = NULL;

    #pragma endregion


    #pragma region PLS vs M3U(8) vs CUE seggregation

    for(lpCurr = _lpLines; lpCurr; lpCurr = lpCurr->next)
    {
        // Ignoring multi-line returns, #EXTM3U or #M3U is the first keyword
        if ( (!lstrcmpi((lpCurr->wsFileName), L"#EXTM3U") ||
              !lstrcmpi((lpCurr->wsFileName), L"#M3U")) &&
              !bPlaylistHeaderFound )
        {
            _btPlaylistType = DEW_PLFILE_TYPE_M3U;
            bPlaylistHeaderFound = TRUE;
            break;
        }
        // Ignoring multi-line returns, [PLAYLIST] is the first keyword
        else if (!lstrcmpi((lpCurr->wsFileName), L"[PLAYLIST]") &&
                 !bPlaylistHeaderFound)
        {
            _btPlaylistType = DEW_PLFILE_TYPE_PLS;
            bPlaylistHeaderFound = TRUE;
            break;
        }
        // Some other thing is the first line. That is not supported
        else if (lstrlen(lpCurr->wsFileName) > 0 && !bPlaylistHeaderFound)
        {
            break;
        }
    }

    // If still nothing is found, we'll make a last attempt
    // and will go with the file extension
    if (!bPlaylistHeaderFound)
    {
        if (!lstrcmpi(PathFindExtension(wsPlaylistFile), L".cue"))
        {
            _btPlaylistType = DEW_PLFILE_TYPE_CUE;
            bPlaylistHeaderFound = TRUE;
            lpCurr = NULL;
        }
        else if (!lstrcmpi(PathFindExtension(wsPlaylistFile), L".m3u8") ||
                 !lstrcmpi(PathFindExtension(wsPlaylistFile), L".m3u"))
        {
            _btPlaylistType = DEW_PLFILE_TYPE_M3U;
            bPlaylistHeaderFound = TRUE;
            lpCurr = NULL;
        }
        else if (!lstrcmpi(PathFindExtension(wsPlaylistFile), L".pls"))
        {
            _btPlaylistType = DEW_PLFILE_TYPE_PLS;
            bPlaylistHeaderFound = TRUE;
        }
    }

    // No header was found. Bail with error set
    if (!bPlaylistHeaderFound)
    {
        this->_FlushLines();
        return;
    }

    #pragma endregion

    #pragma region INDIVIDUAL FORMAT PARSER

    // Now we have the position of the header of the playlist
    // Invoke respective functions to parse and add files to the list
    if (_btPlaylistType == DEW_PLFILE_TYPE_M3U)
        _bError = !this->_PopulateM3UFiles(wsPlaylistFile, lpCurr);
    else if (_btPlaylistType == DEW_PLFILE_TYPE_PLS)
        _bError = !this->_PopulatePLSFiles(wsPlaylistFile, lpCurr);
    else if (_btPlaylistType == DEW_PLFILE_TYPE_CUE)
        _bError = !this->_PopulateCUEFiles(wsPlaylistFile);

    #pragma endregion

    return;
}

void DEWPLAYLISTPARSER::Reset()
{
    this->_FlushFiles();
    this->_FlushLines();
    _bError = TRUE;
    _btEOLType = DEW_PLFILE_EOL_WINDOWS;
    _btEncType = DEW_PLFILE_ENCODING_ANSI;
    _btPlaylistType = DEW_PLFILE_TYPE_UNSUPPORTED;
    return;
}

DEWPLAYLISTPARSER::~DEWPLAYLISTPARSER()
{
    this->Reset();
    return;
}

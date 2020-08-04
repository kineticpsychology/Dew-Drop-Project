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

#ifndef _DEWPLAYLISTEXPORTER_H_
#define _DEWPLAYLISTEXPORTER_H_

#include "DewUIObject.h"
#include "DewIconRepository.h"

// Do not inherit this class please!
typedef class DEWPLAYLISTEXPORTER
{
    private:
        int                 _iDPI = 96;
        float               _fScale = 1.0f;
        HINSTANCE           _hInstance = NULL;
        DWORD               _dwUserData;
        LPDEWUIOBJECT       _Parent = NULL;
        LPDEWUIOBJECT       _Playlist = NULL;
        LPDEWICONREPOSITORY _lpIconRepo = NULL;
        HWND                _hWndExporter = NULL;
        HWND                _hCmdM3U8 = NULL, _hCmdPLS = NULL, _hCmdCancel = NULL;
        HWND                _hChkRelative = NULL;
        DEWPLAYLISTEXPORTINFO   _ExportInfo;

        static LRESULT CALLBACK _ExportMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void        _ShowExportDialog();
        void        _HandleSizing();
        void        _SetChildFont(HWND hWndParent, const wchar_t *wsFontName, int iFontSize);

    public:
        const DEWPLAYLISTEXPORTINFO&    PlaylistExportInfo;

                    DEWPLAYLISTEXPORTER(LPDEWUIOBJECT Parent, LPDEWUIOBJECT Playlist, int iDPI, float fScale);
        // Two references are needed so that this module can inform:
        // 1. The parent window, to take back focus
        // 2. The playlist, so that it can continue the export process
        HWND        Show(DWORD dwUserData);
                    ~DEWPLAYLISTEXPORTER();
} DEWPLAYLISTEXPORTER, *LPDEWPLAYLISTEXPORTER;

#endif // _PLAYLISTEXPORTER_H_

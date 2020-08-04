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

#include "DewPlaylistExporter.h"

LRESULT CALLBACK DEWPLAYLISTEXPORTER::_ExportMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWPLAYLISTEXPORTER)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWPLAYLISTEXPORTER::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    LPWINDOWPOS     lpPos;
    RECT            rctWorkArea;

    switch(nMsg)
    {
        case WM_COMMAND:
        {
            if ((HWND)lParam == _hCmdM3U8)
            {
                this->_ExportInfo.DEWUI_PL_EXPORT_FMT = DEWUI_PL_EXPORT_FMT_M3U8;
                this->_ShowExportDialog();
                break;
            }
            else if ((HWND)lParam == _hCmdPLS)
            {
                this->_ExportInfo.DEWUI_PL_EXPORT_FMT = DEWUI_PL_EXPORT_FMT_PLS;
                this->_ShowExportDialog();
                return FALSE;
            }
            else if ((HWND)lParam == _hCmdCancel)
            {
                SendMessage(_hWndExporter, WM_CLOSE, 0, 0);
                return FALSE;
            }
            else if ((HWND)lParam == _hChkRelative)
            {
                _ExportInfo.bUseRelativePath = (BOOL)(SendMessage(_hChkRelative, BM_GETCHECK, 0, 0) == BST_CHECKED);
                return FALSE;
            }
            else if (lParam == 0x00 && LOWORD(wParam) == IDCANCEL)
            {
                SendMessage(_hWndExporter, WM_CLOSE, 0, 0);
                return FALSE;
            }
            break;
        }
        case WM_SIZE:
        {
            this->_HandleSizing();
            return FALSE;
        }
        // Do not allow the window to move out of the work area
        case WM_WINDOWPOSCHANGING:
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
            lpPos = (LPWINDOWPOS)lParam;
            if (lpPos->cx + lpPos->x > rctWorkArea.right) lpPos->x = rctWorkArea.right - lpPos->cx;
            if (lpPos->x < rctWorkArea.left) lpPos->x = rctWorkArea.left;
            if (lpPos->cy + lpPos->y > rctWorkArea.bottom) lpPos->y = rctWorkArea.bottom - lpPos->cy;
            if (lpPos->y < rctWorkArea.top) lpPos->y = rctWorkArea.top;

            return 0;
        }
        case WM_CLOSE:
        {
            ShowWindow(_hWndExporter, SW_HIDE);
            SendMessage(_Parent->Handle, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWndExporter);
            SendMessage(_Playlist->Handle, WM_DEWMSG_PL_EXPORT_SEL, (WPARAM)_dwUserData, (LPARAM)&_ExportInfo);
            SetForegroundWindow(_Parent->Handle);
            return TRUE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWPLAYLISTEXPORTER::_ShowExportDialog()
{
    OPENFILENAME    ofn { 0 };

    ZeroMemory(_ExportInfo.wsExportFile, sizeof(wchar_t) * MAX_CHAR_PATH);
    _ExportInfo.bOperationValid = FALSE;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWndExporter;
    ofn.lpstrFile = (_ExportInfo.wsExportFile);
    ofn.nMaxFile = MAX_CHAR_PATH;
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (_ExportInfo.DEWUI_PL_EXPORT_FMT == DEWUI_PL_EXPORT_FMT_M3U8)
    {
        ofn.lpstrFilter = L"M3U8 Playlists (*.m3u8)\0*.m3u8\0\0";
        ofn.lpstrTitle = L"Export M3U8 playlist";
        ofn.lpstrDefExt = L".m3u8";
    }
    else if (_ExportInfo.DEWUI_PL_EXPORT_FMT == DEWUI_PL_EXPORT_FMT_PLS)
    {
        ofn.lpstrFilter = L"PLS Playlists (*.pls)\0*.pls\0\0";
        ofn.lpstrTitle = L"Export PLS playlist";
        ofn.lpstrDefExt = L".pls";
    }
    else
    {
        return;
    }

    if (GetSaveFileName(&ofn))
    {
        _ExportInfo.bOperationValid = TRUE;
        SendMessage(_hWndExporter, WM_CLOSE, 0, 0);
    }

    return;
}

void DEWPLAYLISTEXPORTER::_HandleSizing()
{
    RECT    rctClient;
    int     iX, iY, iW, iH, iYPad;

    GetClientRect(_hWndExporter, &rctClient);
    iYPad = _F(50);

    iX = _F(10);
    iW = (rctClient.right - rctClient.left) - _F(20);
    iH = ((rctClient.bottom - rctClient.top) - iYPad - _F(20)) / 2;

    iY = _F(10);
    SetWindowPos(_hCmdM3U8, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iY += iH + _F(10);
    SetWindowPos(_hCmdPLS, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iY = (rctClient.bottom - rctClient.top) - _F(40);
    iW = _F(75);
    iH = _F(30);

    iX = _F(10);
    iW = (rctClient.right - rctClient.left) - _F(105);
    SetWindowPos(_hChkRelative, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = (rctClient.right - rctClient.left) - _F(85);
    iW = _F(75);
    SetWindowPos(_hCmdCancel, 0, iX, iY, iW, iH, SWP_NOZORDER);
    return;
}

void DEWPLAYLISTEXPORTER::_SetChildFont(HWND hWndParent, const wchar_t *wsFontName, int iFontSize)
{
    HWND        hWndChild = NULL;
    LOGFONT     lgfFont { 0 };

    hWndChild = FindWindowEx(hWndParent, NULL, NULL, NULL);
    if(!hWndChild) return;

    ZeroMemory(&lgfFont, sizeof(LOGFONT));
    lgfFont.lfHeight = -MulDiv(iFontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, wsFontName, 32 * sizeof(wchar_t));

    SendMessage(hWndChild, WM_SETFONT,
                (WPARAM)CreateFontIndirect(&lgfFont), MAKELPARAM(TRUE, 0));
    do
    {
        hWndChild = FindWindowEx(hWndParent, hWndChild, NULL, NULL);
        if(hWndChild)
            SendMessage(hWndChild, WM_SETFONT,
                        (WPARAM)CreateFontIndirect(&lgfFont), MAKELPARAM(TRUE, 0));
    }while(hWndChild);
    return;
}

DEWPLAYLISTEXPORTER::DEWPLAYLISTEXPORTER(LPDEWUIOBJECT Parent, LPDEWUIOBJECT Playlist, int iDPI, float fScale) :
PlaylistExportInfo(_ExportInfo)
{
    int             iWinWidth, iWinHeight;
    int             iScreenWidth, iScreenHeight, iXPos, iYPos;
    RECT            rctParent, rctMrg;
    WNDCLASSEX      wcex { 0 };
    const wchar_t   *wsPLExporterClass = L"DEWDROP.PLEXPORTER.WND";
    const wchar_t   *wsTitle = L"Select Playlist Export Format";
    const DWORD     dwBtnStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_FLAT;
    const DWORD     dwChkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;
    const DWORD     dwWinStyle = WS_CAPTION | WS_SYSMENU;
    BUTTON_IMAGELIST    bImgCancel { 0 };

    iWinWidth = _F(320);
    iWinHeight = _F(260);
    _Parent = Parent;
    _Playlist = Playlist;
    _iDPI = iDPI;
    _fScale = fScale;
    _hInstance = GetModuleHandle(NULL);
    _lpIconRepo = new DEWICONREPOSITORY(_iDPI, _fScale);
    ZeroMemory(&_ExportInfo, sizeof(DEWPLAYLISTEXPORTINFO));

    if (_Parent->Handle == NULL)
    {
        iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        iXPos = (iScreenWidth - iWinWidth)/2;
        iYPos = (iScreenHeight - iWinHeight)/2;
    }
    else
    {
        GetWindowRect(_Parent->Handle, &rctParent);
        iScreenWidth = rctParent.right - rctParent.left;
        iScreenHeight = rctParent.bottom - rctParent.top;
        iXPos = (iScreenWidth - iWinWidth)/2 + rctParent.left;
        iYPos = (iScreenHeight - iWinHeight)/2 + rctParent.top;
    }

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsPLExporterClass;

    RegisterClassEx(&wcex);

    _hWndExporter = CreateWindowEx(WS_EX_TOOLWINDOW, wsPLExporterClass, wsTitle, dwWinStyle,
                                   iXPos, iYPos, iWinWidth, iWinHeight,
                                   _Playlist->Handle, NULL, _hInstance, NULL);
    _hCmdM3U8 = CreateWindowEx(0, L"BUTTON", L"M3U8", dwBtnStyle | BS_DEFCOMMANDLINK,
                               0, 0, 0, 0, _hWndExporter, NULL, _hInstance, NULL);
    _hCmdPLS = CreateWindowEx(0, L"BUTTON", L"PLS", dwBtnStyle | BS_COMMANDLINK,
                              0, 0, 0, 0, _hWndExporter, NULL, _hInstance, NULL);
    _hChkRelative = CreateWindowEx(0, L"BUTTON", L"Use &Relative Paths", dwChkStyle,
                                   0, 0, 0, 0, _hWndExporter, NULL, _hInstance, NULL);
    _hCmdCancel = CreateWindowEx(0, L"BUTTON", L"&Cancel", dwBtnStyle,
                                 0, 0, 0, 0, _hWndExporter, NULL, _hInstance, NULL);
    rctMrg.left = _F(5);
    rctMrg.top = _F(1);
    rctMrg.right = _F(1);
    rctMrg.bottom = _F(1);

    bImgCancel.himl = _lpIconRepo->ImgCancel;
    bImgCancel.margin = rctMrg;
    bImgCancel.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;
    SendMessage(_hCmdCancel, BCM_SETIMAGELIST, 0, (LPARAM)&bImgCancel);

    SendMessage(_hCmdM3U8, BCM_SETNOTE, 0, (LPARAM)L"A universally accepted UTF-8 encoded playlist format");
    SendMessage(_hCmdPLS, BCM_SETNOTE, 0, (LPARAM)L"A less common ini-like UTF-8 encoded playlist format");
    this->_SetChildFont(_hWndExporter, L"Tahoma", 8);
    SetWindowSubclass(_hWndExporter, _ExportMsgHandler, (UINT_PTR)_hWndExporter, (DWORD_PTR)this);

    return;
}

HWND DEWPLAYLISTEXPORTER::Show(DWORD dwUserData)
{
    RECT        rctThis, rctParent;
    int         iXPos, iYPos, iWinWidth, iWinHeight, iScreenWidth, iScreenHeight;

    if (!_Parent || !_Playlist || !_hWndExporter) return NULL;

    GetWindowRect(_Parent->Handle, &rctParent);
    GetWindowRect(_hWndExporter, &rctThis);
    iScreenWidth = rctParent.right - rctParent.left;
    iScreenHeight = rctParent.bottom - rctParent.top;
    iWinWidth = rctThis.right - rctThis.left;
    iWinHeight = rctThis.bottom - rctThis.top;

    iXPos = (iScreenWidth - iWinWidth)/2 + rctParent.left;
    iYPos = (iScreenHeight - iWinHeight)/2 + rctParent.top;

    _dwUserData = dwUserData;
    // Reset status (Except _ExportInfo.bUseRelativePath)
    ZeroMemory(_ExportInfo.wsExportFile, sizeof(wchar_t) * MAX_CHAR_PATH);
    _ExportInfo.bOperationValid = FALSE;
    _ExportInfo.DEWUI_PL_EXPORT_FMT = DEWUI_PL_EXPORT_FMT_M3U8;
    _ExportInfo.bUseRelativePath = (BOOL)(SendMessage(_hChkRelative, BM_GETCHECK, 0, 0) == BST_CHECKED);

    SetWindowPos(_hWndExporter, 0, iXPos, iYPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    if (_Parent) EnableWindow(_Parent->Handle, FALSE);
    ShowWindow(_hWndExporter, SW_SHOWNORMAL);
    SetFocus(_hCmdM3U8);
    UpdateWindow(_hWndExporter);
    SetActiveWindow(_hWndExporter);

    return _hWndExporter;
}

DEWPLAYLISTEXPORTER::~DEWPLAYLISTEXPORTER()
{
    DestroyWindow(_hCmdM3U8);
    DestroyWindow(_hCmdPLS);
    DestroyWindow(_hCmdCancel);
    DestroyWindow(_hWndExporter);
    if (_lpIconRepo) { delete _lpIconRepo; _lpIconRepo = NULL; }
    return;
}

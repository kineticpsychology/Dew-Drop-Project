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

#include "DewPropertyPage.h"

LRESULT CALLBACK DEWPROPERTYPAGE::_DewPropPageMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam,
                                                         LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWPROPERTYPAGE)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWPROPERTYPAGE::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };
    HDC             hDC;
    RECT            rctWorkArea;
    LPWINDOWPOS     lpPos;

    switch(nMsg)
    {
        case WM_NOTIFY:
        {
            if (   ((LPNMLISTVIEW)lParam)->hdr.hwndFrom == _hLstMain &&
                    (((LPNMLISTVIEW)lParam)->hdr.code == LVN_DELETEITEM ||
                    ((LPNMLISTVIEW)lParam)->hdr.code == LVN_INSERTITEM)   )
            {
                ListView_SetColumnWidth(_hLstMain, 0, LVSCW_AUTOSIZE);
                ListView_SetColumnWidth(_hLstMain, 1, LVSCW_AUTOSIZE);
            }
            return FALSE;
        }
        case WM_GETMINMAXINFO:
        {
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = _iMinWidth;
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = _iMinHeight;
            return FALSE;
        }
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_DRAWITEM:
        {
            if (((LPDRAWITEMSTRUCT)lParam)->hwndItem == _hCmdOK)
                this->_DrawButton(((LPDRAWITEMSTRUCT)lParam), L"&OK");
            else
                break;
            return TRUE;
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
        case WM_COMMAND:
        {
            // OK, Cancel, <Esc> - All do the same thing
            if ( ((HWND)lParam == 0x00 && (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK)) ||
                 ((HWND)lParam == _hCmdOK) )
            {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
                return FALSE;
            }
            break;
        }
        case WM_CLOSE:
        {
            ShowWindow(hWnd, SW_HIDE);
            if (_hWndParent)
            {
                PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, 0);
                SetForegroundWindow(_hWndParent);
            }
            return FALSE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWPROPERTYPAGE::_ResetGDIObjects()
{
    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntIcon) { DeleteObject(_hFntIcon); _hFntIcon = NULL; }
    if (_hbrWndBack) { DeleteObject(_hbrWndBack); _hbrWndBack = NULL; }
    if (_hbrBtnBack) { DeleteObject(_hbrBtnBack); _hbrBtnBack = NULL; }
    if (_hPenBtnFocus) { DeleteObject(_hPenBtnFocus); _hPenBtnFocus = NULL; }
    if (_hPenLstBorder) { DeleteObject(_hPenLstBorder); _hPenLstBorder = NULL; }
    if (_hPenWndBorder) { DeleteObject(_hPenWndBorder); _hPenWndBorder = NULL; }
    if (_hPenBtnBorder) { DeleteObject(_hPenBtnBorder); _hPenBtnBorder = NULL; }
    return;
}

void DEWPROPERTYPAGE::_Draw(HDC hDC)
{
    RECT        rctList, rctClient;
    HGDIOBJ     hObjOld = NULL;

    hObjOld = SelectObject(hDC, _hFntUI);

    SelectObject(hDC, _hPenWndBorder);
    SelectObject(hDC, _hbrWndBack);
    GetWindowRect(_hLstMain, &rctList);
    GetClientRect(_hWndProperty, &rctClient);
    Rectangle(hDC, rctClient.left, rctClient.top, rctClient.right, rctClient.bottom);

    SelectObject(hDC, _hPenLstBorder);
    MapWindowPoints(HWND_DESKTOP, _hWndProperty, (LPPOINT)&rctList, 2);
    rctList.left -= _F(1); rctList.top -= _F(1); rctList.right += _F(1); rctList.bottom += _F(1);
    Rectangle(hDC, rctList.left, rctList.top, rctList.right, rctList.bottom);

    SelectObject(hDC, hObjOld);
    return;
}

void DEWPROPERTYPAGE::_DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText)
{
    RECT                rctText, rctIcon;
    HGDIOBJ             hObjOld, hOblOldIcon;
    HDC                 hDCMem;
    HBITMAP             hBmpIcon;

    // Current DC Settings
    SetBkMode(lpDIS->hDC, TRANSPARENT);
    SetTextColor(lpDIS->hDC, _crBtnText);

    // Mem DC (Icon) Settings
    rctIcon.left = 0;
    rctIcon.top = 0;
    rctIcon.right = lpDIS->rcItem.bottom - _F(7);
    rctIcon.bottom = lpDIS->rcItem.bottom - _F(7);
    hDCMem = CreateCompatibleDC(lpDIS->hDC);
    hBmpIcon = CreateCompatibleBitmap(lpDIS->hDC, rctIcon.right, rctIcon.bottom);
    hOblOldIcon = SelectObject(hDCMem, hBmpIcon);
    SelectObject(hDCMem, _hFntIcon);
    FillRect(hDCMem, &rctIcon, _hbrBtnBack);
    SetTextColor(hDCMem, _crBtnText);
    SetBkMode(hDCMem, TRANSPARENT);
    DrawText(hDCMem, DEWUI_SYMBOL_BUTTON_OK, -1, &rctIcon, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    hObjOld = SelectObject(lpDIS->hDC, _hbrBtnBack);
    SelectObject(lpDIS->hDC, _hFntUI);

    SelectObject(lpDIS->hDC, _hPenBtnBorder);
    Rectangle(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);

    if ((lpDIS->itemState & ODS_FOCUS) == ODS_FOCUS)
    {
        CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
        InflateRect(&rctText, -_F(2), -_F(2));
        SelectObject(lpDIS->hDC, _hPenBtnFocus);
        Rectangle(lpDIS->hDC, rctText.left, rctText.top, rctText.right, rctText.bottom);
    }

    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        BitBlt(lpDIS->hDC, _F(4), _F(4), rctIcon.right + _F(1), rctIcon.bottom + _F(1), hDCMem, 0, 0, SRCCOPY);
    else
        BitBlt(lpDIS->hDC, _F(3), _F(3), rctIcon.right, rctIcon.bottom, hDCMem, 0, 0, SRCCOPY);
    SelectObject(hDCMem, hOblOldIcon);
    DeleteObject(hBmpIcon);
    DeleteDC(hDCMem);

    CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
    rctText.left = _F(30);
    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        OffsetRect(&rctText, _F(1), _F(1));
    DrawText(lpDIS->hDC, wsText, -1, &rctText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(lpDIS->hDC, hObjOld);

    return;
}

void DEWPROPERTYPAGE::_SetInfoValue(int iIndex, LPCWSTR wsValue)
{
    LVITEM      lvItem { 0 };

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = (LPWSTR)wsValue;
    lvItem.cchTextMax = lstrlen(wsValue);
    SendMessage(_hLstMain, LVM_SETITEM, 0, (LPARAM)&lvItem);

    ListView_SetColumnWidth(_hLstMain, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(_hLstMain, 1, LVSCW_AUTOSIZE);

    return;
}

void DEWPROPERTYPAGE::_SetUIFont(const wchar_t *wsFontName, int iFontSize)
{
    LOGFONT lgfFont;

    ZeroMemory(&lgfFont, sizeof(LOGFONT));
    lgfFont.lfHeight = -MulDiv(iFontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, wsFontName, 32 * sizeof(wchar_t));

    _hFntUI = CreateFontIndirect(&lgfFont);

    SendMessage(_hLstMain, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hCmdOK, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));

    lgfFont.lfHeight = -MulDiv(DEWUI_DIM_SYM_BUTTON_FONT, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, DEWUI_SYMBOL_FONT_NAME, 32 * sizeof(wchar_t));
    _hFntIcon = CreateFontIndirect(&lgfFont);

    return;
}

void DEWPROPERTYPAGE::_HandleSizing()
{
    RECT        rctClient { 0 };
    int         iX, iY, iW, iH;

    GetClientRect(_hWndProperty, &rctClient);
    iX = _nPadding; iY = _nPadding; iW = rctClient.right - _nPadding * 2; iH = rctClient.bottom - (_nPadding * 2 + _F(40));
    SetWindowPos(_hLstMain, 0, iX, iY, iW, iH, SWP_NOZORDER);

    // iX is nudged ever-so-slightly to the right, so that the client rect
    // of the button aligns with the window rect of the listview control
    iW = _F(75); iH = _F(30); iX = rctClient.right - _nPadding - iW + _F(1); iY = rctClient.bottom - (_nPadding + _F(30));
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);

    ListView_SetColumnWidth(_hLstMain, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(_hLstMain, 1, LVSCW_AUTOSIZE);

    return;
}


DEWPROPERTYPAGE::DEWPROPERTYPAGE(HWND hWndParent, const DEWSETTINGS& settings)
{
    WNDCLASSEX      wcex { 0 };
    wchar_t* const  wsKeys[] = { L"File Name", L"Size", L"Full Path", L"Folder",
                                 L"Channels", L"Duration", L"Sample Rate (Frequency)", L"Bitrate",
                                 L"Title", L"Arist", L"Album", L"Track Number", L"Genre", L"Year",
                                 L"Audio Type", L"Driver Library" };
    const wchar_t*  wsClass = L"DEWDROP.PROPERTY.WND";
    const DWORD     dwWndStyle = WS_OVERLAPPEDWINDOW;
    const DWORD     dwLstStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | WS_HSCROLL | WS_VSCROLL;
    const DWORD     dwBtnStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
    int             iIndex;
    LVCOLUMN        lvc { 0 };
    LVGROUP         lvg { 0 };
    LVITEM          lvItem { 0 };

    _hInstance = GetModuleHandle(NULL);
    _hWndParent = hWndParent;
    _iDPI = settings.Theme.DPI;
    _fScale = settings.Theme.Scale;
    _nPadding = _F(10);

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsClass;

    RegisterClassEx(&wcex);
    _hWndProperty = CreateWindowEx(WS_EX_TOOLWINDOW, wsClass, L"Properties", dwWndStyle,
                                   0, 0, 0, 0, _hWndParent, NULL, _hInstance, NULL);
    _hLstMain = CreateWindowEx(0, WC_LISTVIEW, L"", dwLstStyle,
                               0, 0, 0, 0, _hWndProperty, NULL, _hInstance, NULL);
    _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwBtnStyle,
                             0, 0, 0, 0, _hWndProperty, NULL, _hInstance, NULL);


    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;

    lvc.pszText = (LPWSTR)L"Key";
    lvc.iSubItem = 0;
    ListView_InsertColumn(_hLstMain, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Value";
    lvc.iSubItem = 1;
    ListView_InsertColumn(_hLstMain, 1, &lvc);

    ListView_EnableGroupView(_hLstMain, TRUE);
    lvg.cbSize = sizeof(LVGROUP);
    lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE | LVGF_ALIGN;
    lvg.state = LVGS_COLLAPSIBLE;
    lvg.uAlign = LVGA_HEADER_CENTER;

    lvg.pszHeader = L"File Information";
    lvg.iGroupId = _GRP_FILE_INFO;
    ListView_InsertGroup(_hLstMain, 0, &lvg);

    lvg.pszHeader = L"Audio Information";
    lvg.iGroupId = _GRP_AUDIO_INFO;
    ListView_InsertGroup(_hLstMain, 1, &lvg);

    lvg.pszHeader = L"Other Tag Information";
    lvg.iGroupId = _GRP_TAG_INFO;
    ListView_InsertGroup(_hLstMain, 2, &lvg);

    lvg.pszHeader = L"Codec Information";
    lvg.iGroupId = _GRP_CODEC_INFO;
    ListView_InsertGroup(_hLstMain, 3, &lvg);

    for (iIndex = 0; iIndex <= _PREFIX_INDEX_CI_DRLIB; iIndex++)
    {
        lvItem.mask = LVIF_TEXT | LVIF_GROUPID;
        lvItem.iSubItem = 0;
        lvItem.iItem = iIndex;
        lvItem.pszText = wsKeys[iIndex];
        if (iIndex >= _PREFIX_INDEX_CODEC_GRP)
            lvItem.iGroupId = _GRP_CODEC_INFO;
        else if (iIndex > _PREFIX_INDEX_AI_BITRATE)
            lvItem.iGroupId = _GRP_TAG_INFO;
        else if (iIndex > _PREFIX_INDEX_FI_FILE_DIR)
            lvItem.iGroupId = _GRP_AUDIO_INFO;
        else
            lvItem.iGroupId = _GRP_FILE_INFO;
        SendMessage(_hLstMain, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = iIndex;
        lvItem.iSubItem = 1;
        lvItem.pszText = L"-";
        lvItem.cchTextMax = 1;
        SendMessage(_hLstMain, LVM_SETITEM, 0, (LPARAM)&lvItem);
    }

    this->Reset();
    _iMinWidth = _F(_DEFWIDTH);
    _iMinHeight = _F(_DEFHEIGHT);

    SendMessage(_hLstMain, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    SetWindowSubclass(_hWndProperty, _DewPropPageMsgHandler, (UINT_PTR)_hWndProperty, (DWORD_PTR)this);

    return;
}

HWND DEWPROPERTYPAGE::Show(const DEWTHEME& Theme)
{
    int             iScreenWidth, iScreenHeight, iXPos, iYPos, iWidth, iHeight;
    RECT            rctParent, rctWorkArea, rctThis;
    wchar_t         wsText[DEWUI_MAX_TITLE] { 0 };

    GetWindowRect(_hWndProperty, &rctThis);
    iWidth = rctThis.right - rctThis.left;
    iHeight = rctThis.bottom - rctThis.top;

    if (iWidth < _iMinWidth) iWidth = _iMinWidth;
    if (iHeight < _iMinHeight) iHeight = _iMinHeight;

    this->_ResetGDIObjects();
    this->_SetUIFont(Theme.TextFontStyle.FontName, Theme.TextFontStyle.FontSize);

    _hbrWndBack = CreateSolidBrush(Theme.WinStyle.BackColor);
    _hbrBtnBack = CreateSolidBrush(Theme.MMButtonStyle.BackColor);

    _hPenBtnFocus = CreatePen(PS_DOT, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenBtnBorder = CreatePen(PS_SOLID, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenLstBorder = CreatePen(PS_SOLID, _F(1), Theme.ModuleStyle.OutlineColor);
    _hPenWndBorder = CreatePen(PS_SOLID, _F(1), Theme.WinStyle.OutlineColor);

    _crBtnText = Theme.MMButtonStyle.TextColor;

    ListView_SetBkColor(_hLstMain, Theme.ModuleStyle.BackColor);
    ListView_SetTextBkColor(_hLstMain, Theme.ModuleStyle.BackColor);
    ListView_SetTextColor(_hLstMain, Theme.ModuleStyle.TextColor);
    ListView_SetOutlineColor(_hLstMain, Theme.ModuleStyle.OutlineColor);

    if (_hWndParent == NULL)
    {
        iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        GetWindowRect(_hWndParent, &rctParent);
        iScreenWidth = rctParent.right - rctParent.left;
        iScreenHeight = rctParent.bottom - rctParent.top;
    }

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);

    iXPos = (iScreenWidth - iWidth) / 2 + rctParent.left;
    iYPos = (iScreenHeight - iHeight) / 2 + rctParent.top;

    if (iXPos < rctWorkArea.left) iXPos = rctWorkArea.left;
    if (iYPos < rctWorkArea.top) iYPos = rctWorkArea.top;
    if ((iXPos + iWidth) > rctWorkArea.right) iXPos = rctWorkArea.right - iWidth;
    if ((iYPos + iHeight) > rctWorkArea.bottom) iYPos = rctWorkArea.bottom - iHeight;

    SetWindowPos(_hWndProperty, 0, iXPos, iYPos, iWidth, iHeight, SWP_NOZORDER);

    ShowWindow(_hWndProperty, SW_SHOW);
    UpdateWindow(_hWndProperty);
    SetActiveWindow(_hWndProperty);
    if (_hWndParent)
        EnableWindow(_hWndParent, FALSE);

    return _hWndProperty;
}

void DEWPROPERTYPAGE::Set_FileInfo(LPCWSTR wsFile)
{
    HANDLE          hFind = NULL;
    WIN32_FIND_DATA wfd{ 0 };
    wchar_t         wsText[MAX_CHAR_PATH]{ 0 };
    const DWORD     _1GB = 1024 * 1024 * 1024;
    const DWORD     _1MB = 1024 * 1024;
    const DWORD     _1KB = 1024;

    if (!wsFile || lstrlen(wsFile) <= 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_NAME, L"-");
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_PATH, L"-");
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_DIR, L"-");
        SetWindowText(_hWndProperty, L"Properties");
        return;
    }

    hFind = FindFirstFile(wsFile, &wfd);
    if (hFind == NULL || hFind == INVALID_HANDLE_VALUE)
    {
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_NAME, L"-");
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_PATH, L"-");
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_DIR, L"-");
        SetWindowText(_hWndProperty, L"Properties");
        return;
    }
    FindClose(hFind);
    if (((wfd.dwFileAttributes) % FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_NAME, L"-");
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_PATH, wsFile);
        this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_DIR, wsFile);
        SetWindowText(_hWndProperty, L"Properties");
        return;
    }

    this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_NAME, wfd.cFileName);
    
    ZeroMemory(wsText, MAX_CHAR_PATH * sizeof(wchar_t));
    if (wfd.nFileSizeLow >= _1GB)
        StringCchPrintf(wsText, DEWUI_MAX_TITLE, L"%.02f GB", ((float)wfd.nFileSizeLow / (float)_1GB));
    else if (wfd.nFileSizeLow >= _1MB)
        StringCchPrintf(wsText, DEWUI_MAX_TITLE, L"%.02f MB", ((float)wfd.nFileSizeLow / (float)_1MB));
    else if (wfd.nFileSizeLow >= _1KB)
        StringCchPrintf(wsText, DEWUI_MAX_TITLE, L"%.02f KB", ((float)wfd.nFileSizeLow / (float)_1KB));
    this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_SIZE, wsText);

    this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_PATH, wsFile);

    StringCchPrintf(wsText, MAX_CHAR_PATH, wsFile);
    PathCchRemoveFileSpec(wsText, lstrlen(wsText));
    this->_SetInfoValue(_PREFIX_INDEX_FI_FILE_DIR, wsText);

    StringCchPrintf(wsText, DEWUI_MAX_TITLE, L"'%s' Properties", wfd.cFileName);
    SetWindowText(_hWndProperty, wsText);

    return;

}

void DEWPROPERTYPAGE::Set_AudioInfo_Channels(BYTE btChannels)
{
    wchar_t     wsData[32];

    if (btChannels <= 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_AI_CHANNELS, L"-");
        return;
    }
    StringCchPrintf(wsData, 32, L"%u", btChannels);
    this->_SetInfoValue(_PREFIX_INDEX_AI_CHANNELS, wsData);
    return;
}

void DEWPROPERTYPAGE::Set_AudioInfo_Length(DWORD dwDuration)
{
    wchar_t     wsData[32];

    if (dwDuration <= 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_AI_DURATION, L"-");
        return;
    }
    StringCchPrintf(wsData, 32, L"%02u:%02u", (dwDuration/1000)/60, (dwDuration/1000)%60);
    this->_SetInfoValue(_PREFIX_INDEX_AI_DURATION, wsData);
    return;
}

void DEWPROPERTYPAGE::Set_AudioInfo_Frequency(DWORD dwFrequency)
{
    wchar_t     wsData[32];

    if (dwFrequency <= 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_AI_SAMPLE_RATE, L"-");
        return;
    }
    StringCchPrintf(wsData, 32, L"%u Hz", dwFrequency);
    this->_SetInfoValue(_PREFIX_INDEX_AI_SAMPLE_RATE, wsData);
    return;
}

void DEWPROPERTYPAGE::Set_AudioInfo_Bitrate(DWORD dwBitrate)
{
    wchar_t     wsData[32];

    if (dwBitrate <= 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_AI_BITRATE, L"-");
        return;
    }
    StringCchPrintf(wsData, 32, L"%u Kbps", dwBitrate/1000);
    this->_SetInfoValue(_PREFIX_INDEX_AI_BITRATE, wsData);
    return;
}

void DEWPROPERTYPAGE::Set_AudioInfo(BYTE btChannels, DWORD dwDuration, DWORD dwFrequency, DWORD dwBitrate)
{
    this->Set_AudioInfo_Channels(btChannels);
    this->Set_AudioInfo_Length(dwDuration);
    this->Set_AudioInfo_Frequency(dwFrequency);
    this->Set_AudioInfo_Bitrate(dwBitrate);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_Title(LPCWSTR wsTitle, BOOL bSimulatedTitle)
{
    // Put 'bSimulatedTitle' as the first since for missing titles,
    // this will be the quickest flag to check and the remaining checks can eb discarded
    if (bSimulatedTitle || !wsTitle || lstrlen(wsTitle) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_TITLE, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_TITLE, wsTitle);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_Artist(LPCWSTR wsArtist)
{
    if (!wsArtist || lstrlen(wsArtist) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_ARTIST, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_ARTIST, wsArtist);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_Album(LPCWSTR wsAlbum)
{
    if (!wsAlbum || lstrlen(wsAlbum) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_ALBUM, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_ALBUM, wsAlbum);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_TrackNo(LPCWSTR wsTrackNo)
{
    if (!wsTrackNo || lstrlen(wsTrackNo) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_TRACK, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_TRACK, wsTrackNo);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_Genre(LPCWSTR wsGenre)
{
    if (!wsGenre || lstrlen(wsGenre) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_GENRE, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_GENRE, wsGenre);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo_Year(LPCWSTR wsYear)
{
    if (!wsYear || lstrlen(wsYear) == 0)
    {
        this->_SetInfoValue(_PREFIX_INDEX_TI_YEAR, L"-");
        return;
    }
    this->_SetInfoValue(_PREFIX_INDEX_TI_YEAR, wsYear);
    return;
}

void DEWPROPERTYPAGE::Set_TagInfo(LPCWSTR wsTitle, BOOL bSimulatedTitle, LPCWSTR wsArtist,
                                  LPCWSTR wsAlbum, LPCWSTR wsTrackNo, LPCWSTR wsGenre, LPCWSTR wsYear)
{
    this->Set_TagInfo_Title(wsTitle, bSimulatedTitle);
    this->Set_TagInfo_Artist(wsArtist);
    this->Set_TagInfo_Album(wsAlbum);
    this->Set_TagInfo_TrackNo(wsTrackNo);
    this->Set_TagInfo_Genre(wsGenre);
    this->Set_TagInfo_Year(wsYear);
    return;
}

void DEWPROPERTYPAGE::Set_CodecAudioType(LPCWSTR wsAudioType)
{
    if (!wsAudioType || lstrlen(wsAudioType) <= 0)
        this->_SetInfoValue(_PREFIX_INDEX_CI_AUDIO_TYPE, L"-");
    else
        this->_SetInfoValue(_PREFIX_INDEX_CI_AUDIO_TYPE, wsAudioType);
    return;
}

void DEWPROPERTYPAGE::Set_CodecLibrary(LPCWSTR wsLibrary)
{
    if (!wsLibrary || lstrlen(wsLibrary) <= 0)
        this->_SetInfoValue(_PREFIX_INDEX_CI_DRLIB, L"-");
    else
        this->_SetInfoValue(_PREFIX_INDEX_CI_DRLIB, wsLibrary);
    return;
}

void DEWPROPERTYPAGE::Clear_CodecInfo()
{
    int         iIndex;

    do
    {
        iIndex = ListView_GetNextItem(_hLstMain, _PREFIX_INDEX_CI_DRLIB, LVNI_ALL);
        if (iIndex != -1)
            ListView_DeleteItem(_hLstMain, iIndex);
    } while (iIndex != -1);

    return;
}

void DEWPROPERTYPAGE::Add_CodecInfo(LPCWSTR wsKey, LPCWSTR wsValue)
{
    LVITEM      lvItem { 0 };
    
    lvItem.mask = LVIF_TEXT | LVIF_GROUPID;
    lvItem.iItem = ListView_GetItemCount(_hLstMain);
    lvItem.iSubItem = 0;
    lvItem.pszText = (LPWSTR)wsKey;
    lvItem.cchTextMax = lstrlen(wsKey);
    lvItem.iGroupId = _GRP_CODEC_INFO;
    SendMessage(_hLstMain, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 1;
    lvItem.pszText = (LPWSTR)wsValue;
    lvItem.cchTextMax = lstrlen(wsValue);
    SendMessage(_hLstMain, LVM_SETITEM, 0, (LPARAM)&lvItem);

    return;
}

void DEWPROPERTYPAGE::Reset()
{
    for (int iIndex = 0; iIndex < ListView_GetItemCount(_hLstMain); iIndex++)
        this->_SetInfoValue(iIndex, L"-");
    SetWindowText(_hWndProperty, L"Properties");
    return;
}

DEWPROPERTYPAGE::~DEWPROPERTYPAGE()
{
    DestroyWindow(_hLstMain);
    DestroyWindow(_hWndProperty);
    this->_ResetGDIObjects();
    return;
}

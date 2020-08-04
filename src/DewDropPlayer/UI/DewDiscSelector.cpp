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

#include "DewDiscSelector.h"

UINT DEWDISCSELECTOR::_snInstanceCount = 0;

LRESULT CALLBACK DEWDISCSELECTOR::_DSMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWDISCSELECTOR)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
    return 0;
}

void DEWDISCSELECTOR::_HandleSizing()
{
    RECT                rctClient;
    int                 iX, iY, iW, iH;
    LVTILEVIEWINFO      lvTVI { 0 };

    GetClientRect(_hWndDiscSelector, &rctClient);

    iX = _F(10); iY = _F(10); iW = rctClient.right - _F(20); iH = rctClient.bottom - _F(60);
    SetWindowPos(_hLstDrives, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iW = _F(75); iH = _F(30); iY = rctClient.bottom - iH - _F(10);
    iX = rctClient.right - iW - _F(10);
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(10);
    SetWindowPos(_hCmdCancel, 0, iX, iY, iW, iH, SWP_NOZORDER);

    lvTVI.cbSize = sizeof(LVTILEVIEWINFO);
    lvTVI.dwFlags = LVTVIF_FIXEDSIZE;
    lvTVI.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
    lvTVI.cLines = 3;
    lvTVI.sizeTile.cx = (rctClient.right - _F(20)) - GetSystemMetrics(SM_CXVSCROLL);
    lvTVI.sizeTile.cy = _F(DEWUI_DIM_SYM_DISC_ICON);
    ListView_SetTileViewInfo(_hLstDrives, &lvTVI);

    return;
}

void DEWDISCSELECTOR::_EnumDrives()
{
    typedef struct _DEWDRIVELIST
    {
        wchar_t                 wsPathName[MAX_PATH];
        wchar_t                 wsVolumeLabel[MAX_PATH];
        wchar_t                 wsFSName[MAX_PATH];
        wchar_t                 wsDriveType[32];
        struct _DEWDRIVELIST    *next;
    } DEWDRIVELIST, *LPDEWDRIVELIST;

    HANDLE          hVol = NULL;
    DWORD           dwSize;
    DWORD           dwSerialNo, dwMaxComp, dwFSFlags;
    BOOL            bInserted;
    wchar_t         wsVolumeName[MAX_PATH];
    wchar_t         wsPathName[MAX_PATH];

    LPDEWDRIVELIST  lpStart = NULL, lpCurr, lpPrev = NULL, lpNext = NULL;

    hVol = FindFirstVolume(wsVolumeName, MAX_PATH);
    if (hVol != NULL && hVol != INVALID_HANDLE_VALUE)
    {
        do
        {
            dwSize = MAX_PATH;
            dwSerialNo = dwMaxComp = dwFSFlags = 0;
            GetVolumePathNamesForVolumeName(wsVolumeName, wsPathName, dwSize, &dwSize);
            if (lstrlen(wsPathName))
            {
                lpCurr = (LPDEWDRIVELIST)LocalAlloc(LPTR, sizeof(DEWDRIVELIST));
                lpCurr->next = NULL;

                CopyMemory((lpCurr->wsPathName), wsPathName, MAX_PATH * sizeof(wchar_t));
                GetVolumeInformation(lpCurr->wsPathName, (lpCurr->wsVolumeLabel), MAX_PATH, &dwSerialNo, &dwMaxComp, &dwFSFlags, (lpCurr->wsFSName), MAX_PATH);
                switch(GetDriveType(lpCurr->wsPathName))
                {
                    case DRIVE_NO_ROOT_DIR: { StringCchPrintf((lpCurr->wsDriveType), 32, L"Invalid Root Path\n"); break; }
                    case DRIVE_REMOVABLE: { StringCchPrintf((lpCurr->wsDriveType), 32, L"Removable Media\n"); break; }
                    case DRIVE_FIXED: { StringCchPrintf((lpCurr->wsDriveType), 32, L"Fixed Drive\n"); break; }
                    case DRIVE_REMOTE: { StringCchPrintf((lpCurr->wsDriveType), 32, L"Remote Drive\n"); break; }
                    case DRIVE_CDROM: { StringCchPrintf((lpCurr->wsDriveType), 32, L"CD ROM Drive\n"); break; }
                    case DRIVE_RAMDISK: { StringCchPrintf((lpCurr->wsDriveType), 32, L"RAM Disk\n"); break; }
                    default: StringCchPrintf((lpCurr->wsDriveType), 32, L"Unknown\n");
                }

                if (lpStart == NULL) // No entry. Empty list
                {
                    lpStart = lpCurr;
                }
                else if (lstrcmpi((lpStart->wsPathName), (lpCurr->wsPathName)) > 0) // Entry needs to be made at the very beginning
                {
                    lpCurr->next = lpStart;
                    lpStart = lpCurr;
                }
                else if (lpStart->next == NULL) // Just one entry is there. A new entry is to be made at the end
                {
                    lpStart->next = lpCurr;
                }
                else
                {
                    bInserted = FALSE;

                    lpPrev = lpStart;
                    do
                    {
                        lpNext = lpPrev->next;

                        if (lpNext != NULL &&
                            lstrcmpi((lpNext->wsPathName), (lpCurr->wsPathName)) > 0 &&
                            lstrcmpi((lpPrev->wsPathName), (lpCurr->wsPathName)) <= 0)
                        {
                            lpCurr->next = lpNext;
                            lpPrev->next = lpCurr;
                            bInserted = TRUE;
                            break;
                        }
                        lpPrev = lpPrev->next;
                    } while (lpPrev->next);
                    if (!bInserted)
                        lpPrev->next = lpCurr;
                }
            }
        } while (FindNextVolume(hVol, wsVolumeName, MAX_PATH));
        FindVolumeClose(hVol);

        lpCurr = lpStart;
        while (lpCurr)
        {
            lpPrev = lpCurr;
            lpCurr = lpPrev->next;

            // Filter to check for CDFS and only insert those into the listview
            if (GetDriveType(lpPrev->wsPathName) == DRIVE_CDROM)
                this->_AddDiscInfo(lpPrev->wsPathName, lpPrev->wsVolumeLabel, lpPrev->wsDriveType, lpPrev->wsFSName, (lpPrev == lpStart));

            LocalFree(lpPrev);
        }
    }
    return;
}

void DEWDISCSELECTOR::_AddDiscInfo(LPWSTR wsPath, LPWSTR wsLabel, LPWSTR wsDriveType, LPWSTR wsFSType, BOOL bFlushAll)
{
    LVITEM      lvItem { 0 };
    int         iIndex;
    UINT        nOrder[] = { 1, 2 };
    int         iColFmt[] = { LVCFMT_FILL, LVCFMT_FILL, LVCFMT_FILL };
    wchar_t     wsTitle[MAX_PATH * 2];

    if (bFlushAll)
        ListView_DeleteAllItems(_hLstDrives);

    iIndex = ListView_GetItemCount(_hLstDrives);
    StringCchPrintf(wsTitle, MAX_PATH * 2, L"%s (%s)", wsLabel, wsPath);

    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 0;
    lvItem.pszText = wsTitle;
    lvItem.iImage = 0;
    lvItem.cColumns = 2;
    lvItem.puColumns = nOrder;
    ListView_InsertItem(_hLstDrives, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = wsDriveType;
    lvItem.iImage = 0;
    ListView_SetItem(_hLstDrives, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 2;
    lvItem.pszText = wsFSType;
    lvItem.iImage = 0;
    ListView_SetItem(_hLstDrives, &lvItem);

    return;

}

void DEWDISCSELECTOR::_DestroyGDIObjects()
{
    int         iIndex, iCount;
    IMAGEINFO   imgInfo { 0 };

    if (_lpSymbolFont) { delete _lpSymbolFont; _lpSymbolFont = NULL; }
    if (_hbrWndBack) { DeleteObject(_hbrWndBack); _hbrWndBack = NULL; }
    if (_hbrBtnBack) { DeleteObject(_hbrBtnBack); _hbrBtnBack = NULL; }
    if (_hPenBtnFocus) { DeleteObject(_hPenBtnFocus); _hPenBtnFocus = NULL; }
    if (_hPenBtnBorder) { DeleteObject(_hPenBtnBorder); _hPenBtnBorder = NULL; }
    if (_hPenLstBorder) { DeleteObject(_hPenLstBorder); _hPenLstBorder = NULL; }
    if (_hPenWndBorder) { DeleteObject(_hPenWndBorder); _hPenWndBorder = NULL; }
    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntIcon) { DeleteObject(_hFntIcon); _hFntIcon = NULL; }
    if (_pFntSymbol) { delete _pFntSymbol; _pFntSymbol = NULL; }
    if (_pBrText) { delete _pBrText; _pBrText = NULL; }

    iCount = ImageList_GetImageCount(_hImlDriveTypes);
    for (iIndex = 0; iIndex < iCount; iIndex++)
    {
        ImageList_GetImageInfo(_hImlDriveTypes, iIndex, &imgInfo);
        if (imgInfo.hbmImage) { DeleteObject(imgInfo.hbmImage); imgInfo.hbmImage = NULL; }
        if (imgInfo.hbmMask) { DeleteObject(imgInfo.hbmMask); imgInfo.hbmMask = NULL; }
    }
    ImageList_RemoveAll(_hImlDriveTypes);
    return;
}

void DEWDISCSELECTOR::_ApplyTheme(const DEWTHEME& Theme)
{
    LOGFONT     lgfFont { 0 };
    HWND        hWndChild = NULL;
    Color       crRef;

    this->_DestroyGDIObjects();

    _lpSymbolFont = new DEWSYMBOLFONT();

    _hbrWndBack = CreateSolidBrush(Theme.WinStyle.BackColor);
    _hbrBtnBack = CreateSolidBrush(Theme.MMButtonStyle.BackColor);

    _hPenBtnFocus = CreatePen(PS_DOT, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenBtnBorder = CreatePen(PS_SOLID, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenLstBorder = CreatePen(PS_SOLID, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenWndBorder = CreatePen(PS_SOLID, _F(1), Theme.WinStyle.OutlineColor);

    _crBtnText = Theme.MMButtonStyle.TextColor;

    crRef.SetFromCOLORREF(Theme.ModuleStyle.TextColor);
    _pBrText = new SolidBrush(crRef);

    ListView_SetBkColor(_hLstDrives, Theme.ModuleStyle.BackColor);
    ListView_SetTextBkColor(_hLstDrives, Theme.ModuleStyle.BackColor);
    ListView_SetTextColor(_hLstDrives, Theme.ModuleStyle.TextColor);
    ListView_SetOutlineColor(_hLstDrives, Theme.ModuleStyle.OutlineColor);

    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    _hFntUI = CreateFontIndirect(&lgfFont);

    lgfFont.lfHeight = -MulDiv(DEWUI_DIM_SYM_BUTTON_FONT, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, DEWUI_SYMBOL_FONT_NAME, 32 * sizeof(wchar_t));
    _hFntIcon = CreateFontIndirect(&lgfFont);

    _pFntSymbol = new Font(_lpSymbolFont->SymbolFontGDIPlus, (REAL)_F(DEWUI_DIM_SYM_DISC_FONT), FontStyleRegular, UnitPoint);
    this->_AddImageListImage(DEWUI_SYMBOL_OPEN_DISC);

    hWndChild = FindWindowEx(_hWndDiscSelector, hWndChild, NULL, NULL);
    do
    {
        if (hWndChild)
            SendMessage(hWndChild, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
        hWndChild = FindWindowEx(_hWndDiscSelector, hWndChild, NULL, NULL);
    } while (hWndChild);

    return;
}

void DEWDISCSELECTOR::_AddImageListImage(LPCWSTR wsSymbol)
{
    Bitmap      *pImg = NULL;
    Color       crRef;
    Graphics    *pGr = NULL;
    int         BTN_ICON_SIZE;
    PointF      ptStart (0.0f, 0.0f);
    RectF       rctPos;
    HBITMAP     hBmp;

    BTN_ICON_SIZE = _F(DEWUI_DIM_SYM_DISC_ICON);
    pImg = new Bitmap(BTN_ICON_SIZE, BTN_ICON_SIZE, PixelFormat32bppARGB);
    pGr = Graphics::FromImage(pImg);
    pGr->SetTextRenderingHint(TextRenderingHintAntiAlias);
    pGr->MeasureString(wsSymbol, -1, _pFntSymbol, ptStart, &rctPos);
    ptStart.X = ((REAL)BTN_ICON_SIZE - rctPos.Width)/2.0f;
    ptStart.Y = ((REAL)BTN_ICON_SIZE - rctPos.Height)/2.0f;
    pGr->DrawString(wsSymbol, -1, _pFntSymbol, ptStart, _pBrText);
    pImg->GetHBITMAP(Color::Transparent, &hBmp);

    ImageList_Add(_hImlDriveTypes, hBmp, (HBITMAP)NULL);

    DeleteObject(hBmp);
    delete pImg;
    delete pGr;

    return;
}

void DEWDISCSELECTOR::_SetCurrentSelection()
{
    int         iIndex;
    LVITEM      lvSel;
    wchar_t     wsSel[MAX_PATH * 2] { 0 };

    iIndex = ListView_GetNextItem(_hLstDrives, -1, LVNI_SELECTED);
    if (iIndex == -1) return;

    lvSel.mask = LVIF_TEXT;
    lvSel.iItem = iIndex;
    lvSel.iSubItem = 0;
    lvSel.pszText = wsSel;
    lvSel.cchTextMax = MAX_PATH * 2;
    ListView_GetItem(_hLstDrives, &lvSel);
    CopyMemory(_wsSelPath, (StrStrI(wsSel, L":") - 1), 2 * sizeof(wchar_t));

    SendMessage(_hWndDiscSelector, WM_CLOSE, 0, 0);
    PostMessage(_hWndParent, WM_DEWMSG_DISC_SEL, 0, (LPARAM)_wsSelPath);
    return;
}

void DEWDISCSELECTOR::_Draw()
{
    RECT        rctClient;
    RECT        rctList;
    HGDIOBJ     hObjOld;

    GetClientRect(_hWndDiscSelector, &rctClient);
    GetWindowRect(_hLstDrives, &rctList);
    MapWindowPoints(HWND_DESKTOP, _hWndDiscSelector, (LPPOINT)&rctList, 2);

    hObjOld = SelectObject(_hDC, _hbrWndBack);
    SelectObject(_hDC, _hPenWndBorder);
    Rectangle(_hDC, rctClient.left, rctClient.top, rctClient.right, rctClient.bottom);
    SelectObject(_hDC, _hPenLstBorder);
    rctList.left -= _F(1); rctList.top -= _F(1); rctList.right += _F(1); rctList.bottom += _F(1);
    Rectangle(_hDC, rctList.left - 1, rctList.top, rctList.right, rctList.bottom);

    SelectObject(_hDC, hObjOld);
    return;
}

void DEWDISCSELECTOR::_DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText, LPCWSTR wsSymbol)
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
    DrawText(hDCMem, wsSymbol, -1, &rctIcon, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

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

LRESULT DEWDISCSELECTOR::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };
    LPNMHDR         lpNMH;
    LPWINDOWPOS     lpPos;
    RECT            rctWorkArea;

    switch(nMsg)
    {
        case WM_COMMAND:
        {
            if (    (HWND)lParam == _hCmdCancel ||
                    ((HWND)lParam == 0x00 && LOWORD(wParam) == IDCANCEL)    )
                SendMessage(_hWndDiscSelector, WM_CLOSE, 0, 0);
            else if (   (HWND)lParam == _hCmdOK ||
                        ((HWND)lParam == 0x00 && LOWORD(wParam) == IDOK)    )
                this->_SetCurrentSelection();
            break;
        }
        case WM_NOTIFY:
        {
            lpNMH = (LPNMHDR)lParam;
            if (lpNMH->hwndFrom == _hLstDrives)
            {
                if (lpNMH->code == NM_CLICK || lpNMH->code == LVN_ITEMCHANGED)
                {
                    ShowWindow(_hCmdOK, ((ListView_GetNextItem(_hLstDrives, -1, LVNI_SELECTED) != -1) ? SW_SHOW : SW_HIDE));
                }
                else if (lpNMH->code == NM_DBLCLK)
                {
                    this->_SetCurrentSelection();
                }
            }
            return FALSE;
        }
        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            this->_Draw();
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_DRAWITEM:
        {
            if (((LPDRAWITEMSTRUCT)lParam)->hwndItem == _hCmdOK)
                this->_DrawButton(((LPDRAWITEMSTRUCT)lParam), L"&OK", DEWUI_SYMBOL_BUTTON_OK);
            else if (((LPDRAWITEMSTRUCT)lParam)->hwndItem == _hCmdCancel)
                this->_DrawButton(((LPDRAWITEMSTRUCT)lParam), L"&Cancel", DEWUI_SYMBOL_BUTTON_CANCEL);
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
        case WM_CLOSE:
        {
            ShowWindow(_hWndDiscSelector, SW_HIDE);
            PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWndDiscSelector);
            SetForegroundWindow(_hWndParent);
            return TRUE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

DEWDISCSELECTOR::DEWDISCSELECTOR(HWND hWndParent, int iDPI, float fScale) : SelectedPath(_wsSelPath)
{
    if ((DEWDISCSELECTOR::_snInstanceCount) == 0)
    {
        WNDCLASSEX      wcex { 0 };
        const wchar_t   *wsClass = L"DEWDROP.DISCSEL.WND";
        const DWORD     dwWinStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
        const DWORD     dwBtnStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
        const DWORD     dwLstStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP;
        LVCOLUMN        lvc { 0 };

        _hWndParent = hWndParent;
        _hInstance = GetModuleHandle(NULL);
        _iDPI = iDPI;
        _fScale = fScale;

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
        _hWndDiscSelector = CreateWindowEx(WS_EX_TOOLWINDOW, wsClass, L"Select an Audio CD Disc", dwWinStyle,
                                           0, 0, 0, 0, _hWndParent, NULL, _hInstance, NULL);
        _hLstDrives = CreateWindowEx(0, WC_LISTVIEW, L"", dwLstStyle,
                                     0, 0, 0, 0, _hWndDiscSelector, NULL, _hInstance, NULL);
        _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwBtnStyle,
                                 0, 0, 0, 0, _hWndDiscSelector, NULL, _hInstance, NULL);
        _hCmdCancel = CreateWindowEx(0, L"BUTTON", L"&Cancel", dwBtnStyle,
                                     0, 0, 0, 0, _hWndDiscSelector, NULL, _hInstance, NULL);

        _hImlDriveTypes = ImageList_Create(_F(DEWUI_DIM_SYM_DISC_ICON), _F(DEWUI_DIM_SYM_DISC_ICON), ILC_COLOR32 | ILC_MASK, 1, 1);

        _hDC = GetDC(_hWndDiscSelector);

        ListView_SetExtendedListViewStyleEx(_hLstDrives,
                                             LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT,
                                             LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
        ListView_SetImageList(_hLstDrives, _hImlDriveTypes, LVSIL_NORMAL);

        lvc.mask = LVCF_FMT | LVCF_TEXT;
        lvc.fmt = LVCFMT_LEFT;

        lvc.iImage = 0;
        lvc.pszText = L"Label/Path";
        lvc.iSubItem = 0;
        ListView_InsertColumn(_hLstDrives, 0, &lvc);

        lvc.pszText = L"Drive Type";
        lvc.iSubItem = 1;
        ListView_InsertColumn(_hLstDrives, 1, &lvc);

        lvc.pszText = L"File System";
        lvc.iSubItem = 2;
        ListView_InsertColumn(_hLstDrives, 2, &lvc);

        ListView_SetView(_hLstDrives, LV_VIEW_TILE);

        SetWindowSubclass(_hWndDiscSelector, _DSMsgHandler, (UINT_PTR)_hWndDiscSelector, (DWORD_PTR)this);
    }
    (DEWDISCSELECTOR::_snInstanceCount)++;

    return;
}

HWND DEWDISCSELECTOR::Show(const DEWTHEME& Theme)
{
    int         iWidth, iHeight, iXPos, iYPos;
    int         iParentWidth, iParentHeight;
    RECT        rctParent, rctWorkArea;

    GetWindowRect(_hWndParent, &rctParent);
    iWidth = _F(320);
    iHeight = _F(400);
    iParentWidth = rctParent.right - rctParent.left;
    iParentHeight = rctParent.bottom - rctParent.top;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);

    iXPos = (iParentWidth - iWidth)/2 + rctParent.left;
    iYPos = (iParentHeight - iHeight)/2 + rctParent.top;

    if (iXPos < rctWorkArea.left) iXPos = rctWorkArea.left;
    if (iYPos < rctWorkArea.top) iYPos = rctWorkArea.top;
    if ((iXPos + iWidth) > rctWorkArea.right) iXPos = rctWorkArea.right - iWidth;
    if ((iYPos + iHeight) > rctWorkArea.bottom) iYPos = rctWorkArea.bottom - iHeight;

    ZeroMemory(_wsSelPath, 4 * sizeof(wchar_t)); // Flush out the current entry while opening the window

    this->_ApplyTheme(Theme);
    this->_EnumDrives();
    SetWindowPos(_hWndDiscSelector, 0, iXPos, iYPos, iWidth, iHeight, SWP_NOZORDER);
    ShowWindow(_hCmdOK, SW_HIDE);

    ShowWindow(_hWndDiscSelector, SW_SHOW);
    UpdateWindow(_hWndDiscSelector);
    SetActiveWindow(_hWndDiscSelector);
    if (_hWndParent)
        EnableWindow(_hWndParent, FALSE);

    return _hWndDiscSelector;
}

DEWDISCSELECTOR::~DEWDISCSELECTOR()
{
    this->_DestroyGDIObjects();
    RemoveWindowSubclass(_hWndDiscSelector, _DSMsgHandler, (UINT_PTR)_hWndDiscSelector);
    ReleaseDC(_hWndDiscSelector, _hDC);
    DestroyWindow(_hLstDrives);
    DestroyWindow(_hCmdCancel);
    DestroyWindow(_hCmdOK);
    DestroyWindow(_hWndDiscSelector);
}

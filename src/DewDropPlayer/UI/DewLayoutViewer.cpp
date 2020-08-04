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

#include "DewLayoutViewer.h"

LRESULT CALLBACK DEWLAYOUTVIEWER::_LstLayoutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    LPDEWLAYOUTVIEWER   lpViewer;

    lpViewer = (LPDEWLAYOUTVIEWER)dwRefData;
    if ((HWND)nID == lpViewer->_hLstLayout)
        return lpViewer->_LstLayoutWndProc(hWnd, nMsg, wParam, lParam);
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWLAYOUTVIEWER::_Scale(BYTE btIconScale)
{
    if (_btObjectType != DEWUI_OBJ_TYPE_MODULE ||
        _btObjectID != DEWUI_OBJ_LAYOUT_VIEWER)
        return;
    if (!(btIconScale == DEWTHEME_ICON_SCALE_SMALL ||
          btIconScale == DEWTHEME_ICON_SCALE_MEDIUM ||
          btIconScale == DEWTHEME_ICON_SCALE_LARGE))
        return;

    _iWidth = _F(DEWUI_DIM_WINDOW_X);
    _iHeight =_F((DEWUI_DIM_WINDOW_Y - DEWUI_DIM_WINDOW_Y_NOPL));
    _fDimension = 0.0f;
    return;
}

void DEWLAYOUTVIEWER::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;
    RECT        rctLst;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;

    pGr = Graphics::FromHDC(hDCCanvas);
    pGr->FillRectangle(_pBrBack, _F(0), _F(0), _iWidth, _iHeight);
    pGr->DrawLine(_pPenWinOutline, _iWidth - _F(1), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    pGr->DrawLine(_pPenWinOutline, _iWidth - _F(1), _iHeight - _F(1), _F(0), _iHeight - _F(1));
    pGr->DrawLine(_pPenWinOutline, _F(0), _iHeight - _F(1), _F(0), _F(0));
    GetWindowRect(_hLstLayout, &rctLst);
    MapWindowPoints(HWND_DESKTOP, _hWnd, (LPPOINT)&rctLst, 2);
    pGr->DrawRectangle(_pPenOutline, _F(DEWUI_DIM_PADDING_X) - _F(1), _F(DEWUI_DIM_PADDING_X) - _F(1),
                       (INT)(rctLst.right - rctLst.left) + _F(1), (INT)(rctLst.bottom - rctLst.top) + _F(1));

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWLAYOUTVIEWER::_PrepareImages() {} // NOP

LRESULT DEWLAYOUTVIEWER::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    int             iIndex;
    HDC             hDC = NULL;
    PAINTSTRUCT     ps { 0 };

    switch (nMsg)
    {
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
            return FALSE;
        }

        case WM_NOTIFY:
        {
            if (((LPNMHDR)lParam)->hwndFrom == _hLstLayout)
            {
                switch (((LPNMHDR)lParam)->code)
                {
                    case NM_CLICK:
                    {
                        if (ListView_GetSelectedCount(_hLstLayout))
                        {
                            iIndex = ListView_GetNextItem(_hLstLayout, -1, LVNI_SELECTED);
                            if (_lpComponentStack)
                                SetFocus(_lpComponentStack[iIndex]._lpObject->Handle);
                            SendMessage(_Parent->Handle, WM_DEWMSG_LAYOUT_OBJ_ACTIVATED, (WPARAM)(iIndex + 1), 0);
                        }
                        else
                        {
                            SendMessage(_Parent->Handle, WM_DEWMSG_LAYOUT_OBJ_ACTIVATED, DEWUI_OBJ_NONE, 0);
                        }
                        return FALSE;
                    }
                    break;
                }
            }
        }
        return FALSE;
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWLAYOUTVIEWER::_LstLayoutWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWLAYOUTVIEWER::_MoveToDefaultLocation() {} // NOP. Layout viewer will always remain at the bottom

DEWLAYOUTVIEWER::DEWLAYOUTVIEWER(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
    const wchar_t   *wsLayoutViewerClass = L"DEWDROP.LAYOUT.VIEWER.WND";
    WNDCLASSEX      wcex { 0 };
    LVCOLUMN        lvc { 0 };
    int             iSpan = 0;

    _btObjectID = DEWUI_OBJ_LAYOUT_VIEWER;
    _btObjectType = DEWUI_OBJ_TYPE_MODULE;
    _Parent = Parent;
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;
    ZeroMemory(_wsText, DEWUI_MAX_TITLE * sizeof(wchar_t));

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
    wcex.lpszClassName  = wsLayoutViewerClass;

    RegisterClassEx(&wcex);

    this->_Scale(Theme.IconScale);
    _ptPos.x = _F(0);
    _ptPos.y = _F(DEWUI_DIM_WINDOW_Y_NOPL);
    this->ApplyTheme(Theme, FALSE, FALSE);

    _hWnd = CreateWindowEx(0, wsLayoutViewerClass, _wsText, _LVSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hLstLayout = CreateWindowEx(0, WC_LISTVIEW, L"", _LSTSTYLE,
                                _F(DEWUI_DIM_PADDING_X), _F(DEWUI_DIM_PADDING_X),
                                _iWidth - _F(DEWUI_DIM_PADDING_X) * 2,
                                _iHeight - _F(DEWUI_DIM_PADDING_X) * 2,
                                _hWnd, NULL, _hInstance, NULL);

    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;

    iSpan = _iWidth - _F(DEWUI_DIM_PADDING_X) * 2 - _F((GetSystemMetrics(SM_CXVSCROLL)));

    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = (LPWSTR)L"Component Name";
    lvc.cx = (int)((float)iSpan * 0.6f);
    lvc.iSubItem = 0;
    ListView_InsertColumn(_hLstLayout, 0, &lvc);

    lvc.fmt = LVCFMT_CENTER;
    lvc.pszText = (LPWSTR)L"Dimension";
    lvc.cx = (int)((float)iSpan * 0.2f);
    lvc.iSubItem = 1;
    ListView_InsertColumn(_hLstLayout, 1, &lvc);

    lvc.fmt = LVCFMT_CENTER;
    lvc.pszText = (LPWSTR)L"X";
    lvc.cx = (int)((float)iSpan * 0.1f);
    lvc.iSubItem = 2;
    ListView_InsertColumn(_hLstLayout, 2, &lvc);

    lvc.fmt = LVCFMT_CENTER;
    lvc.fmt = LVCFMT_RIGHT;
    lvc.pszText = (LPWSTR)L"Y";
    lvc.cx = (int)((float)iSpan * 0.1f);
    lvc.iSubItem = 3;
    ListView_InsertColumn(_hLstLayout, 3, &lvc);

    SendMessage(_hLstLayout, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    _hDC = GetDC(_hWnd);
    this->_InitiateSubclassing();

    if (_hLstLayout && _hLstLayout != INVALID_HANDLE_VALUE)
    {
        ListView_SetBkColor(_hLstLayout, _crBack.ToCOLORREF());
        ListView_SetTextBkColor(_hLstLayout, _crBack.ToCOLORREF());
        ListView_SetTextColor(_hLstLayout, _crText.ToCOLORREF());
        ListView_SetOutlineColor(_hLstLayout, _crOutline.ToCOLORREF());
        SendMessage(_hLstLayout, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
        SetWindowSubclass(_hLstLayout, DEWLAYOUTVIEWER::_LstLayoutMsgHandler, (UINT_PTR)_hLstLayout, (DWORD_PTR)this);
    }

    return;
}

void DEWLAYOUTVIEWER::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle       gdiFontStyle;
    LOGFONT         lgfFont { 0 };
    Color           crWinOurline;

    _crBack.SetFromCOLORREF(Theme.WinStyle.BackColor2);
    _crText.SetFromCOLORREF(Theme.ModuleStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.ModuleStyle.OutlineColor);
    crWinOurline.SetFromCOLORREF(Theme.WinStyle.OutlineColor);

    if (_pBrText) delete _pBrText;
    if (_pBrBack) delete _pBrBack;
    if (_pPenOutline) delete _pPenOutline;
    if (_pPenWinOutline) delete _pPenWinOutline;
    if (_pPenText) delete _pPenText;

    _pBrText = new SolidBrush(_crText);
    _pBrBack = new SolidBrush(_crBack);
    _pPenOutline = new Pen(_crOutline, (REAL)_F(1));
    _pPenWinOutline = new Pen(crWinOurline, (REAL)_F(1));
    _pPenText = new Pen(_crText, (REAL)_F(1));

    CopyMemory(_wsFontName, Theme.TextFontStyle.FontName, 32*sizeof(wchar_t));
    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[Theme.IconScale];
    _iFontSize = (int)((REAL)Theme.TextFontStyle.FontSize);
    _bBold = Theme.TextFontStyle.IsBold;
    _bItalic = Theme.TextFontStyle.IsItalic;
    if (_bBold)
        gdiFontStyle = _bItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = _bItalic ? FontStyleItalic : FontStyleRegular;

    if (_pFont) delete _pFont;
    _pFont = new Font(_wsFontName, (REAL)_iFontSize, gdiFontStyle, UnitPoint);
    _fDimension = 0.0f;
    _bAlternateIconMode = (Theme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE);
    _bTransparentIcons = (Theme.TransparentIcons == 1);

    // Scaling doesn't really do anything special
    if (bCalcScale) this->_Scale(Theme.IconScale);

    if (_hFont) { DeleteObject(_hFont); _hFont = NULL; }

    lgfFont.lfHeight = -MulDiv(_iFontSize, this->_iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = _bBold ? FW_BOLD : FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, _wsFontName, 32 * sizeof(wchar_t));
    _hFont = CreateFontIndirect(&lgfFont);

    if (_hLstLayout && _hLstLayout != INVALID_HANDLE_VALUE)
    {
        ListView_SetBkColor(_hLstLayout, _crBack.ToCOLORREF());
        ListView_SetTextBkColor(_hLstLayout, _crBack.ToCOLORREF());
        ListView_SetTextColor(_hLstLayout, _crText.ToCOLORREF());
        ListView_SetOutlineColor(_hLstLayout, _crOutline.ToCOLORREF());
        SendMessage(_hLstLayout, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
    }

    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWLAYOUTVIEWER::SetComponentStack(LPDEWUICOMPONENT ComponentStack, UINT nStackLength)
{
    LVITEM      lvItem;
    UINT        nIndex;
    wchar_t     wsText[64] { 0 };

    _lpComponentStack = ComponentStack;
    SendMessage(_hLstLayout, LVM_DELETEALLITEMS, 0, 0);

    if (_lpComponentStack)
    {
        lvItem.mask = LVIF_TEXT;

        for (nIndex = 0; nIndex < nStackLength; nIndex++)
        {
            if (_lpComponentStack[nIndex]._lpObject)
            {
                lvItem.iItem = nIndex;

                lvItem.pszText = _lpComponentStack[nIndex]._wsCaption;
                lvItem.iSubItem = 0;
                SendMessage(_hLstLayout, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

                StringCchPrintf(wsText, 32, L"%d x %d",
                                _lpComponentStack[nIndex]._lpObject->Width,
                                _lpComponentStack[nIndex]._lpObject->Height);
                lvItem.iSubItem = 1;
                lvItem.pszText = wsText;
                SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);

                StringCchPrintf(wsText, 32, L"%d", _lpComponentStack[nIndex]._lpObject->Position.x);
                lvItem.iSubItem = 2;
                lvItem.pszText = wsText;
                SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);

                StringCchPrintf(wsText, 32, L"%d", _lpComponentStack[nIndex]._lpObject->Position.y);
                lvItem.iSubItem = 3;
                lvItem.pszText = wsText;
                SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);
            }
        }
    }
    return;
}

void DEWLAYOUTVIEWER::UpdateComponentLocations(BYTE btObjectID)
{
    wchar_t     wsPos[16];
    int         iIndex;
    LVITEM      lvItem;

    // btObjectID values:
    // DEWUI_OBJ_NONE - De-select everything and do not update anything
    // DEWUI_OBJ_XX (valid) - Select the valid object and update its location
    // DEWUI_OBJECT_COUNT - Update all, but don't select anything

    // Deselect everything
    ListView_SetItemState(_hLstLayout, -1, 0, 0x00F);

    if (!_lpComponentStack) return;

    if (btObjectID > DEWUI_OBJ_NONE && btObjectID < DEWUI_OBJECT_COUNT)
    {
        iIndex = (int)(btObjectID - 1);

        StringCchPrintf(wsPos, 16, L"%d", _lpComponentStack[iIndex]._lpObject->Position.x);
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = iIndex;
        lvItem.iSubItem = 2;
        lvItem.pszText = wsPos;
        lvItem.cchTextMax = lstrlen(wsPos);
        SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);

        StringCchPrintf(wsPos, 16, L"%d", _lpComponentStack[iIndex]._lpObject->Position.y);
        lvItem.iSubItem = 3;
        lvItem.pszText = wsPos;
        lvItem.cchTextMax = lstrlen(wsPos);
        SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);

        ListView_SetItemState(_hLstLayout, iIndex, LVIS_SELECTED,  0x000F);
        // set item values
    }
    else if (btObjectID == DEWUI_OBJECT_COUNT)
    {
        // Set item values for all
        for (iIndex = 0; iIndex < DEWUI_OBJECT_COUNT - 2; iIndex++)
        {
            StringCchPrintf(wsPos, 16, L"%d", _lpComponentStack[iIndex]._lpObject->Position.x);
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = iIndex;
            lvItem.iSubItem = 1;
            lvItem.pszText = wsPos;
            lvItem.cchTextMax = lstrlen(wsPos);
            SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);

            StringCchPrintf(wsPos, 16, L"%d", _lpComponentStack[iIndex]._lpObject->Position.y);
            lvItem.iSubItem = 2;
            lvItem.pszText = wsPos;
            lvItem.cchTextMax = lstrlen(wsPos);
            SendMessage(_hLstLayout, LVM_SETITEM, 0, (LPARAM)&lvItem);
        }
    }

    return;
}

void DEWLAYOUTVIEWER::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWLAYOUTVIEWER::~DEWLAYOUTVIEWER()
{
    RemoveWindowSubclass(_hLstLayout, _LstLayoutMsgHandler, (UINT_PTR)_hLstLayout);
    DestroyWindow(_hLstLayout);
    if (_hFont) { DeleteObject(_hFont); _hFont = NULL; }
    return;
}

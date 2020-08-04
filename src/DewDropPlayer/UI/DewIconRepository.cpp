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

#include "DewIconRepository.h"

#pragma region STATIC INITIALIZATION

LPDEWSYMBOLFONT DEWICONREPOSITORY::_slpSymbolFont = NULL;
HIMAGELIST DEWICONREPOSITORY::_shImgOK = NULL;
HIMAGELIST DEWICONREPOSITORY::_shImgCancel = NULL;
HIMAGELIST DEWICONREPOSITORY::_shImgResetTheme = NULL;
HIMAGELIST DEWICONREPOSITORY::_shImgTab = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpLayoutMode = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpResetLayout = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpDewMode = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpAbout = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpQuit = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpDelete = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpExportSel = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpExportAll = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpOpenFile = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpOpenDir = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpOpenDisc = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpPlay = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpPause = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpStop = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpPrevious = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpNext = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpRestore = NULL;
#ifdef DEW_THEMER_MODE
HBITMAP DEWICONREPOSITORY::_shBmpDefaultTheme = NULL;
HBITMAP DEWICONREPOSITORY::_shBmpSave = NULL;
#endif // DEW_THEMER_MODE

UINT DEWICONREPOSITORY::_snInstanceCount = 0;

#pragma endregion

void DEWICONREPOSITORY::_CreateMenuHBITMAP(LPCWSTR wsSymbolText, int iIconSize, HBITMAP *phBmp)
{
    if (!phBmp || iIconSize == 0 || !wsSymbolText || lstrlen(wsSymbolText) <= 0) return;
    if (*phBmp) { DeleteObject(*phBmp); *phBmp = NULL; }

    Bitmap      *pImg = NULL;
    Font        *pFntIcon = NULL;
    Graphics    *pGr = NULL;
    SolidBrush  *pBrText = NULL;
    PointF      ptStart(0.0f, 0.0f);
    RectF       rctPos;

    pFntIcon = new Font((DEWICONREPOSITORY::_slpSymbolFont)->SymbolFontGDIPlus, (REAL)DEWUI_DIM_SYM_MENU_FONT, FontStyleRegular, UnitPoint);
    pBrText = new SolidBrush(Color::Black);
    pImg = new Bitmap(iIconSize, iIconSize, PixelFormat32bppARGB);
    pGr = Graphics::FromImage(pImg);
    pGr->SetTextRenderingHint(TextRenderingHintAntiAlias);
    pGr->MeasureString(wsSymbolText, -1, pFntIcon, ptStart, &rctPos);
    ptStart.X = ((REAL)iIconSize - rctPos.Width) / 2.0f;
    ptStart.Y = ((REAL)iIconSize - rctPos.Height) / 2.0f;
    pGr->DrawString(wsSymbolText, -1, pFntIcon, ptStart, pBrText);
    pImg->GetHBITMAP(Color::Black, phBmp);

    delete pGr;
    delete pImg;
    delete pBrText;
    delete pFntIcon;

    return;
}

void DEWICONREPOSITORY::_CreateButtonHIMAGELIST(LPCWSTR wsSymbolText, int iIconSize, HIMAGELIST* phiml)
{
    if (!phiml || iIconSize == 0 || !wsSymbolText || lstrlen(wsSymbolText) <= 0) return;
    if (*phiml) { ImageList_Destroy(*phiml); *phiml = NULL; }

    Bitmap      *pImg = NULL;
    Font        *pFntIcon = NULL;
    Graphics    *pGr = NULL;
    SolidBrush  *pBrText = NULL;
    PointF      ptStart(0.0f, 0.0f);
    RectF       rctPos;
    HBITMAP     hBmp;

    pFntIcon = new Font((DEWICONREPOSITORY::_slpSymbolFont)->SymbolFontGDIPlus, (REAL)DEWUI_DIM_SYM_BUTTON_FONT, FontStyleRegular, UnitPoint);
    pBrText = new SolidBrush(Color::Black);
    pImg = new Bitmap(iIconSize, iIconSize, PixelFormat32bppARGB);
    pGr = Graphics::FromImage(pImg);
    pGr->SetTextRenderingHint(TextRenderingHintAntiAlias);
    pGr->MeasureString(wsSymbolText, -1, pFntIcon, ptStart, &rctPos);
    ptStart.X = ((REAL)iIconSize - rctPos.Width) / 2.0f;
    ptStart.Y = ((REAL)iIconSize - rctPos.Height) / 2.0f;
    pGr->DrawString(wsSymbolText, -1, pFntIcon, ptStart, pBrText);
    pImg->GetHBITMAP(Color::Transparent, &hBmp);

    *phiml = ImageList_Create(iIconSize, iIconSize, ILC_COLOR32 | ILC_MASK, 1, 1);
    ImageList_Add(*phiml, hBmp, (HBITMAP)NULL);

    delete pGr;
    delete pImg;
    delete pBrText;
    delete pFntIcon;
    DeleteObject(hBmp);

    return;
}

void DEWICONREPOSITORY::_CreateStaticObjects()
{
    const int       BTN_ICON_SIZE = _F(DEWUI_DIM_SYM_BUTTON_ICON);
    const int       MENU_ICON_SIZE = _F((DEWUI_DIM_SYM_MENU_ICON));
    HBITMAP         hBmpConfig, hBmpHotkeys;

    DEWICONREPOSITORY::_slpSymbolFont = new DEWSYMBOLFONT();

    this->_CreateButtonHIMAGELIST(DEWUI_SYMBOL_BUTTON_OK, BTN_ICON_SIZE, &(DEWICONREPOSITORY::_shImgOK));
    this->_CreateButtonHIMAGELIST(DEWUI_SYMBOL_BUTTON_CANCEL, BTN_ICON_SIZE, &(DEWICONREPOSITORY::_shImgCancel));
    this->_CreateButtonHIMAGELIST(DEWUI_SYMBOL_BUTTON_RESET, BTN_ICON_SIZE, &(DEWICONREPOSITORY::_shImgResetTheme));

    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_CONFIG, MENU_ICON_SIZE, &hBmpConfig);
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_HOTKEY, MENU_ICON_SIZE, &hBmpHotkeys);
    DEWICONREPOSITORY::_shImgTab = ImageList_Create(MENU_ICON_SIZE, MENU_ICON_SIZE, ILC_COLOR32 | ILC_MASK, 1, 1);
    ImageList_Add((DEWICONREPOSITORY::_shImgTab), hBmpConfig, (HBITMAP)NULL);
    ImageList_Add((DEWICONREPOSITORY::_shImgTab), hBmpHotkeys, (HBITMAP)NULL);
    DeleteObject(hBmpConfig); DeleteObject(hBmpHotkeys);

    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_LAYOUT_MODE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpLayoutMode));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_RESET_LAYUOT, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpResetLayout));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_DEW_MODE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpDewMode));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_ABOUT, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpAbout));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_CLOSE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpQuit));

    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_DELETE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpDelete));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_EXPORT_SEL, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpExportSel));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_EXPORT_ALL, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpExportAll));

    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_OPEN_FILE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpOpenFile));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_OPEN_DIR, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpOpenDir));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_OPEN_DISC, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpOpenDisc));

    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_PLAY, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpPlay));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_PAUSE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpPause));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_STOP, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpStop));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_PREV, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpPrevious));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_NEXT, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpNext));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_RESTORE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpRestore));

    #ifdef DEW_THEMER_MODE
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_DEFAULT, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpDefaultTheme));
    this->_CreateMenuHBITMAP(DEWUI_SYMBOL_SAVE, MENU_ICON_SIZE, &(DEWICONREPOSITORY::_shBmpSave));
    #endif // DEW_THEMER_MODE

    return;
}

void DEWICONREPOSITORY::_DestroyStaticObjects()
{
    if (DEWICONREPOSITORY::_slpSymbolFont) { delete DEWICONREPOSITORY::_slpSymbolFont; DEWICONREPOSITORY::_slpSymbolFont = NULL; }

    if (DEWICONREPOSITORY::_shImgOK) { ImageList_Destroy(DEWICONREPOSITORY::_shImgOK); DEWICONREPOSITORY::_shImgOK = NULL; }
    if (DEWICONREPOSITORY::_shImgCancel) { ImageList_Destroy(DEWICONREPOSITORY::_shImgCancel); DEWICONREPOSITORY::_shImgCancel = NULL; }
    if (DEWICONREPOSITORY::_shImgResetTheme) { ImageList_Destroy(DEWICONREPOSITORY::_shImgResetTheme); DEWICONREPOSITORY::_shImgResetTheme = NULL; }
    if (DEWICONREPOSITORY::_shImgTab) { ImageList_Destroy(DEWICONREPOSITORY::_shImgTab); DEWICONREPOSITORY::_shImgTab = NULL; }

    if (DEWICONREPOSITORY::_shBmpLayoutMode) { DeleteObject(DEWICONREPOSITORY::_shBmpLayoutMode); DEWICONREPOSITORY::_shBmpLayoutMode = NULL; }
    if (DEWICONREPOSITORY::_shBmpResetLayout) { DeleteObject(DEWICONREPOSITORY::_shBmpResetLayout); DEWICONREPOSITORY::_shBmpResetLayout = NULL; }
    if (DEWICONREPOSITORY::_shBmpDewMode) { DeleteObject(DEWICONREPOSITORY::_shBmpDewMode); DEWICONREPOSITORY::_shBmpDewMode = NULL; }
    if (DEWICONREPOSITORY::_shBmpAbout) { DeleteObject(DEWICONREPOSITORY::_shBmpAbout); DEWICONREPOSITORY::_shBmpAbout = NULL; }
    if (DEWICONREPOSITORY::_shBmpQuit) { DeleteObject(DEWICONREPOSITORY::_shBmpQuit); DEWICONREPOSITORY::_shBmpQuit = NULL; }
    if (DEWICONREPOSITORY::_shBmpDelete) { DeleteObject(DEWICONREPOSITORY::_shBmpDelete); DEWICONREPOSITORY::_shBmpDelete = NULL; }
    if (DEWICONREPOSITORY::_shBmpExportSel) { DeleteObject(DEWICONREPOSITORY::_shBmpExportSel); DEWICONREPOSITORY::_shBmpExportSel = NULL; }
    if (DEWICONREPOSITORY::_shBmpExportAll) { DeleteObject(DEWICONREPOSITORY::_shBmpExportAll); DEWICONREPOSITORY::_shBmpExportAll = NULL; }
    if (DEWICONREPOSITORY::_shBmpOpenFile) { DeleteObject(DEWICONREPOSITORY::_shBmpOpenFile); DEWICONREPOSITORY::_shBmpOpenFile = NULL; }
    if (DEWICONREPOSITORY::_shBmpOpenDir) { DeleteObject(DEWICONREPOSITORY::_shBmpOpenDir); DEWICONREPOSITORY::_shBmpOpenDir = NULL; }
    if (DEWICONREPOSITORY::_shBmpOpenDisc) { DeleteObject(DEWICONREPOSITORY::_shBmpOpenDisc); DEWICONREPOSITORY::_shBmpOpenDisc = NULL; }
    if (DEWICONREPOSITORY::_shBmpPlay) { DeleteObject(DEWICONREPOSITORY::_shBmpPlay); DEWICONREPOSITORY::_shBmpPlay = NULL; }
    if (DEWICONREPOSITORY::_shBmpPause) { DeleteObject(DEWICONREPOSITORY::_shBmpPause); DEWICONREPOSITORY::_shBmpPause = NULL; }
    if (DEWICONREPOSITORY::_shBmpStop) { DeleteObject(DEWICONREPOSITORY::_shBmpStop); DEWICONREPOSITORY::_shBmpStop = NULL; }
    if (DEWICONREPOSITORY::_shBmpPrevious) { DeleteObject(DEWICONREPOSITORY::_shBmpPrevious); DEWICONREPOSITORY::_shBmpPrevious = NULL; }
    if (DEWICONREPOSITORY::_shBmpNext) { DeleteObject(DEWICONREPOSITORY::_shBmpNext); DEWICONREPOSITORY::_shBmpNext = NULL; }
    if (DEWICONREPOSITORY::_shBmpRestore) { DeleteObject(DEWICONREPOSITORY::_shBmpRestore); DEWICONREPOSITORY::_shBmpRestore = NULL; }

    #ifdef DEW_THEMER_MODE
    if (DEWICONREPOSITORY::_shBmpDefaultTheme) { DeleteObject(DEWICONREPOSITORY::_shBmpDefaultTheme); DEWICONREPOSITORY::_shBmpDefaultTheme = NULL; }
    if (DEWICONREPOSITORY::_shBmpSave) { DeleteObject(DEWICONREPOSITORY::_shBmpSave); DEWICONREPOSITORY::_shBmpSave = NULL; }
    #endif // DEW_THEMER_MODE

    return;
}

DEWICONREPOSITORY::DEWICONREPOSITORY(int iDPI, float fScale) :
ImgOK(DEWICONREPOSITORY::_shImgOK),
ImgCancel(DEWICONREPOSITORY::_shImgCancel),
ImgResetTheme(DEWICONREPOSITORY::_shImgResetTheme),
ImgTab(DEWICONREPOSITORY::_shImgTab),
BmpLayoutMode(DEWICONREPOSITORY::_shBmpLayoutMode),
BmpResetLayout(DEWICONREPOSITORY::_shBmpResetLayout),
BmpDewMode(DEWICONREPOSITORY::_shBmpDewMode),
BmpAbout(DEWICONREPOSITORY::_shBmpAbout),
BmpQuit(DEWICONREPOSITORY::_shBmpQuit),
BmpDelete(DEWICONREPOSITORY::_shBmpDelete),
BmpExportSel(DEWICONREPOSITORY::_shBmpExportSel),
BmpExportAll(DEWICONREPOSITORY::_shBmpExportAll),
BmpOpenFile(DEWICONREPOSITORY::_shBmpOpenFile),
BmpOpenDir(DEWICONREPOSITORY::_shBmpOpenDir),
BmpOpenDisc(DEWICONREPOSITORY::_shBmpOpenDisc),
BmpPlay(DEWICONREPOSITORY::_shBmpPlay),
BmpPause(DEWICONREPOSITORY::_shBmpPause),
BmpStop(DEWICONREPOSITORY::_shBmpStop),
BmpPrevious(DEWICONREPOSITORY::_shBmpPrevious),
BmpNext(DEWICONREPOSITORY::_shBmpNext),
BmpRestore(DEWICONREPOSITORY::_shBmpRestore)
#ifdef DEW_THEMER_MODE
,
BmpDefaultTheme(DEWICONREPOSITORY::_shBmpDefaultTheme),
BmpSave(DEWICONREPOSITORY::_shBmpSave)
#endif // DEW_THEMER_MODE
{
    _iDPI = iDPI;
    _fScale = fScale;

    // Create all the static elements for the first time only
    if (DEWICONREPOSITORY::_snInstanceCount == 0)
        this->_CreateStaticObjects();

    (DEWICONREPOSITORY::_snInstanceCount)++;
}

DEWICONREPOSITORY::~DEWICONREPOSITORY()
{
    (DEWICONREPOSITORY::_snInstanceCount)--;

    if (DEWICONREPOSITORY::_snInstanceCount == 0)
        this->_DestroyStaticObjects();
    return;
}

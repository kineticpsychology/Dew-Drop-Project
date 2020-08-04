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

#include "DewWindow.h"

BOOL DEWWINDOW::_GetSharedMemFileName(LPWSTR wsPath)
{
    BOOL            bRet = FALSE;

    if (!wsPath) return bRet;

    if (_lpMapContent)
    {
        // The poster should have updated this location
        // with a valid path name. Get that and close the map.
        if (_lpMapContent->wsMediaPath)
        {
            CopyMemory(wsPath, (_lpMapContent->wsMediaPath), sizeof(wchar_t)*MAX_CHAR_PATH);
            bRet = TRUE;
        }
    }

    return bRet;
}

void DEWWINDOW::_CreateUIComponents()
{
    UINT        nIndex;

    _lpCmdMinimize = new DEWMINIMIZEBUTTON(this, _PlayerSettings.Theme); _lpCmdMinimize->SetTip(L"Minimize");
    _lpCmdClose = new DEWCLOSEBUTTON(this, _PlayerSettings.Theme); _lpCmdClose->SetTip(L"Close");

    _lpAlbumArt = new DEWALBUMART(this, _PlayerSettings.Theme);
    _lpSeekBar = new DEWSEEKBAR(this, _PlayerSettings.Theme);
#ifndef DEW_THEMER_MODE
    _lpTitleInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_TITLE, L""); _lpTitleInfo->SetTip(L"Song Title");
    _lpArtistInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_ARTIST, L""); _lpArtistInfo->SetTip(L"Song Artist");
    _lpAlbumInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_ALBUM, L""); _lpAlbumInfo->SetTip(L"Song Album");
#else
    _lpTitleInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_TITLE, L"Lift"); _lpTitleInfo->SetTip(L"Song Title");
    _lpArtistInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_ARTIST, L"Poets Of The Fall"); _lpArtistInfo->SetTip(L"Song Artist");
    _lpAlbumInfo = new DEWINFOLABEL(this, _PlayerSettings.Theme, DEWUI_INFOLABEL_TYPE_ALBUM, L"Signs Of Life"); _lpAlbumInfo->SetTip(L"Song Album");
#endif
    _lpCmdPlayPause = new DEWPLAYPAUSEBUTTON(this, _PlayerSettings.Theme); _lpCmdPlayPause->SetTip(L"Ready to play");
    _lpCmdStop = new DEWSTOPBUTTON(this, _PlayerSettings.Theme); _lpCmdStop->SetTip(L"Stop song");
    _lpCmdPrev = new DEWPREVIOUSBUTTON(this, _PlayerSettings.Theme); _lpCmdPrev->SetTip(L"Play previous track");
    _lpCmdNext = new DEWNEXTBUTTON(this, _PlayerSettings.Theme); _lpCmdNext->SetTip(L"Play next track");
    _lpCmdVolume = new DEWVOLUMEBUTTON(this, _PlayerSettings.Theme, _PlayerSettings.VolumeLevel); _lpCmdVolume->SetTip(L"Hold and drag mouse to adjust volume");
    _lpCmdRepeat = new DEWREPEATBUTTON(this, _PlayerSettings.Theme, _PlayerSettings.RepeatLevel);
    _lpCmdPlaylist = new DEWPLAYLISTBUTTON(this, _PlayerSettings.Theme); _lpCmdPlaylist->SetTip(L"Toggle playlist section");
    _lpCmdBrowse = new DEWBROWSEBUTTON(this, _PlayerSettings.Theme); _lpCmdBrowse->SetTip(L"Browse and select audio files/folders/disc");
    _lpCmdSettings = new DEWSETTINGSBUTTON(this, _PlayerSettings.Theme); _lpCmdSettings->SetTip(L"Dew Drop Player Settings");
    _lpCmdExInfo = new DEWEXINFOBUTTON(this, _PlayerSettings.Theme); _lpCmdExInfo->SetTip(L"Show extended information");
    _lpCmdTimer = new DEWTIMERBUTTON(this, _PlayerSettings.Theme); _lpCmdTimer->SetTip(L"Set countdown timer to auto-shutdown Dew Drop Player");
    _lpCmdDewMode = new DEWDEWMODEBUTTON(this, _PlayerSettings.Theme); _lpCmdDewMode->SetTip(L"Switch to Dew Mode");
#ifndef DEW_THEMER_MODE
    _lpPlaylist = new DEWPLAYLIST(this, _PlayerSettings.Theme, _b_EP_DeepScan);
#else
    _lpPlaylist = new DEWPLAYLIST(this, _PlayerSettings.Theme, FALSE);
#endif

    _lpLayoutViewer = new DEWLAYOUTVIEWER(this, _PlayerSettings.Theme);

    _lpPropPage = new DEWPROPERTYPAGE(_hWnd, _PlayerSettings);
    _lpShutdownTimer = new DEWSHUTDOWNTIMER(_hWnd, _iDPI, _fScale);
    _lpPlaylistExporter = new DEWPLAYLISTEXPORTER(this, _lpPlaylist, _iDPI, _fScale);
    _lpDiscSelector = new DEWDISCSELECTOR(_hWnd, _iDPI, _fScale);
#ifndef DEW_THEMER_MODE
    _lpAboutWindow = new DEWABOUTWINDOW(_hWnd, _iDPI, _fScale);
#endif
    _lpCmdDMPlayPause = new DEWDMPLAYPAUSEBUTTON(this, _PlayerSettings.Theme);
    _lpIconRepo = new DEWICONREPOSITORY(_iDPI, _fScale);

    this->_CreateOpenMenu();

    _ducComponents[0]._lpObject = _lpCmdMinimize; StringCchPrintf((_ducComponents[0]._wsCaption), 32, DEWCTL_CAP_MIN_BUTTON);
    _ducComponents[1]._lpObject = _lpCmdClose; StringCchPrintf((_ducComponents[1]._wsCaption), 32, DEWCTL_CAP_CLOSE_BUTTON);
    _ducComponents[2]._lpObject = _lpAlbumArt; StringCchPrintf((_ducComponents[2]._wsCaption), 32, DEWCTL_CAP_ALBUM_ART);
    _ducComponents[3]._lpObject = _lpSeekBar; StringCchPrintf((_ducComponents[3]._wsCaption), 32, DEWCTL_CAP_SEEKBAR);
    _ducComponents[4]._lpObject = _lpTitleInfo; StringCchPrintf((_ducComponents[4]._wsCaption), 32, DEWCTL_CAP_INFO_TITLE);
    _ducComponents[5]._lpObject = _lpArtistInfo; StringCchPrintf((_ducComponents[5]._wsCaption), 32, DEWCTL_CAP_INFO_ARTIST);
    _ducComponents[6]._lpObject = _lpAlbumInfo; StringCchPrintf((_ducComponents[6]._wsCaption), 32, DEWCTL_CAP_INFO_ALBUM);
    _ducComponents[7]._lpObject = _lpCmdPlayPause; StringCchPrintf((_ducComponents[7]._wsCaption), 32, DEWCTL_CAP_PLAY_PAUSE_BUTTON);
    _ducComponents[8]._lpObject = _lpCmdStop; StringCchPrintf((_ducComponents[8]._wsCaption), 32, DEWCTL_CAP_STOP_BUTTON);
    _ducComponents[9]._lpObject = _lpCmdPrev; StringCchPrintf((_ducComponents[9]._wsCaption), 32, DEWCTL_CAP_PREV_BUTTON);
    _ducComponents[10]._lpObject = _lpCmdNext; StringCchPrintf((_ducComponents[10]._wsCaption), 32, DEWCTL_CAP_NEXT_BUTTON);
    _ducComponents[11]._lpObject = _lpCmdVolume; StringCchPrintf((_ducComponents[11]._wsCaption), 32, DEWCTL_CAP_VOLUME_BUTTON);
    _ducComponents[12]._lpObject = _lpCmdRepeat; StringCchPrintf((_ducComponents[12]._wsCaption), 32, DEWCTL_CAP_REPEAT_BUTTON);
    _ducComponents[13]._lpObject = _lpCmdPlaylist; StringCchPrintf((_ducComponents[13]._wsCaption), 32, DEWCTL_CAP_PLAYLIST_BUTTON);
    _ducComponents[14]._lpObject = _lpCmdBrowse; StringCchPrintf((_ducComponents[14]._wsCaption), 32, DEWCTL_CAP_BROWSE_BUTTON);
    _ducComponents[15]._lpObject = _lpCmdSettings; StringCchPrintf((_ducComponents[15]._wsCaption), 32, DEWCTL_CAP_SETTINGS_BUTTON);
    _ducComponents[16]._lpObject = _lpCmdExInfo; StringCchPrintf((_ducComponents[16]._wsCaption), 32, DEWCTL_CAP_EXINFO_BUTTON);
    _ducComponents[17]._lpObject = _lpCmdTimer; StringCchPrintf((_ducComponents[17]._wsCaption), 32, DEWCTL_CAP_TIMER_BUTTON);
    _ducComponents[18]._lpObject = _lpCmdDewMode; StringCchPrintf((_ducComponents[18]._wsCaption), 32, DEWCTL_CAP_DEWMODE_BUTTON);
    _ducComponents[19]._lpObject = _lpPlaylist; StringCchPrintf((_ducComponents[19]._wsCaption), 32, DEWCTL_CAP_PLAYLIST);
    _ducComponents[20]._lpObject = _lpLayoutViewer; StringCchPrintf((_ducComponents[20]._wsCaption), 32, DEWCTL_CAP_LAYOUT_VIEWER);

    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++)
    {
        // We do not need an explicit 'Hide' style, as WS_VISIBLE style is not applied on children
        if (!(_PlayerSettings.ComponentVisibility[nIndex]) && _ducComponents[nIndex]._lpObject)
            _ducComponents[nIndex]._lpObject->SetVisible(FALSE);
        if (_ducComponents[nIndex]._lpObject)
            _ducComponents[nIndex]._lpObject->SetLocation(_PlayerSettings.ComponentPos[nIndex]);
    }
    // Playlist will deviate from the settings (both position & visibility)
    _lpPlaylist->SetVisible();
    _lpLayoutViewer->SetVisible(FALSE);

    return;
}

void DEWWINDOW::_SetLayoutMode()
{
    UINT        nIndex;

    _bLayoutMode = !_bLayoutMode;
    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++)
        _ducComponents[nIndex]._lpObject->SetLayoutMode(_bLayoutMode, TRUE);
    this->_ShowFullView((_bLayoutMode ? TRUE : _PlayerSettings.PlaylistVisible));
    _lpPlaylist->SetVisible(!_bLayoutMode);
    _lpLayoutViewer->SetVisible(_bLayoutMode);
    if (_bLayoutMode)
    {
        _lpLayoutViewer->Refresh();
        // Layout mode is starting. Populate the temporary lookups with the actual values
        CopyMemory(_lpdwLayoutPos, (_PlayerSettings.ComponentPos), DEWUI_OBJECT_COUNT * sizeof(DWORD));
        CopyMemory(_lpbLayoutVisibility, (_PlayerSettings.ComponentVisibility), DEWUI_OBJECT_COUNT * sizeof(BOOL));
        _bShowTitle = _PlayerSettings.ShowTitle;
    }
    else
    {
        _lpPlaylist->Refresh();
        // Layout mode is complete. Put back the original values from the temporary lookups to the settings arrays
        CopyMemory((_PlayerSettings.ComponentPos), _lpdwLayoutPos, DEWUI_OBJECT_COUNT * sizeof(DWORD));
        CopyMemory((_PlayerSettings.ComponentVisibility), _lpbLayoutVisibility, DEWUI_OBJECT_COUNT * sizeof(BOOL));
        _PlayerSettings.ShowTitle = _bShowTitle;
    }
    // Reset the active object id during transitions
    _btActiveLayoutObjectID = DEWUI_OBJ_NONE;
    return;
}

void DEWWINDOW::_ResetLayout()
{
    UINT        nIndex;

    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
    {
        if (_ducComponents[nIndex]._lpObject)
            _ducComponents[nIndex]._lpObject->SetLocation(0x00);
    }

    // Update the locations of all the controls
    _btActiveLayoutObjectID = DEWUI_OBJECT_COUNT;
    _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);

    // Reset the position matrix
    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
        _lpdwLayoutPos[nIndex] = 0x00;
    return;
}

void DEWWINDOW::_SetDewMode(BOOL bDewMode)
{
    Graphics    *pGr = NULL;
    UINT        nIndex;
    RECT        rctPos;

    _PlayerSettings.DewMode = bDewMode;

    if (!_PlayerSettings.DewMode)
    {
        _lpCmdDMPlayPause->SetVisible(FALSE);
        SetWindowLongPtr(_hWnd, GWL_EXSTYLE, (LONG)(WS_EX_LAYERED | WS_EX_ACCEPTFILES));
        SetLayeredWindowAttributes(_hWnd, 0x00, 0xFF, LWA_ALPHA);
        // This is just to ensure that the background is reset
        this->ApplyTheme(_PlayerSettings.Theme, TRUE);
        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
        {
            if (_ducComponents[nIndex]._lpObject)
            {
                _ducComponents[nIndex]._lpObject->SetVisible(_PlayerSettings.ComponentVisibility[nIndex]);
                _ducComponents[nIndex]._lpObject->SetLocation(_PlayerSettings.ComponentPos[nIndex]);
            }
        }

        this->_Draw(NULL);
        SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, _iWidth,
                     (_PlayerSettings.PlaylistVisible ? _iHeight : _iTrimmedHeight),
                     SWP_NOZORDER);
        SetWindowPos(_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        return;
    }

    // Hide everything
    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
        if (_ducComponents[nIndex]._lpObject) _ducComponents[nIndex]._lpObject->SetVisible(FALSE);

    // Backup the location before changing the position/size
    GetWindowRect(_hWnd, &rctPos);
    _ptPos.x = rctPos.left;
    _ptPos.y = rctPos.top;

    // Refresh background image with new dimension
    if (DEWUIOBJECT::_pImgBackground) { delete DEWUIOBJECT::_pImgBackground; DEWUIOBJECT::_pImgBackground = NULL; }
    DEWUIOBJECT::_pImgBackground = new Bitmap(_iDewModeWidth, _iDewModeHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(DEWUIOBJECT::_pImgBackground);
    if (_pBrBack) { delete _pBrBack;  _pBrBack = NULL; }
    _pBrBack = new LinearGradientBrush(Point(_iDewModeWidth / 2, 0), Point(_iDewModeWidth / 2, _iDewModeHeight), _crBack, _crBack2);
    pGr->FillRectangle(_pBrBack, 0, 0, _iDewModeWidth, _iDewModeHeight);

    delete pGr;
    pGr = NULL;

    // Call this only after 'DEWUIOBJECT::_pImgBackground' has been refreshed
    // or the transparency illusion will not work!
    _lpCmdDMPlayPause->Move(_F(1), _F(1));
    _lpCmdDMPlayPause->SetVisible(TRUE);

    this->_SnapDewModeWindow(0x00);
    SetWindowLongPtr(_hWnd, GWL_EXSTYLE, (LONG)(WS_EX_LAYERED | WS_EX_TOOLWINDOW));
    SetLayeredWindowAttributes(_hWnd, 0x00, 0xFF, LWA_ALPHA);
    if (_PlayerSettings.TopmostInDewMode)
        SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    return;
}

void DEWWINDOW::_ShowFullView(BOOL bFullView)
{
    int     iY, iH;
    RECT    rctWindow, rctWorkArea { 0 };

    GetWindowRect(_hWnd, &rctWindow);
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    iH = (bFullView ? _iHeight : _iTrimmedHeight);
    iY = (rctWindow.top + iH <= rctWorkArea.bottom) ? rctWindow.top : (rctWorkArea.bottom - iH);
    SetWindowPos(_hWnd, 0, rctWindow.left, iY, _iWidth, iH, SWP_NOZORDER);
    if (!_bLayoutMode)
        this->_PlayerSettings.PlaylistVisible = bFullView ? 1 : 0;
    return;
}

void DEWWINDOW::_SnapDewModeWindow(BYTE btVK_Key)
{
    int         iHPos, iVPos, iXPos, iYPos;
    RECT        rctWorkArea { 0 };

    if (!_PlayerSettings.DewMode) return;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    if (btVK_Key != 0x00)
    {
        iHPos = (_btDewModePos >> 4);
        iVPos = (_btDewModePos & 0x0F);
    }

    switch(btVK_Key)
    {
        case VK_LEFT: if (iHPos > 0) iHPos--; break;
        case VK_UP: if (iVPos > 0) iVPos--; break;
        case VK_RIGHT: if (iHPos < 2) iHPos++; break;
        case VK_DOWN: if (iVPos < 2) iVPos++; break;
        default: iHPos = 1; iVPos = 0; break;
    }

    switch(iHPos)
    {
        case 0: iXPos = rctWorkArea.left; break;
        case 2: iXPos = rctWorkArea.right - _iDewModeWidth; break;
        // case 1, default
        default: iXPos = (rctWorkArea.right - rctWorkArea.left - _iDewModeWidth) / 2 + rctWorkArea.left; break;
    }

    switch(iVPos)
    {
        case 1: iYPos = (rctWorkArea.bottom - rctWorkArea.top - _iDewModeHeight) / 2 + rctWorkArea.top; break;
        case 2: iYPos = rctWorkArea.bottom - _iDewModeHeight; break;
        // case 0, default
        default: iYPos = rctWorkArea.top;
    }

    _btDewModePos = (BYTE)((iHPos << 4) | (iVPos));
    SetWindowPos(_hWnd, 0, iXPos, iYPos, _iDewModeWidth, _iDewModeHeight, SWP_NOZORDER);
    return;
}

void DEWWINDOW::_TrayMenu(int iX, int iY)
{

#ifndef DEW_THEMER_MODE

    MENUITEMINFO        mii { 0 };
    UINT                nPos = 0, nItemCount = 0;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_BITMAP;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;
    nItemCount = _lpPlaylist->GetFileCount();

    if (_hMnuTray) { DestroyMenu(_hMnuTray); _hMnuTray = NULL; }
    _hMnuTray = CreatePopupMenu();

    if (_lpAudioEngine) // Pause/Resume
    {
        mii.wID = IDM_TRAY_PLAYPAUSE;
        
        if (_lpAudioEngine->Status == DEWS_MEDIA_PAUSED)
            { mii.dwTypeData = (LPTSTR)L"Resume"; mii.hbmpItem = _lpIconRepo->BmpPlay; }
        else if (_lpAudioEngine->Status == DEWS_MEDIA_PLAYING)
            { mii.dwTypeData = (LPTSTR)L"Pause"; mii.hbmpItem = _lpIconRepo->BmpPause; }
        InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);
    }
    else if (nItemCount > 0) // The 'Play' option will be available only if there is at least one item
    {
        mii.fState = MFS_ENABLED;
        mii.wID = IDM_TRAY_PLAYPAUSE;
        mii.dwTypeData = (LPTSTR)L"Play";
        mii.hbmpItem = _lpIconRepo->BmpPlay;
        InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);
    }
    else
    {
        mii.fState = MFS_DISABLED;
        mii.wID = IDM_TRAY_PLAYPAUSE;
        mii.dwTypeData = (LPTSTR)L"Play";
        mii.hbmpItem = _lpIconRepo->BmpPlay;
        InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);
    }
    
    // Stop, Previous & Next will only be available for an active engine
    mii.fState = ((_lpAudioEngine != NULL) ? MFS_ENABLED : MFS_DISABLED);
    
    mii.wID = IDM_TRAY_STOP;
    mii.dwTypeData = (LPTSTR)L"Stop"; mii.hbmpItem = _lpIconRepo->BmpStop;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);

    // Previous
    mii.wID = IDM_TRAY_PREV;
    mii.dwTypeData = (LPTSTR)L"Previous"; mii.hbmpItem = _lpIconRepo->BmpPrevious;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);

    // Next
    mii.wID = IDM_TRAY_NEXT;
    mii.dwTypeData = (LPTSTR)L"Next"; mii.hbmpItem = _lpIconRepo->BmpNext;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);

    // Separator
    mii.fState = MFS_ENABLED;
    mii.wID = IDM_TRAY_SEP;
    mii.fType = MFT_SEPARATOR;
    mii.dwTypeData = (LPTSTR)L""; mii.hbmpItem = NULL;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);
    
    mii.fType = MFT_STRING;

    // Restore
    mii.wID = IDM_TRAY_RESTORE;
    mii.dwTypeData = (LPTSTR)L"Restore"; mii.hbmpItem = _lpIconRepo->BmpRestore;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);

    // Quit
    mii.wID = IDM_TRAY_QUIT;
    mii.dwTypeData = (LPTSTR)L"Quit"; mii.hbmpItem = _lpIconRepo->BmpQuit;
    InsertMenuItem(_hMnuTray, nPos++, TRUE, &mii);

    // Refer to 'Jack's answer in the link: (He's a freaking genius)
    // https://stackoverflow.com/questions/4145561/system-tray-context-menu-doesnt-disappear
    // Apparently, if we don't make the window foreground, tray context menu won't disappear
    // Also, this needs to be done BEFORE we display the popup menu!
    SetForegroundWindow(_hWnd);
    TrackPopupMenuEx(_hMnuTray, TPM_RIGHTBUTTON, iX, iY, _hWnd, NULL);

#endif

    return;
}

void DEWWINDOW::_ContextMenu(int iX, int iY)
{
    MENUITEMINFO        miiMain { 0 }, miiSub { 0 };
    UINT                nPos = 0, nSubPosHide = 0, nSubPosShow = 0, nIndex = 0;
    RECT                rctPlaylist;

    // Since _ContextMenu is driven by WM_CONTEXTMENU message
    // We'll not allow this to popup if it is in the area of the playlist
    GetWindowRect(_lpPlaylist->Handle, &rctPlaylist);
    if (IsWindowVisible(_lpPlaylist->Handle) &&
        iX >= rctPlaylist.left && iX <= rctPlaylist.right &&
        iY >= rctPlaylist.top && iY <= rctPlaylist.bottom)
        return;

    miiMain.cbSize = sizeof(MENUITEMINFO);
    miiMain.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_BITMAP;
    miiMain.fType = MFT_STRING;

    if (_hMnuContext) { DestroyMenu(_hMnuContext); _hMnuContext = NULL; }
    if (_hMnuShow) { DestroyMenu(_hMnuShow); _hMnuShow = NULL; }
    if (_hMnuHide) { DestroyMenu(_hMnuHide); _hMnuHide = NULL; }
    _hMnuContext = CreatePopupMenu();

    // Layout mode will not show during Dew Mode
    if (!_PlayerSettings.DewMode)
    {
        if (_bLayoutMode)
        {
            miiMain.fState = MFS_CHECKED;
            miiMain.hbmpItem = NULL;
        }
        else
        {
            miiMain.hbmpItem = _lpIconRepo->BmpLayoutMode;
        }
        miiMain.wID = IDM_CTX_LAYOUT;
        miiMain.dwTypeData = (LPTSTR)L"Layout Mode";
        miiMain.cch = 11;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
        miiMain.fState = MFS_UNCHECKED;
    }

    if (!_bLayoutMode)
    {
        if (_PlayerSettings.DewMode)
        {
            miiMain.fState = MFS_CHECKED;
            miiMain.hbmpItem = NULL;
        }
        else
        {
            miiMain.hbmpItem = _lpIconRepo->BmpDewMode;
        }
        miiMain.wID = IDM_CTX_DEWMODE;
        miiMain.dwTypeData = (LPTSTR)L"Dew Mode";
        miiMain.cch = 8;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
        miiMain.fState = MFS_UNCHECKED;
    }

    // Conditional (applicable for Layout mode only)
    if (_bLayoutMode)
    {
        miiSub.cbSize = sizeof(MENUITEMINFO);
        miiSub.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID;
        miiSub.fType = MFT_STRING;

        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++) // Skip Playlist & Layout viewer
        {
            if (_lpbLayoutVisibility[nIndex]) // A 'Hide' menu for visible items
            {
                if (!_hMnuHide) _hMnuHide = CreatePopupMenu();
                miiSub.wID = IDM_CTX_HIDE + _ducComponents[nIndex]._lpObject->ObjectID;
                miiSub.dwTypeData = (LPTSTR)(_ducComponents[nIndex]._wsCaption);
                miiSub.cch = lstrlen(_ducComponents[nIndex]._wsCaption);
                InsertMenuItem(_hMnuHide, nSubPosHide++, TRUE, &miiSub);
            }
            else // A 'Show' menu for invisible items
            {
                if (!_hMnuShow) _hMnuShow = CreatePopupMenu();
                miiSub.wID = IDM_CTX_SHOW + _ducComponents[nIndex]._lpObject->ObjectID;
                miiSub.dwTypeData = (LPTSTR)(_ducComponents[nIndex]._wsCaption);
                miiSub.cch = lstrlen(_ducComponents[nIndex]._wsCaption);
                InsertMenuItem(_hMnuShow, nSubPosShow++, TRUE, &miiSub);
            }
        }

        if (_bShowTitle)
        {
            if (!_hMnuHide) _hMnuHide = CreatePopupMenu();
            miiSub.wID = IDM_CTX_HIDE_TITLE;
            miiSub.dwTypeData = L"Title Text";
            miiSub.cch = 10;
            InsertMenuItem(_hMnuHide, nSubPosHide++, TRUE, &miiSub);
        }
        else
        {
            if (!_hMnuShow) _hMnuShow = CreatePopupMenu();
            miiSub.wID = IDM_CTX_SHOW_TITLE;
            miiSub.dwTypeData = L"Title Text";
            miiSub.cch = 10;
            InsertMenuItem(_hMnuShow, nSubPosShow++, TRUE, &miiSub);
        }

        miiMain.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;
        if (_hMnuShow)
        {
            // Separator
            miiSub.wID = IDM_TRAY_SEP;
            miiSub.fType = MFT_SEPARATOR;
            miiSub.dwTypeData = (LPTSTR)L""; miiSub.cch = 0;
            InsertMenuItem(_hMnuShow, nSubPosShow++, TRUE, &miiSub);
            miiSub.fType = MFT_STRING;

            // Add a 'Show-All' option
            miiSub.wID = IDM_CTX_SHOW;
            miiSub.dwTypeData = (LPTSTR)L"Show All";
            miiSub.cch = 9;
            InsertMenuItem(_hMnuShow, nSubPosShow++, TRUE, &miiSub);

            miiMain.wID = IDM_CTX_SHOW;
            miiMain.hSubMenu = _hMnuShow;
            miiMain.dwTypeData = (LPTSTR)L"Show";
            miiMain.cch = 4;
            InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
        }

        if (_hMnuHide)
        {
            // Separator
            miiSub.wID = IDM_TRAY_SEP;
            miiSub.fType = MFT_SEPARATOR;
            miiSub.dwTypeData = (LPTSTR)L""; miiSub.cch = 0;
            InsertMenuItem(_hMnuHide, nSubPosHide++, TRUE, &miiSub);
            miiSub.fType = MFT_STRING;

            // Add a 'Hide-All' option
            miiSub.wID = IDM_CTX_HIDE;
            miiSub.dwTypeData = (LPTSTR)L"Hide All";
            miiSub.cch = 9;
            InsertMenuItem(_hMnuHide, nSubPosHide++, TRUE, &miiSub);

            miiMain.wID = IDM_CTX_HIDE;
            miiMain.hSubMenu = _hMnuHide;
            miiMain.dwTypeData = (LPTSTR)L"Hide";
            miiMain.cch = 4;
            InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
        }
        miiMain.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_BITMAP;
        miiMain.hbmpItem = NULL;
    }

    if (_bLayoutMode)
    {
        // Reset Layout
        miiMain.wID = IDM_CTX_RESET_LAYOUT;
        miiMain.dwTypeData = (LPTSTR)L"Reset to default layout";
        miiMain.cch = 23;
        miiMain.hbmpItem = _lpIconRepo->BmpResetLayout;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
    }

    // Separator
    miiMain.wID = IDM_TRAY_SEP;
    miiMain.fType = MFT_SEPARATOR;
    miiMain.dwTypeData = (LPTSTR)L""; miiMain.cch = 0;
    InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);
    miiMain.fType = MFT_STRING;

    // About
    miiMain.fType = MFT_STRING;
    miiMain.wID = IDM_CTX_ABOUT;
    miiMain.dwTypeData = (LPTSTR)L"About";
    miiMain.cch = 5;
    miiMain.hbmpItem = _lpIconRepo->BmpAbout;
    InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);

    // Quit
    miiMain.wID = IDM_TRAY_QUIT;
    miiMain.dwTypeData = (LPTSTR)L"Quit";
    miiMain.cch = 4;
    miiMain.hbmpItem = _lpIconRepo->BmpQuit;
    InsertMenuItem(_hMnuContext, nPos++, TRUE, &miiMain);

    TrackPopupMenuEx(_hMnuContext, TPM_RIGHTBUTTON, iX, iY, _hWnd, NULL);
    return;
}

void DEWWINDOW::_CreateOpenMenu()
{
    MENUITEMINFO    miiOpen { 0 };
    UINT            nPos = 0;

    if (_hMnuOpen) { DestroyMenu(_hMnuOpen); _hMnuOpen = NULL; }
    _hMnuOpen = CreatePopupMenu();

    miiOpen.cbSize = sizeof(MENUITEMINFO);
    miiOpen.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
    miiOpen.fType = MFT_STRING;

    // Open File
    miiOpen.wID = IDM_OPEN_FILE;
    miiOpen.dwTypeData = (LPTSTR)L"Open Audio File(s)";
    miiOpen.cch = 18;
    miiOpen.hbmpItem = _lpIconRepo->BmpOpenFile;
    InsertMenuItem(_hMnuOpen, nPos++, TRUE, &miiOpen);

    // Open Folder
    miiOpen.wID = IDM_OPEN_DIR;
    miiOpen.dwTypeData = (LPTSTR)L"Open Folder";
    miiOpen.cch = 11;
    miiOpen.hbmpItem = _lpIconRepo->BmpOpenDir;
    InsertMenuItem(_hMnuOpen, nPos++, TRUE, &miiOpen);

    // Open Audio CD
    miiOpen.wID = IDM_OPEN_DISC;
    miiOpen.dwTypeData = (LPTSTR)L"Open Audio CD";
    miiOpen.cch = 13;
    miiOpen.hbmpItem = _lpIconRepo->BmpOpenDisc;
    InsertMenuItem(_hMnuOpen, nPos++, TRUE, &miiOpen);

    return;
}

void DEWWINDOW::_HideToTray()
{
    _nidTray.hIcon = _hIcon;
    StringCchPrintf(_nidTray.szTip, 128, _wsText);
    Shell_NotifyIcon(NIM_ADD, &_nidTray);
    this->SetVisible(FALSE);
    return;
}

void DEWWINDOW::_RestoreFromTray()
{
    ShowWindow(_hWnd, SW_RESTORE);
    Shell_NotifyIcon(NIM_DELETE, &_nidTray);
    SetForegroundWindow(_hWnd);
    if (_hMnuTray) { DestroyMenu(_hMnuTray); _hMnuTray = NULL; }
    return;
}

void DEWWINDOW::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;
    PointF      ptTitle(0.0f, 0.0f);
    RectF       rctTitleSpan;
    UINT        nIndex;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0);

    if (!_PlayerSettings.DewMode)
    {
        ZeroMemory(&ptTitle, sizeof(PointF));
        pGr->MeasureString(_wsText, -1, _pFont, ptTitle, &rctTitleSpan);
        ptTitle.X = (REAL)_F(DEWUI_DIM_PADDING_Y_HEADER);
        ptTitle.Y = (ptTitle.X - rctTitleSpan.Height) / 2.0f;

        if (_bLayoutMode)
        {
            if (_bShowTitle)
            {
                pGr->DrawString(_wsText, -1, _pFont, ptTitle, _pBrText);
                if (_pImgIcon)
                    pGr->DrawImage(_pImgIcon, 0, 0);
                else
                    pGr->DrawString(DEWUI_SYMBOL_MAIN_APP, -1, _pFntIcon, _ptIconStart, _pBrText);
            }
        }
        else
        {
            if (_PlayerSettings.ShowTitle)
            {
                pGr->DrawString(_wsText, -1, _pFont, ptTitle, _pBrText);
                if (_pImgIcon)
                    pGr->DrawImage(_pImgIcon, 0, 0);
                else
                    pGr->DrawString(DEWUI_SYMBOL_MAIN_APP, -1, _pFntIcon, _ptIconStart, _pBrText);
            }
        }

        if (!(this->_PlayerSettings.PlaylistVisible) && !_bLayoutMode)
            pGr->DrawRectangle(_pPenOutline, _F(0), _F(0), _iWidth - _F(1), _iTrimmedHeight - _F(1));
        else if (this->_PlayerSettings.PlaylistVisible || _bLayoutMode)
            pGr->DrawRectangle(_pPenOutline, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));

        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++)
        {
            if (_ducComponents[nIndex]._lpObject)
            {
                if (_bLayoutMode)
                {
                    if (_lpbLayoutVisibility[nIndex])
                        _ducComponents[nIndex]._lpObject->Refresh();
                }
                else
                {
                    if (_PlayerSettings.ComponentVisibility[nIndex])
                        _ducComponents[nIndex]._lpObject->Refresh();
                }
            }
        }
    }
    else
    {
    #ifndef DEW_THEMER_MODE
        RectF   rctDewTextBounds((REAL)(_lpCmdDMPlayPause->Position.x + _lpCmdDMPlayPause->Width),
                                 (REAL)(_lpCmdDMPlayPause->Position.y),
                                 (REAL)((_iDewModeWidth - _F(1)) - (_lpCmdDMPlayPause->Position.x + _lpCmdDMPlayPause->Width)),
                                 (REAL)(_iDewModeHeight - _F(1) - _lpCmdDMPlayPause->Position.y));
        wchar_t         wsDMTitle[DEWUI_MAX_TITLE]{ 0 };
        PointF          ptStart(0.0f, 0.0f);
        RectF           rctSize(0.0f, 0.0f, 0.0f, 0.0f);
        
        // Draw only the audio title, if available. Otherwise, just draw the player title
        if (_lpAudioEngine)
            StringCchPrintf(wsDMTitle, DEWUI_MAX_TITLE, (_lpAudioEngine->Tag.Title));
        else
            StringCchPrintf(wsDMTitle, DEWUI_MAX_TITLE, DEWUI_CAP_DEFAULT);
        
        // We need the size just to align the text vertically.
        // Bummer alert! Apparently doing a DrawString with a layout rect
        // and StringFormat messes up when drawing unicode strings :(
        pGr->MeasureString(wsDMTitle, -1, _pFont, ptStart, &rctSize);
        ptStart.X = rctDewTextBounds.X;
        ptStart.Y = (rctDewTextBounds.Height - rctSize.Height) / 2.0f + rctDewTextBounds.Y;
        
        pGr->DrawRectangle(_pPenOutline, _F(0), _F(0), _iDewModeWidth - _F(1), _iDewModeHeight - _F(1));
        pGr->DrawString(wsDMTitle, -1, _pFont, ptStart, _pBrText);
    #endif
    }

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWWINDOW::_BrowseAndSetFiles()
{
    OPENFILENAME        ofn { 0 };
    const DWORD         MAX_SONG_PATH_CAPACITY = 32760;
    wchar_t             wsSongList[MAX_SONG_PATH_CAPACITY] { 0 };
    wchar_t             wsDir[MAX_CHAR_PATH] { 0 };
    wchar_t             wsSong[MAX_CHAR_PATH] { 0 };
    UINT                nIndex, nLength;
    wchar_t             *wsStringPart = NULL;
    BYTE                btAtLeastOneSongAdded = 0;
    DWORD               dwAttr = 0;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWnd;
    // Ripped straight from the AUDIOFACTORY::GetMediaTypeFromExtension() function
    ofn.lpstrFilter = L"Supported Audio Files (3GP, AAC, AC3, AIF/AIFF, APE, CDA, FLAC, M4A/MP4/M4V, MID/MIDI, MPC, MP2/MP3, OGG, OPUS, WAV, WMA/ASF, WV\0" \
                      L"*.AAC;*.AC3;*.AIFF;*.AIF;*.APE;*.CDA;*.FLAC;*.MID;*.MIDI;*.MPC;*.MP3;*.MP2;*.OPUS;*.WAV;*.ASF;*.WMA;*.WV;*.OGG;*.OGA;*.MP4;*.M4A;*.M4V;*.3GP;\0" \
                      L"Supported Playlist Files (M3U/M3U8, CUE, PLS)\0*.M3U;*.M3U8;*.CUE;*.PLS\0All Files (*.*)\0*.*\0\0";
    ofn.lpstrFile = wsSongList;
    ofn.nMaxFile = MAX_SONG_PATH_CAPACITY;
    ofn.lpstrTitle = L"Select audio file(s)";
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY;
    if (GetOpenFileName(&ofn))
    {
        nIndex = 0;
        do
        {
            wsStringPart = (LPWSTR)(&(wsSongList[nIndex]));
            nLength = lstrlen(wsStringPart);
            if (nLength <= 0) break;
            if (nIndex == 0) // First iteration
            {
                dwAttr = GetFileAttributes(wsStringPart);
                
                // First time. It could be a folder (multiple selection) or a single file (single selection)
                if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                {
                    StringCchPrintf(wsDir, MAX_CHAR_PATH, L"%s", wsStringPart);
                }
                else
                {
                    _lpPlaylist->Add(wsStringPart);
                    btAtLeastOneSongAdded = 1;
                }
            }
            else
            {
                StringCchPrintf(wsSong, MAX_CHAR_PATH, L"%s\\%s", wsDir, wsStringPart);
                _lpPlaylist->Add(wsSong);
                btAtLeastOneSongAdded = 1;
            }
            nIndex += nLength + 1;
        } while (nLength);
        if (btAtLeastOneSongAdded)
        {
            _lpPlaylist->StartQueueProcessing();
            DragAcceptFiles(_hWnd, FALSE);
        }
    }
    return;
}

void DEWWINDOW::_BrowseAndSetFolder()
{
    IFileDialog     *pIFD = NULL;
    DWORD           dwOptions;
    PWSTR           pszFilePath = NULL;
    wchar_t         wsDir[MAX_CHAR_PATH] { 0 };

    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIFD))))
    {
        if (SUCCEEDED(pIFD->GetOptions(&dwOptions)))
        {
            pIFD->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        if (SUCCEEDED(pIFD->Show(NULL)))
        {
            IShellItem *pISI;
            if (SUCCEEDED(pIFD->GetResult(&pISI)))
            {
                if(SUCCEEDED(pISI->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                {
                    CopyMemory(wsDir, pszFilePath, lstrlen(pszFilePath) * sizeof(wchar_t));
                    CoTaskMemFree(pszFilePath);
                }
                pISI->Release();
            }
        }
        pIFD->Release();
    }

    if (lstrlen(wsDir))
    {
        if (wsDir[lstrlen(wsDir) - 1] == L'\\')
            wsDir[lstrlen(wsDir) - 1] = L'\0';
        _lpPlaylist->Add(wsDir);
        _lpPlaylist->StartQueueProcessing();
        DragAcceptFiles(_hWnd, FALSE);
    }

    return;
}

void DEWWINDOW::_BrowseAndSetDisc(LPCWSTR wsDiscPath)
{
    wchar_t         wsFilter[16] { 0 };
    wchar_t         wsFilePath[MAX_CHAR_PATH] { 0 };
    WIN32_FIND_DATA wfd { 0 };
    HANDLE          hFind = NULL;

    if (!wsDiscPath || _lpPlaylist->QueuingInProgress) return;

    StringCchPrintf(wsFilter, 16, L"%s\\*.cda", wsDiscPath);
    hFind = FindFirstFile(wsFilter, &wfd);
    if (hFind == NULL || hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        StringCchPrintf(wsFilePath, MAX_CHAR_PATH, L"%s\\%s", wsDiscPath, wfd.cFileName);
        _lpPlaylist->Add(wsFilePath);

    } while (FindNextFile(hFind, &wfd));
    _lpPlaylist->StartQueueProcessing();
    DragAcceptFiles(_hWnd, FALSE);
    return;
}

void DEWWINDOW::_ShowAboutScreen()
{
#ifndef DEW_THEMER_MODE
    _PlayerSettings.FocusWindowHandle = _lpAboutWindow->Show(_PlayerSettings.Theme);
#endif
    return;
}

LRESULT DEWWINDOW::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC             hDC;
    PAINTSTRUCT     ps { 0 };
    LRESULT         lrHit;
    POINT           ptCur, ptSysMenu;
    wchar_t         wsPath[MAX_CHAR_PATH] { 0 };
    UINT            nFileCount, nIndex;
    LPWINDOWPOS     lpPos;
    RECT            rctWorkArea;
#ifndef DEW_THEMER_MODE
    int             iObjWidth, iObjHeight, iAdvFactor;
#endif

    // Generally speaking in 'Theme' mode, the main window will be disabled
    // We'll talk to it via Handle messages. So, user generated interaction
    // messages will never generate
    switch (nMsg)
    {
        case WM_NCHITTEST:
        {
            if (!_PlayerSettings.DewMode) // Applies in normal mode only
            {
                ptCur.x = LOWORD(lParam);
                ptCur.y = HIWORD(lParam);
                MapWindowPoints(HWND_DESKTOP, hWnd, &ptCur, 1);
                lrHit = DefWindowProc(hWnd, nMsg, wParam, lParam);
                if (ptCur.x >= _rctTitleBar.left && ptCur.x <= _rctTitleBar.right &&
                    ptCur.y >= _rctTitleBar.top && ptCur.y <= _rctTitleBar.bottom &&
                    lrHit == HTCLIENT)
                    lrHit = HTCAPTION;
                return lrHit;
            }
            break;
        }

        // Click anywhere other than controls and the active object id is reset
        case WM_NCLBUTTONDOWN:
        case WM_LBUTTONDOWN:
        {
            if (_bLayoutMode)
            {
                SendMessage(_hWnd, WM_DEWMSG_LAYOUT_OBJ_ACTIVATED, DEWUI_OBJ_NONE, 0);
            }
            else if (nMsg == WM_NCLBUTTONDOWN)
            {
                GetCursorPos(&ptCur);
                CopyMemory(&ptSysMenu, &ptCur, sizeof(POINT));

                MapWindowPoints(HWND_DESKTOP, _hWnd, &ptSysMenu, 1);
                if (ptSysMenu.x >= 0 && ptSysMenu.x <= _rctTitleBar.bottom &&
                    ptSysMenu.y >= 0 && ptSysMenu.y <= _rctTitleBar.bottom)
                {
                    GetWindowRect(_hWnd, &rctWorkArea);
                    TrackPopupMenuEx(_hMnuSystem, TPM_RIGHTBUTTON,
                                     rctWorkArea.left, rctWorkArea.top + _rctTitleBar.bottom, _hWnd, NULL);
                }
            }
            break;
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

        case WM_DROPFILES:
        {
            // Do not accept anything in DewMode
            if (!_PlayerSettings.DewMode)
            {
                nFileCount = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
                if (!nFileCount)
                {
                    DragFinish((HDROP)wParam);
                    return TRUE;
                }
                // Ensure that anything dragged & dropped does start playing immediately
                if (_lpMapContent)
                    _lpMapContent->btCmdlineOpt = DEWUI_CMDLINE_OPT_ENQUEUE;
                for (nIndex = 0; nIndex < nFileCount; nIndex++)
                {
                    ZeroMemory(wsPath, MAX_CHAR_PATH * sizeof(wchar_t));
                    DragQueryFile((HDROP)wParam, nIndex, wsPath, MAX_PATH);
                    _lpPlaylist->Add(wsPath);
                }
                _lpPlaylist->StartQueueProcessing();
                DragFinish((HDROP)wParam);
                // Wait until the asyncronous listing is complete. Re-enable in the event
                DragAcceptFiles(_hWnd, FALSE);
                return TRUE;
            }
            break;
        }

        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
            return FALSE;
        }

    #ifndef DEW_THEMER_MODE
        
        case WM_HOTKEY:
        {
            switch (HIWORD(lParam))
            {
                case VK_MEDIA_PLAY_PAUSE:
                    this->_EA_HandlePlayPause(TRUE);
                    break;
                case VK_MEDIA_STOP:
                    this->_EA_HandleStop();
                    break;
                case VK_MEDIA_PREV_TRACK:
                    this->_EA_HandlePrevious(TRUE);
                    break;
                case VK_MEDIA_NEXT_TRACK:
                    this->_EA_HandleNext(TRUE);
                    break;
            }
            return FALSE;
        }

        case WM_CONTEXTMENU:
        {
            GetCursorPos(&ptCur);
            this->_ContextMenu(ptCur.x, ptCur.y);
            return FALSE;
        }

        case WM_SYSCOMMAND:
        {
            if (_PlayerSettings.DewMode)
                return FALSE; // Stop the sys menu from displaying when in 'Dew Mode'
            break;
        }

        case WM_SIZE:
        {
            if (wParam == SIZE_MINIMIZED)
            {
                // Dew and Layout modes won't minimize to tray
                if (!_bLayoutMode && !_PlayerSettings.DewMode)
                {
                    if (_PlayerSettings.MinimizeAction == DEWOPT_MINIMIZE_TRAY)
                    {
                        this->_HideToTray();
                    }
                    break;
                }
                return FALSE;
            }
            break;
        }

        // Applies to all modes (Normal/Layout/Dew)
        case WM_COMMAND:
        {
            // Menu Commands
            if (lParam == 0)
            {
                switch(LOWORD(wParam))
                {

                    #pragma region SYSCOMMANDS
                    // These are possible SYSCOMMANDS. If they come through
                    // via manual mode, bounce them back with WM_SYSCOMMAND msg
                    // paired with their their respective SC codes
                    case SC_MOVE:
                    case SC_MINIMIZE:
                    case SC_CLOSE:
                    {
                        GetCursorPos(&ptCur);
                        SendMessage(_hWnd, WM_SYSCOMMAND, LOWORD(wParam),
                                    MAKELPARAM(ptCur.x, ptCur.y));
                        break;
                    }
                    #pragma endregion

                    #pragma region BROWSE/OPEN MENU COMMANDS
                    case IDM_OPEN_FILE:
                    {
                        this->_BrowseAndSetFiles();
                        return FALSE;
                    }
                    case IDM_OPEN_DIR:
                    {
                        this->_BrowseAndSetFolder();
                        return FALSE;
                    }
                    case IDM_OPEN_DISC:
                    {
                        _PlayerSettings.FocusWindowHandle = _lpDiscSelector->Show(_PlayerSettings.Theme);
                        return FALSE;
                    }
                    #pragma endregion

                    #pragma region TRAY MENU COMMANDS
                    case IDM_TRAY_PLAYPAUSE:
                    {
                        this->_EA_HandlePlayPause(TRUE);
                        break;
                    }
                    case IDM_TRAY_STOP:
                    {
                        this->_EA_HandleStop();
                        break;
                    }
                    case IDM_TRAY_PREV:
                    {
                        this->_EA_HandlePrevious(TRUE);
                        break;
                    }
                    case IDM_TRAY_NEXT:
                    {
                        this->_EA_HandleNext(TRUE);
                        break;
                    }
                    case IDM_TRAY_RESTORE:
                    {
                        this->_RestoreFromTray();
                        break;
                    }
                    case IDM_TRAY_QUIT:
                    {
                        Shell_NotifyIcon(NIM_DELETE, &_nidTray);
                        SendMessage(_hWnd, WM_CLOSE, 1, 0);
                        break;
                    }
                    #pragma endregion

                    #pragma region CONTEXT MENU COMMANDS
                    case IDM_CTX_LAYOUT:
                    {
                        this->_SetLayoutMode();
                        return FALSE;
                    }
                    case IDM_CTX_DEWMODE:
                    {
                        this->_SetDewMode(!_PlayerSettings.DewMode);
                        return FALSE;
                    }
                    case IDM_CTX_ABOUT:
                    {
                        this->_ShowAboutScreen();
                        return FALSE;
                    }
                    case IDM_CTX_RESET_LAYOUT:
                    {
                        this->_ResetLayout();
                        return FALSE;
                    }
                    case IDM_CTX_HIDE:
                    {
                        // Hide All
                        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++)
                        {
                            _ducComponents[nIndex]._lpObject->SetVisible(FALSE);
                            _lpbLayoutVisibility[nIndex] = FALSE;
                        }
                        _bShowTitle = FALSE;
                        this->_Draw(NULL);
                        return FALSE;
                    }
                    case IDM_CTX_HIDE_TITLE:
                    {
                        _bShowTitle = FALSE;
                        this->_Draw(NULL);
                        return FALSE;
                    }
                    case IDM_CTX_SHOW:
                    {
                        // Show All
                        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 2; nIndex++)
                        {
                            _ducComponents[nIndex]._lpObject->SetVisible();
                            _lpbLayoutVisibility[nIndex] = TRUE;
                        }
                        _bShowTitle = TRUE;
                        this->_Draw(NULL);
                        return FALSE;
                    }
                    case IDM_CTX_SHOW_TITLE:
                    {
                        _bShowTitle = TRUE;
                        this->_Draw(NULL);
                        return FALSE;
                    }
                    #pragma endregion

                    #pragma region HOTKEY MENU COMMANDS

                    // <UP>/<DOWN>/<LEFT>/<RIGHT> -
                    // Move around control components in layout mode
                    // NOTE: Do NOT add the DPI Scale factor to iAdvFactor
                    //       i.e. do npot change to 'iAdvFactor = _F(1)'.
                    //       This is deliberate as we want a pixel-level
                    //       precision in movement of each component
                    // ** If not in their satisfying citeria/modes,
                    //    then pass the keystroke to the playlist
                    //    window's listview control (if visible) **
                    case IDM_HK_LAYOUT_UP:
                    {

                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = 1;
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y - iAdvFactor;
                            if (ptCur.y < _F(1)) ptCur.y = _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_UP, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_DOWN:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = 1;
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y + iAdvFactor;
                            iObjHeight = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Height;
                            if (ptCur.y + iObjHeight > _F(DEWUI_DIM_WINDOW_Y_NOPL) - _F(1)) ptCur.y = _F(DEWUI_DIM_WINDOW_Y_NOPL) - iObjHeight - _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_DOWN, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_LEFT:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = 1;
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x - iAdvFactor;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y;
                            if (ptCur.x < _F(1)) ptCur.x = _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_LEFT, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_RIGHT:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = 1;
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x + iAdvFactor;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y;
                            iObjWidth = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Width;
                            if (ptCur.x + iObjWidth > _F(DEWUI_DIM_WINDOW_X) - _F(1)) ptCur.x = _F(DEWUI_DIM_WINDOW_X) - iObjWidth - _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);

                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_RIGHT, 0);
                        }
                        return FALSE;
                    }

                    // <UP>/<DOWN>/<LEFT>/<RIGHT> -
                    // Move around control components in layout mode (bigger move jumps)
                    // ** If not in their satisfying citeria/modes,
                    //    then pass the keystroke to the playlist
                    //    window's listview control (if visible) **
                    case IDM_HK_LAYOUT_ACCEL_UP:
                    {

                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = _F(10);
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y - iAdvFactor;
                            if (ptCur.y < _F(1)) ptCur.y = _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_UP, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_ACCEL_DOWN:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = _F(10);
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y + iAdvFactor;
                            iObjHeight = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Height;
                            if (ptCur.y + iObjHeight > _F(DEWUI_DIM_WINDOW_Y_NOPL) - _F(1)) ptCur.y = _F(DEWUI_DIM_WINDOW_Y_NOPL) - iObjHeight - _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_DOWN, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_ACCEL_LEFT:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = _F(10);
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x - iAdvFactor;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y;
                            if (ptCur.x < _F(1)) ptCur.x = _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_LEFT, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_ACCEL_RIGHT:
                    {
                        if (_bLayoutMode && _btActiveLayoutObjectID > DEWUI_OBJ_NONE &&
                            _btActiveLayoutObjectID <= DEWUI_OBJECT_COUNT)
                        {
                            iAdvFactor = _F(10);
                            ptCur.x = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.x + iAdvFactor;
                            ptCur.y = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Position.y;
                            iObjWidth = _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Width;
                            if (ptCur.x + iObjWidth > _F(DEWUI_DIM_WINDOW_X) - _F(1)) ptCur.x = _F(DEWUI_DIM_WINDOW_X) - iObjWidth - _F(1);
                            _ducComponents[_btActiveLayoutObjectID - 1]._lpObject->Move(ptCur.x, ptCur.y);
                            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);

                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_RIGHT, 0);
                        }
                        return FALSE;
                    }

                    // <CTRL> + <UP>/<DOWN>/<LEFT>/<RIGHT> -
                    // Snap to preset screen positions (Dew Mode only)
                    // ** If not in their satisfying citeria/modes,
                    //    then pass the keystroke to the playlist
                    //    window's listview control (if visible) **
                    case IDM_HK_DEWSNAP_UP:
                    {
                        if (_PlayerSettings.DewMode && !_bLayoutMode)
                        {
                            this->_SnapDewModeWindow((BYTE)VK_UP);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_UP, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_DEWSNAP_DOWN:
                    {
                        if (_PlayerSettings.DewMode && !_bLayoutMode)
                        {
                            this->_SnapDewModeWindow((BYTE)VK_DOWN);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_DOWN, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_DEWSNAP_LEFT:
                    {
                        if (_PlayerSettings.DewMode && !_bLayoutMode)
                        {
                            this->_SnapDewModeWindow((BYTE)VK_LEFT);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_LEFT, 0);
                        }
                        return FALSE;
                    }
                    case IDM_HK_DEWSNAP_RIGHT:
                    {
                        if (_PlayerSettings.DewMode && !_bLayoutMode)
                        {
                            this->_SnapDewModeWindow((BYTE)VK_RIGHT);
                        }
                        else // Pass it on to the playlist window's ListView control
                        {
                            SendMessage(_lpPlaylist->Handle, WM_KEYDOWN, VK_RIGHT, 0);
                        }
                        return FALSE;
                    }

                    case IDM_HK_DEWMODE_TOGGLE: // <CTRL> + <SHIFT> + D: Toggle Dew Mode
                    {
                        if (!_bLayoutMode)
                            this->_SetDewMode(!_PlayerSettings.DewMode);
                        return FALSE;
                    }
                    case IDM_HK_LAYOUT_TOGGLE: // <CTRL> + <SHIFT> + L - Toggle Layout Mode
                    {
                        if (!_PlayerSettings.DewMode)
                            this->_SetLayoutMode();
                        return FALSE;
                    }
                    case IDM_HK_REPEAT_CYCLE: // <CTRL> + R - Cycle through repeat modes
                    {
                        if (!_bLayoutMode)
                        {
                            _lpCmdRepeat->SetNextState();
                            _PlayerSettings.RepeatLevel = _lpCmdRepeat->State;
                        }
                        return FALSE;
                    }
                    case IDM_HK_PLAYLIST_TOGGLE: // <CTRL> +P - Toggle playlist visibility
                    {
                        if (!_PlayerSettings.DewMode && !_bLayoutMode)
                            this->_ShowFullView(!(this->_PlayerSettings.PlaylistVisible == 1));
                        return FALSE;
                    }
                    case IDM_HK_OPEN_FILE_DLG: // <CTRL> + O - Open File(s)
                    {
                        if (!_bLayoutMode)
                            this->_BrowseAndSetFiles();
                        return FALSE;
                    }
                    case IDM_HK_OPEN_DIR_DLG: // <CTRL> + <SHIFT> + O - Open Folder
                    {
                        if (!_bLayoutMode)
                            this->_BrowseAndSetFolder();
                        return FALSE;
                    }
                    case IDM_HK_EXINFO: // <CTRL> + J - File/Tag/Codec Info
                    {
                        if (!_bLayoutMode)
                            _PlayerSettings.FocusWindowHandle = _lpPropPage->Show(_PlayerSettings.Theme);
                        return FALSE;
                    }
                    case IDM_HK_TIMER: // <CTRL> + T - Shutdown timer
                    {
                        if (!_bLayoutMode)
                            _PlayerSettings.FocusWindowHandle = _lpShutdownTimer->Show(_PlayerSettings.Theme);
                        return FALSE;
                    }
                    case IDM_HK_SETTINGS: // <CTRL> + X - Settings window
                    {
                        if (!_bLayoutMode)
                            _PlayerSettings.FocusWindowHandle = _PlayerSettings.ShowSettings(_hWnd);
                        return FALSE;
                    }
                    case IDM_HK_ABOUT: // <F1> - About Screen
                    {
                        this->_ShowAboutScreen();
                        return FALSE;
                    }
                    case IDM_HK_CONTEXT_MENU: // <MENU KEY> - Context Menu
                    {
                        GetCursorPos(&ptCur);
                        GetWindowRect(_hWnd, &rctWorkArea);
                        if (ptCur.x >= rctWorkArea.left && ptCur.x <= rctWorkArea.right &&
                            ptCur.y >= rctWorkArea.top && ptCur.y <= rctWorkArea.bottom)
                            this->_ContextMenu(ptCur.x, ptCur.y);
                        else
                            this->_ContextMenu(rctWorkArea.left, rctWorkArea.top + _F(DEWUI_DIM_PADDING_Y_HEADER));
                        return FALSE;
                    }
                    case IDM_HK_PLAYLIST_SEL_ALL: // <CTRL> + A - Select all Playlist items
                    {
                        if (!_PlayerSettings.DewMode && !_bLayoutMode && _lpPlaylist)
                            _lpPlaylist->SelectAllItems();
                        return FALSE;
                    }
                    case IDM_HK_PLAYLIST_EXPORT_SEL: // <CTRL> + S - 'Export Selected'
                    {
                        if (_lpPlaylist->GetSelectedCount() > 0 && // At least one item is selected
                            !_PlayerSettings.DewMode && !_bLayoutMode)
                            this->ProcessMessage(_lpPlaylist, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)FALSE); 
                        return FALSE;
                    }
                    case IDM_HK_PLAYLIST_EXPORT_ALL: // <CTRL> + <SHIFT> + S - 'Export All'
                    {
                        if (_lpPlaylist->GetFileCount() > 0 && // At least one item is present in the playlist
                            !_PlayerSettings.DewMode && !_bLayoutMode)
                            this->ProcessMessage(_lpPlaylist, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)TRUE);
                        return FALSE;
                    }
                    
                    #pragma endregion

                }

                if (LOWORD(wParam) > IDM_CTX_HIDE && LOWORD(wParam) < (IDM_CTX_HIDE + DEWUI_OBJECT_COUNT))
                {
                    _ducComponents[LOWORD(wParam) - IDM_CTX_HIDE - 1]._lpObject->SetVisible(FALSE);
                    _lpbLayoutVisibility[LOWORD(wParam) - IDM_CTX_HIDE - 1] = FALSE;
                }
                else if (LOWORD(wParam) > IDM_CTX_SHOW && LOWORD(wParam) < (IDM_CTX_SHOW + DEWUI_OBJECT_COUNT))
                {
                    _ducComponents[LOWORD(wParam) - IDM_CTX_SHOW - 1]._lpObject->SetVisible();
                    _lpbLayoutVisibility[LOWORD(wParam) - IDM_CTX_SHOW - 1] = TRUE;
                }

                return FALSE;
            }
            break;
        }

        case WM_DEWMSG_FLUSH_PLAYLIST:
        {
            if (_lpPlaylist)
                _lpPlaylist->RemoveAllItems();
            return FALSE;
        }

        // Song enqueuing through shared memory will not apply in 'Dew Mode'
        case WM_DEWMSG_ENQUEUE:
        {
            if (_lpPlaylist)
            {
                // Do not accept anything if playlist is already enqueuing
                if (!(_lpPlaylist->QueuingInProgress))
                {
                    if (this->_GetSharedMemFileName(wsPath))
                        _lpPlaylist->Add(wsPath);
                    // It is the sender's responsibility to send AT LEAST
                    // one message with wParam = 0. Otherwise queuing will never kick-start!
                    if (wParam == DEWUI_ENQUEUE_SEQ_LAST)
                    {
                        _lpPlaylist->StartQueueProcessing();
                        DragAcceptFiles(_hWnd, FALSE);
                    }
                }
            }
            return FALSE; // Doesn't really matter
        }

        // Playback complete. Move to the next song.
        case WM_DEWMSG_PLAYBACK_COMPLETE:
        {
            this->_EA_HandleNext(FALSE); // Automatic (Non-manual) mode
            return FALSE;
        }
    
    #endif
        case WM_DEWMSG_TRAY:
        {
            if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
            {
                this->_RestoreFromTray();
            }
            if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_LBUTTONUP)
            {
                GetCursorPos(&ptCur);
                this->_TrayMenu(ptCur.x, ptCur.y);
            }
            return TRUE;
        }

        case WM_DEWMSG_STYLE_CHANGED:
        {
            this->ApplyTheme(_PlayerSettings.Theme);
            return FALSE;
        }

        case WM_DEWMSG_CHILD_CLOSED:
        {
            // A child window has closed. Set the focus back to this window
            _PlayerSettings.FocusWindowHandle = _hWnd;
            EnableWindow(_hWnd, TRUE);
            return FALSE;
        }

        case WM_DEWMSG_CHILD_POS_CHANGED:
        {
            _lpdwLayoutPos[wParam-1] = lParam;
            _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
            return FALSE;
        }

        case WM_DEWMSG_LAYOUT_OBJ_ACTIVATED:
        {
            // Check Valid Object ID
            if (wParam > DEWUI_OBJ_NONE && wParam <= DEWUI_OBJECT_COUNT)
                _btActiveLayoutObjectID = (BYTE)wParam;
            else
                _btActiveLayoutObjectID = DEWUI_OBJ_NONE;
            // Sent selection code to layout viewer
            // Ensure to check that viewer did not send this, to being with
            // or else it will cause an infinite loop of messages
            if (lParam != 0x00)
                _lpLayoutViewer->UpdateComponentLocations(_btActiveLayoutObjectID);
            return FALSE;
        }

        case WM_DEWMSG_COUNTDOWN_OVER:
        {
            SendMessage(_hWnd, WM_CLOSE, 1, 0);
            return FALSE;
        }

        case WM_DEWMSG_DISC_SEL:
        {
            this->_BrowseAndSetDisc((LPCWSTR)lParam);
            return FALSE;
        }

        case WM_CLOSE:
        {
            if (wParam == 1 || _PlayerSettings.CloseAction == DEWOPT_CLOSE_QUIT)
            {
                if (_bLayoutMode)
                {
                    nIndex = MessageBox(_hWnd, L"You have an unsaved layout in progress.\nQuitting now " \
                                        L"will revert all the changes to their previous states.\n\n" \
                                        L"Are you sure you want to quit?", L"Confirm Quit",
                                        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
                    if (nIndex == IDNO)
                        return FALSE;
                }
            #ifndef DEW_THEMER_MODE
                this->_EA_HandleStop();
            #endif
                PostQuitMessage(0);
                return TRUE;
            }
            else if (_PlayerSettings.CloseAction == DEWOPT_CLOSE_TRAY && !_bLayoutMode)
            {
                this->_HideToTray();
                return FALSE;
            }
            return FALSE;
        }

        case WM_QUIT:
        {
            if (!_bLayoutMode)
                return TRUE;
            return FALSE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWWINDOW::_ApplySettings(const DEWSETTINGS& AppSettings)
{
#ifndef DEW_THEMER_MODE
    this->_b_EP_DeepScan = (AppSettings.DeepScan == 1);
#endif

    this->_bMinToTray = (AppSettings.MinimizeAction == DEWOPT_MINIMIZE_TRAY);
    this->_bCloseToTray = (AppSettings.MinimizeAction == DEWOPT_CLOSE_TRAY);
    this->ApplyTheme(AppSettings.Theme, FALSE);
    if (_lpCmdVolume)
        _lpCmdVolume->SetVolumeLevel(AppSettings.VolumeLevel);

    return;
}

void DEWWINDOW::_Scale(BYTE btIconScale) {} // NOP

void DEWWINDOW::_PrepareImages() {} // NOP

void DEWWINDOW::_MoveToDefaultLocation() {} // NOP

#ifndef DEW_THEMER_MODE
DEWWINDOW::DEWWINDOW(DEWSETTINGS& AppSettings, BOOL bPrepPopulateOverride) :
_PlayerSettings(AppSettings), AccelHotKey(_hAccelHK)
#else
DEWWINDOW::DEWWINDOW(DEWSETTINGS& AppSettings, HWND hWndContainer) : _PlayerSettings(AppSettings)
#endif
{
    WNDCLASSEX      wcex { 0 };
    int             iHeight;
    const wchar_t   *wsWindowClass = L"DEWDROP.MAIN.WND";
    BYTE            btIconScale;
    UINT            nIndex = 0;
    MENUITEMINFO    miiSys { 0 };
#ifdef DEW_THEMER_MODE // Simulate no files in 'Theme' mode. Playlist will populate some dummy data
    BOOL bPrepPopulateOverride = FALSE;
#else
    int             iScreenWidth, iScreenHeight;

    // Create the Accelerator Table for the hotkeys
    // Create the Accelerator table
    ACCEL       acclUI[28] = {
                               // <UP/DOWN/LEFT/RIGHT>:
                               // Moves around controls (Layout Mode)
                               {                     FVIRTKEY, VK_UP,   IDM_HK_LAYOUT_UP },
                               {                     FVIRTKEY, VK_DOWN, IDM_HK_LAYOUT_DOWN },
                               {                     FVIRTKEY, VK_LEFT, IDM_HK_LAYOUT_LEFT },
                               {                     FVIRTKEY, VK_RIGHT, IDM_HK_LAYOUT_RIGHT },
                               
                               // <SHIFT> + <UP/DOWN/LEFT/RIGHT>:
                               // Move around controls faster (Layout Mode)
                               {            FSHIFT | FVIRTKEY, VK_UP,   IDM_HK_LAYOUT_ACCEL_UP },
                               {            FSHIFT | FVIRTKEY, VK_DOWN, IDM_HK_LAYOUT_ACCEL_DOWN },
                               {            FSHIFT | FVIRTKEY, VK_LEFT, IDM_HK_LAYOUT_ACCEL_LEFT },
                               {            FSHIFT | FVIRTKEY, VK_RIGHT, IDM_HK_LAYOUT_ACCEL_RIGHT },
                               
                               // <CTRL> + <UP/DOWN/LEFT/RIGHT>:
                               // Snaps the Player Window to screen edges (Dew Mode)
                               { FCONTROL |          FVIRTKEY, VK_UP,   IDM_HK_DEWSNAP_UP },
                               { FCONTROL |          FVIRTKEY, VK_DOWN, IDM_HK_DEWSNAP_DOWN },
                               { FCONTROL |          FVIRTKEY, VK_LEFT, IDM_HK_DEWSNAP_LEFT },
                               { FCONTROL |          FVIRTKEY, VK_RIGHT, IDM_HK_DEWSNAP_RIGHT },
                               
                               { FCONTROL | FSHIFT | FVIRTKEY, VK_D,    IDM_HK_DEWMODE_TOGGLE }, // <CTRL> + <SHIFT> + D - Toggle Dew Mode
                               { FCONTROL | FSHIFT | FVIRTKEY, VK_L,    IDM_HK_LAYOUT_TOGGLE }, // <CTRL> + <SHIFT> + L - Toggle Layout Mode
                               
                               { FCONTROL |          FVIRTKEY, VK_R,    IDM_HK_REPEAT_CYCLE }, // <CTRL> + R - Cycle Repeat Modes
                               { FCONTROL |          FVIRTKEY, VK_P,    IDM_HK_PLAYLIST_TOGGLE }, // <CTRL> + P - Toggle Playlist visibility
                               { FCONTROL |          FVIRTKEY, VK_O,    IDM_HK_OPEN_FILE_DLG }, // <CTRL> + O - Browse & add file(s)
                               { FCONTROL | FSHIFT | FVIRTKEY, VK_O,    IDM_HK_OPEN_DIR_DLG }, // <CTRL> + <SHIFT> + O - Browse & add a folder
                               { FCONTROL |          FVIRTKEY, VK_J,    IDM_HK_EXINFO }, // <CTRL> + J - Display File/Tag/Codec Info (Thanks, VLC!)
                               { FCONTROL |          FVIRTKEY, VK_T,    IDM_HK_TIMER }, // <CTRL> + T - Open auto-shutdown timer window
                               { FCONTROL |          FVIRTKEY, VK_X,    IDM_HK_SETTINGS }, // <CTRL> + X - Open settings window
                               {                     FVIRTKEY, VK_F1,   IDM_HK_ABOUT }, // <F1> - About screen (Please do visit. A lot of good folks listed there!)
                               
                               {                     FVIRTKEY, VK_APPS, IDM_HK_CONTEXT_MENU }, // Keyboard Menu Key - Display context menu
                               {            FSHIFT | FVIRTKEY, VK_F10,  IDM_HK_CONTEXT_MENU }, // <SHIFT> + <F10> - Display context menu (same as above)
                               
                               { FCONTROL |          FVIRTKEY, VK_A,    IDM_HK_PLAYLIST_SEL_ALL }, // <CTRL> + A - (Playlist) Select all items
                               { FCONTROL |          FVIRTKEY, VK_S,    IDM_HK_PLAYLIST_EXPORT_SEL }, // <CTRL> + S - (Playlist) Export selected items
                               { FCONTROL | FSHIFT | FVIRTKEY, VK_S,    IDM_HK_PLAYLIST_EXPORT_ALL }, // <CTRL> + <SHIFT> + S - (Playlist) Export all items
                               
                               { FCONTROL |          FVIRTKEY, VK_Q,    IDM_TRAY_QUIT } }; // <CTRL> + Q - Quit (Thanks, VLC!)

    _hAccelHK = CreateAcceleratorTable(acclUI, 28);

#endif

    _btObjectID = DEWUI_OBJ_WINDOW;
    _btObjectType = DEWUI_OBJ_TYPE_WINDOW;
    _Parent = NULL;
    _iDPI = _PlayerSettings.Theme.DPI;
    _fScale = _PlayerSettings.Theme.Scale;
    ZeroMemory(_ducComponents, DEWUI_OBJECT_COUNT * sizeof(DEWUICOMPONENT));

    btIconScale = _PlayerSettings.Theme.IconScale;

    if (!(btIconScale == DEWTHEME_ICON_SCALE_SMALL ||
          btIconScale == DEWTHEME_ICON_SCALE_MEDIUM ||
          btIconScale == DEWTHEME_ICON_SCALE_LARGE))
        btIconScale = DEWTHEME_ICON_SCALE_MEDIUM;

    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[btIconScale];

    _iWidth = _F(DEWUI_DIM_WINDOW_X);
    _iTrimmedHeight = _F(DEWUI_DIM_WINDOW_Y_NOPL);
    _iHeight = _F(DEWUI_DIM_WINDOW_Y);
    _iDewModeWidth = _iWidth;
    _iDewModeHeight = _F(DEWUI_DIM_WINDOW_Y_DEWMODE);
    _rctTitleBar.left = 0;
    _rctTitleBar.top = 0;
    _rctTitleBar.right = _iWidth;
    _rctTitleBar.bottom = (int)((float)DEWUI_DIM_PADDING_Y_HEADER * _fScale * _fIconScale);

    ZeroMemory(&_nidTray, sizeof(NOTIFYICONDATA));
    #ifndef DEW_THEMER_MODE
    _hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_DEWAPP));
    #endif
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_CAP_DEFAULT);
    this->_ApplySettings(_PlayerSettings);

#ifndef DEW_THEMER_MODE
    iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    iHeight = ((_PlayerSettings.PlaylistVisible == 1) ? _iHeight : _iTrimmedHeight);
    _ptPos.x = (iScreenWidth - _iWidth)/2;
    _ptPos.y = (iScreenHeight -  iHeight)/2;
#else
    _ptPos.x = 0;
    _ptPos.y = 0;
    iHeight = _iHeight;
#endif

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = _hIcon;
    wcex.hIconSm        = _hIcon;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsWindowClass;

    RegisterClassEx(&wcex);
#ifndef DEW_THEMER_MODE
    _hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_ACCEPTFILES, wsWindowClass, _wsText,
                           WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX, _ptPos.x, _ptPos.y,
                           _iWidth, iHeight, NULL, NULL, _hInstance, NULL);
    SetLayeredWindowAttributes(_hWnd, 0x00, 0xFF, LWA_ALPHA);
#else
    _hWnd = CreateWindowEx(0, wsWindowClass, _wsText,
                           WS_CHILD, _ptPos.x, _ptPos.y,
                           _iWidth, iHeight, hWndContainer, NULL, _hInstance, NULL);
#endif

    _nidTray.cbSize = sizeof(NOTIFYICONDATA);
    _nidTray.uCallbackMessage = WM_DEWMSG_TRAY;
    _nidTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    _nidTray.uVersion = NOTIFYICON_VERSION;
    _nidTray.hWnd = _hWnd;

    this->_CreateUIComponents();
    if (_lpLayoutViewer) _lpLayoutViewer->SetComponentStack(_ducComponents, DEWUI_OBJECT_COUNT - 2);

#ifndef DEW_THEMER_MODE
    // Open the map for read/write operations
    _hMap = OpenFileMapping(GENERIC_READ | GENERIC_WRITE, FALSE, DEWUI_MAP);
    if (_hMap)
    {
        _lpMapContent = (LPMAPCONTENT)MapViewOfFile(_hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        _lpMapContent->hWnd = _hWnd;
    }
    // This CTOR is running means it is the first instance. So set this window
    // up, for receiving communication messages from other instances
    if (_lpMapContent) _lpMapContent->hWnd = _hWnd;
#else
    // Do not tinker with the shared memory in 'Theme' mode
    _hMap = NULL;
    _lpMapContent = NULL;
#endif

#ifndef DEW_THEMER_MODE
    // No Override and there's history
    if (!bPrepPopulateOverride && _PlayerSettings.HistoryFileCount && _PlayerSettings.HistoryFileCount > 0)
    {
        for (nIndex = 0; nIndex < _PlayerSettings.HistoryFileCount; nIndex++)
            _lpPlaylist->Add(_PlayerSettings.SongList[nIndex].wsFileName);
        _lpPlaylist->StartQueueProcessing();
        // Wait until the asyncronous listing is complete. Re-enable in the event
        DragAcceptFiles(_hWnd, FALSE);
    }
    else // Either there is override, or there is no history to be pre-populated
    {
        // In that case, we need to both set the event as well as set
        // it to NULL. This is because since there are no files, the asynchronous
        // call will never happen and so, the event will never be set to NULL.
        // As a result, any new instance of DEWSTARTUP will always encounter
        // a non-NULL event handle and will never send any file for queuing.
        if (_lpMapContent->hEvtStartEnqueue &&
            _lpMapContent->hEvtStartEnqueue != INVALID_HANDLE_VALUE)
        {
            SetEvent(_lpMapContent->hEvtStartEnqueue);
        }
        // There is no history as well as there are no args to
        // populate the playlist. Make it available for other app instances
        // for consumption
        if (!bPrepPopulateOverride)
        {
            _lpMapContent->hEvtStartEnqueue = NULL;
        }
    }

    if (!_PlayerSettings.SongList) LocalFree(_PlayerSettings.SongList);
    _PlayerSettings.SongList = NULL;
    _PlayerSettings.HistoryFileCount = 0x00;
    _PlayerSettings.FocusWindowHandle = _hWnd;

    // Do not register for hotkeys in 'Theme' mode
    // Register the multimedia hotkeys
    RegisterHotKey(_hWnd, 0, 0, VK_MEDIA_PLAY_PAUSE);
    RegisterHotKey(_hWnd, 0, 0, VK_MEDIA_STOP);
    RegisterHotKey(_hWnd, 0, 0, VK_MEDIA_PREV_TRACK);
    RegisterHotKey(_hWnd, 0, 0, VK_MEDIA_NEXT_TRACK);
#endif

// Dew Mode does not apply in 'Theme' mode
#ifndef DEW_THEMER_MODE
    // All Setup complete. Now check to see if dew mode was enabled. Start there accordingly.
    if (_PlayerSettings.DewMode)
        this->_SetDewMode(_PlayerSettings.DewMode);
    else
        _lpCmdDMPlayPause->SetVisible(FALSE);
#else
    _lpCmdDMPlayPause->SetVisible(FALSE);
#endif


#ifdef DEW_THEMER_MODE
    _lpSeekBar->SetLength(311000);
    _lpSeekBar->SetTime(79000);
#endif

    // Since we will manually provide the system menu, we need to disable the
    // 'Restore', 'Size' & 'Maximize' options
    _hMnuSystem = GetSystemMenu(_hWnd, FALSE);

    miiSys.cbSize = sizeof(MENUITEMINFO);
    miiSys.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE;

    // Maximize
    GetMenuItemInfo(_hMnuSystem, SC_RESTORE, FALSE, &miiSys);
    miiSys.fState = MFS_DISABLED;
    SetMenuItemInfo(_hMnuSystem, SC_RESTORE, FALSE, &miiSys);

    // Maximize
    GetMenuItemInfo(_hMnuSystem, SC_SIZE, FALSE, &miiSys);
    miiSys.fState = MFS_DISABLED;
    SetMenuItemInfo(_hMnuSystem, SC_SIZE, FALSE, &miiSys);

    // Maximize
    GetMenuItemInfo(_hMnuSystem, SC_MAXIMIZE, FALSE, &miiSys);
    miiSys.fState = MFS_DISABLED;
    SetMenuItemInfo(_hMnuSystem, SC_MAXIMIZE, FALSE, &miiSys);

    this->_InitiateSubclassing();

    return;
}

void DEWWINDOW::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle       gdiFontStyle;
    Graphics        *pGr = NULL;
    Graphics        *pGrIcon = NULL;
    IStream         *pstrBkgnd = NULL;
    Bitmap          *pImgBkgnd = NULL;
    RectF           rctIcon(0, 0, 0, 0);
    BOOL            bBkgndImg = FALSE;
    ULONG           dwRet = 0;
    LARGE_INTEGER   liPos { 0 };
    HRESULT         hrStreamOp = S_OK;
    HICON           hIcoApp = NULL;
    INT             iIconDim;
    BOOL            bCustIconApplied = FALSE;
    UINT            nIndex;

    _crBack.SetFromCOLORREF(Theme.WinStyle.BackColor);
    _crText.SetFromCOLORREF(Theme.WinStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.WinStyle.OutlineColor);
    _crBack2.SetFromCOLORREF(Theme.WinStyle.BackColor2);

    if (_pBrText) delete _pBrText;
    if (_pBrBack) delete _pBrBack;
    if (_pPenOutline) delete _pPenOutline;
    if (_pPenText) delete _pPenText;

    _pBrText = new SolidBrush(_crText);
    _pBrBack = new LinearGradientBrush(Point(_iWidth/2, 0), Point(_iWidth/2, _iTrimmedHeight), _crBack, _crBack2);
    _pPenOutline = new Pen(_crOutline, (REAL)_F(1));
    _pPenText = new Pen(_crText, (REAL)_F(1));

    CopyMemory(_wsFontName, Theme.TitleFontStyle.FontName, 32*sizeof(wchar_t));
    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[Theme.IconScale];
    _iFontSize = Theme.TitleFontStyle.FontSize;
    _bBold = Theme.TitleFontStyle.IsBold;
    _bItalic = Theme.TitleFontStyle.IsItalic;
    if (_bBold)
        gdiFontStyle = _bItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = _bItalic ? FontStyleItalic : FontStyleRegular;

    if (_pFont) delete _pFont;
    _pFont = new Font(_wsFontName, (REAL)_iFontSize, gdiFontStyle, UnitPoint);
    _fDimension = 0.0f;
    _bAlternateIconMode = (Theme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE);
    if (DEWUIOBJECT::_pImgBackground) { delete DEWUIOBJECT::_pImgBackground; DEWUIOBJECT::_pImgBackground = NULL; }

    DEWUIOBJECT::_pImgBackground = new Bitmap(_iWidth, _iTrimmedHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(DEWUIOBJECT::_pImgBackground);

    // Background Image
    if (Theme.BackgroundImageSize > 0x00)
    {
        pstrBkgnd = SHCreateMemStream(Theme.BackgroundImageData, Theme.BackgroundImageSize);
        if (pstrBkgnd)
        {
            hrStreamOp = pstrBkgnd->Write(Theme.BackgroundImageData, Theme.BackgroundImageSize, &dwRet);
            if (SUCCEEDED(hrStreamOp))
            {
                hrStreamOp = pstrBkgnd->Seek(liPos, STREAM_SEEK_SET, NULL);
                if (SUCCEEDED(hrStreamOp))
                {
                    pImgBkgnd = Bitmap::FromStream(pstrBkgnd);
                    if (pImgBkgnd)
                    {

                        pGr->DrawImage(pImgBkgnd, 0, 0, _iWidth, _iTrimmedHeight);
                        delete pImgBkgnd;
                        pImgBkgnd = NULL;
                        bBkgndImg = TRUE;
                    }
                }
            }
            pstrBkgnd->Release();
            pstrBkgnd = NULL;
        }
    }

    // Fallback: If the background image fails to get set
    // shade the background with the gradient
    if (!bBkgndImg)
        pGr->FillRectangle(_pBrBack, 0, 0, _iWidth, _iTrimmedHeight);

    // App Icon
    iIconDim = _F(DEWUI_DIM_PADDING_Y_HEADER);
    if (_pImgIcon) { delete _pImgIcon; _pImgIcon = NULL; }

    if (Theme.IconImageSize > 0x00)
    {
        _pImgIcon = new Bitmap(iIconDim, iIconDim, PixelFormat32bppARGB);
        // pstrBkgnd = SHCreateMemStream(Theme.BackgroundImageData, Theme.BackgroundImageSize);
        // CreateStreamOnHGlobal(NULL, TRUE, &pstrBkgnd);
        CreateStreamOnHGlobal(NULL, TRUE, &pstrBkgnd); // Change to SHCreateMemStream!
        if (pstrBkgnd)
        {
            hrStreamOp = pstrBkgnd->Write(Theme.IconImageData, Theme.IconImageSize, &dwRet);
            if (SUCCEEDED(hrStreamOp))
            {
                hrStreamOp = pstrBkgnd->Seek(liPos, STREAM_SEEK_SET, NULL);
                if (SUCCEEDED(hrStreamOp))
                {
                    pImgBkgnd = Bitmap::FromStream(pstrBkgnd);
                    if (pImgBkgnd)
                    {
                        hIcoApp = _hIcon; // Create a backup of the original icon
                        _hIcon = NULL;
                        // Now try to extract the icon out of the image
                        hrStreamOp = pImgBkgnd->GetHICON(&_hIcon);
                        // If the extraction succeeds, we don't need the older icon anymore
                    #ifndef DEW_THEMER_MODE
                        if (SUCCEEDED(hrStreamOp) && hIcoApp)
                    #else
                        if (SUCCEEDED(hrStreamOp))
                    #endif
                        {
                            pGrIcon = Graphics::FromImage(_pImgIcon);
                            // Ready the title image to be drawn
                            pGrIcon->DrawImage(pImgBkgnd, _F(4), _F(4),
                                               _F((DEWUI_DIM_PADDING_Y_HEADER-8)),
                                               _F((DEWUI_DIM_PADDING_Y_HEADER-8)) );
                            delete pGrIcon;
                            pGrIcon = NULL;
                            // And set the window icon as well
                            SetClassLongPtr(_hWnd, GCLP_HICON, (LONG)_hIcon);
                            SetClassLongPtr(_hWnd, GCLP_HICONSM, (LONG)_hIcon);
                            DestroyIcon(hIcoApp);
                            hIcoApp = NULL;
                            _ptIconStart.X = 0.0f; _ptIconStart.Y = 0.0f;
                            bCustIconApplied = TRUE;
                        }
                        else
                        {
                            // Restore back the original icon
                            _hIcon = hIcoApp;
                        }
                        delete pImgBkgnd;
                        pImgBkgnd = NULL;
                        bBkgndImg = TRUE;
                    }
                }
            }
            pstrBkgnd->Release();
            pstrBkgnd = NULL;
        }
    }

    // Fallback: If no icon was available, use the measurement to draw the 'Dew' logo
    if (!_pImgIcon || !bCustIconApplied)
    {
        if (_pFntIcon) { delete _pFntIcon; _pFntIcon = NULL; }
        _pFntIcon = new Font(_lpSymbolFont->SymbolFontGDIPlus, (REAL)_F(16), FontStyleRegular, UnitPixel);
        _ptIconStart.X = 0.0f; _ptIconStart.Y = 0.0f;
        if (_pFntIcon)
        {
            pGr->MeasureString(DEWUI_SYMBOL_MAIN_APP, -1, _pFntIcon, _ptIconStart, &rctIcon);
            _ptIconStart.X = ((REAL)iIconDim - rctIcon.Width)/2.0f;
            _ptIconStart.Y = ((REAL)iIconDim - rctIcon.Height)/2.0f;
            if (_hIcon) { DestroyIcon(_hIcon); _hIcon = NULL; }
            #ifndef DEW_THEMER_MODE
            _hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_DEWAPP));
            #endif
            SetClassLongPtr(_hWnd, GCLP_HICON, (LONG)_hIcon);
            SetClassLongPtr(_hWnd, GCLP_HICONSM, (LONG)_hIcon);
        }
    }
    delete pGr;
    pGr = NULL;

    // Broadcast the 'ApplyTheme' functions of all the (sub)components
    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
    {
        // Verify that the pointers are valid
        if (_ducComponents[nIndex]._lpObject)
        {
            // Refresh the display only if the component is visible.
            if (_PlayerSettings.ComponentVisibility[nIndex])
                _ducComponents[nIndex]._lpObject->ApplyTheme(Theme);
            // Otherwise, just keep the images prepared.
            // The individual '_Draw()' will be called when the components are displayed back anyways.
            else
                _ducComponents[nIndex]._lpObject->ApplyTheme(Theme, FALSE);
            
            // Finally, if the location is default,
            // pass that on to the component, so that
            // the default location (x,y) is auto-revised.
            // Components explicitly placed elsewhere
            // will not be disturbed.
            if (_PlayerSettings.ComponentPos[nIndex] == 0x00)
                _ducComponents[nIndex]._lpObject->SetLocation(0x00);
        }
    }

    // All done. Now set a dummy length and time
    // for the seekbar (themer mode only)
#ifdef DEW_THEMER_MODE
    if (_lpSeekBar)
    {
        _lpSeekBar->SetLength(311000);
        _lpSeekBar->SetTime(107000);
    }
#endif

    if (bForceDraw)
        this->_Draw(NULL);

    if (_lpCmdDMPlayPause) _lpCmdDMPlayPause->ApplyTheme(Theme);
    return;
}

void DEWWINDOW::Move(int iXPos, int iYPos, BOOL bCentered) {} // NOP

void DEWWINDOW::Scale(BYTE btIconScale) {} // NOP

void DEWWINDOW::SetLocation(DWORD dwPresetLocation) {} // NOP

#ifdef DEW_THEMER_MODE
void DEWWINDOW::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP
#endif

DEWWINDOW::~DEWWINDOW()
{
    UINT        nIndex, nCount;

    // Release the memory map
    if (_lpMapContent)
    {
        UnmapViewOfFile(_lpMapContent);
        _lpMapContent = NULL;
    }

    if (_hMap)
    {
        CloseHandle(_hMap);
        _hMap = NULL;
    }

    Shell_NotifyIcon(NIM_DELETE, &_nidTray);
    DestroyIcon(_hIcon);
    // Before wrapping up, make sure playlist files are saved (if enabled)
    if (_PlayerSettings.RememberHistory)
    {
        // Flush out any past history
        if (_PlayerSettings.SongList)
        {
            LocalFree(_PlayerSettings.SongList);
            _PlayerSettings.SongList = NULL;
        }
        _PlayerSettings.HistoryFileCount = 0;

        nCount = _lpPlaylist->GetFileCount();
        if (nCount)
        {
            _PlayerSettings.SongList = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST)*nCount);
            nCount = _lpPlaylist->GetFiles(_PlayerSettings.SongList, nCount);
        }
        _PlayerSettings.HistoryFileCount = nCount;
    }

    for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT; nIndex++)
    {
        if (_ducComponents[nIndex]._lpObject)
        {
            delete (_ducComponents[nIndex]._lpObject);
            _ducComponents[nIndex]._lpObject = NULL;
        }
    }

    if (_lpIconRepo) { delete _lpIconRepo; _lpIconRepo = NULL; }
    if (_lpPropPage) { delete _lpPropPage; _lpPropPage = NULL; }
    if (_lpShutdownTimer) { delete _lpShutdownTimer; _lpShutdownTimer = NULL; }
    if (_lpPlaylistExporter) { delete _lpPlaylistExporter; _lpPlaylistExporter = NULL; }
    if (_lpDiscSelector) { delete _lpDiscSelector; _lpDiscSelector = NULL; }
#ifndef DEW_THEMER_MODE
    if (_lpAboutWindow) { delete _lpAboutWindow; _lpAboutWindow = NULL; }
    this->_EA_FlushEngine();
#endif
    if (_lpCmdDMPlayPause) { delete _lpCmdDMPlayPause; _lpCmdDMPlayPause = NULL; }
    if (_hMnuSystem) { DestroyMenu(_hMnuSystem); _hMnuSystem = NULL; }
    if (_hMnuTray) { DestroyMenu(_hMnuTray); _hMnuTray = NULL; }
    if (_hMnuContext) { DestroyMenu(_hMnuContext); _hMnuContext = NULL; }
    if (_hMnuOpen) { DestroyMenu(_hMnuOpen); _hMnuOpen = NULL; }
    if (_hMnuShow) { DestroyMenu(_hMnuShow); _hMnuShow = NULL; }
    if (_hMnuHide) { DestroyMenu(_hMnuHide); _hMnuHide = NULL; }
    if (_pFntIcon) { delete _pFntIcon; _pFntIcon = NULL; }
    if (_pImgIcon) { delete _pImgIcon; _pImgIcon = NULL; }

    return;
}

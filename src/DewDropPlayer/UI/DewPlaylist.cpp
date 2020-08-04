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

#include "DewPlaylist.h"

#ifndef DEW_THEMER_MODE

#pragma region PLAYLISTAUDIOITEM Implementations

PLAYLISTAUDIOITEM::PLAYLISTAUDIOITEM() :
ID(_wsID), FilePath(_wsFilePath), Title(_wsTitle), Duration(_dwDuration),
Parsed(_bParsed), Valid(_bValid), MediaType(_btMediaType), Index(_iRowIndex)
{
    this->ResetData();
}

PLAYLISTAUDIOITEM::PLAYLISTAUDIOITEM(const PLAYLISTAUDIOITEM& PLAudioItem) :
ID(_wsID), FilePath (_wsFilePath), Title (_wsTitle), Duration (_dwDuration),
Parsed (_bParsed), Valid (_bValid), MediaType (_btMediaType),
Index (_iRowIndex)
{
    CopyMemory((this->_wsID), PLAudioItem._wsID, 64 * sizeof(wchar_t));
    CopyMemory((this->_wsFilePath), PLAudioItem._wsFilePath, MAX_CHAR_PATH * sizeof(wchar_t));
    CopyMemory((this->_wsTitle), PLAudioItem._wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    this->_dwDuration = PLAudioItem._dwDuration;
    this->_bParsed = PLAudioItem._bParsed;
    this->_bValid = PLAudioItem._bValid;
    this->_btMediaType = PLAudioItem._btMediaType;

    this->_hWndLVSrc = PLAudioItem._hWndLVSrc;
    this->_iRowIndex = PLAudioItem._iRowIndex;
    this->_iColIndexID = PLAudioItem._iColIndexID;
    this->_iColIndexFilePath = PLAudioItem._iColIndexFilePath;
    this->_iColIndexTitle = PLAudioItem._iColIndexTitle;
    this->_iColIndexDurationDisplay = PLAudioItem._iColIndexDurationDisplay;
    this->_iColIndexDuration = PLAudioItem._iColIndexDuration;
    this->_iColIndexParsed = PLAudioItem._iColIndexParsed;
    this->_iColIndexValid = PLAudioItem._iColIndexValid;
    this->_iColIndexMediaType = PLAudioItem._iColIndexMediaType;

    return;
}

PLAYLISTAUDIOITEM::PLAYLISTAUDIOITEM(HWND hWndLVSrc, const int iRow, int iColIndexID,
                                     int iColIndexFilePath, int iColIndexTitle,
                                     int iColIndexDurationDisplay, int iColIndexDuration, int iColIndexParsed,
                                     int iColIndexValid, int iColIndexMediaType) :
// Public consts
ID(_wsID), FilePath (_wsFilePath), Title (_wsTitle), Duration (_dwDuration),
Parsed (_bParsed), Valid (_bValid), MediaType (_btMediaType), Index (_iRowIndex)
{
    this->SetFromListView(hWndLVSrc, iRow, iColIndexID, iColIndexFilePath, iColIndexTitle, _iColIndexDurationDisplay, iColIndexDuration, iColIndexParsed, iColIndexValid, iColIndexMediaType);

    return;
}

void PLAYLISTAUDIOITEM::operator= (const PLAYLISTAUDIOITEM& PLAudioItem)
{
    CopyMemory((this->_wsFilePath), PLAudioItem._wsFilePath, MAX_CHAR_PATH * sizeof(wchar_t));
    CopyMemory((this->_wsTitle), PLAudioItem._wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    this->_dwDuration = PLAudioItem._dwDuration;
    this->_bParsed = PLAudioItem._bParsed;
    this->_bValid = PLAudioItem._bValid;
    this->_btMediaType = PLAudioItem._btMediaType;

    this->_hWndLVSrc = PLAudioItem._hWndLVSrc;
    this->_iRowIndex = PLAudioItem._iRowIndex;
    this->_iColIndexID = PLAudioItem._iColIndexID;
    this->_iColIndexFilePath = PLAudioItem._iColIndexFilePath;
    this->_iColIndexTitle = PLAudioItem._iColIndexTitle;
    this->_iColIndexDurationDisplay = PLAudioItem._iColIndexDurationDisplay;
    this->_iColIndexDuration = PLAudioItem._iColIndexDuration;
    this->_iColIndexParsed = PLAudioItem._iColIndexParsed;
    this->_iColIndexValid = PLAudioItem._iColIndexValid;
    this->_iColIndexMediaType = PLAudioItem._iColIndexMediaType;
}


void PLAYLISTAUDIOITEM::SetFromListView(HWND hWndLVSrc, const int iRow, int iColIndexID, int iColIndexFilePath, int iColIndexTitle, int iColIndexDurationDisplay, int iColIndexDuration, int iColIndexParsed, int iColIndexValid, int iColIndexMediaType)
{
    LVITEM              lvItem{ 0 };
    wchar_t             wsText[32]{ 0 };
    wchar_t             wsFilePath[MAX_CHAR_PATH];

    _hWndLVSrc = hWndLVSrc;
    _iRowIndex = iRow;
    _iColIndexID = iColIndexID;
    _iColIndexFilePath = iColIndexFilePath;
    _iColIndexTitle = iColIndexTitle;
    _iColIndexDurationDisplay = iColIndexDurationDisplay;
    _iColIndexDuration = iColIndexDuration;
    _iColIndexParsed = iColIndexParsed;
    _iColIndexValid = iColIndexValid;
    _iColIndexMediaType = iColIndexMediaType;

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;

    // 'ID'
    lvItem.iSubItem = _iColIndexID;
    lvItem.pszText = _wsID;
    lvItem.cchTextMax = 64;
    ListView_GetItem(_hWndLVSrc, &lvItem);

    // 'File Path'
    lvItem.iSubItem = _iColIndexFilePath;
    lvItem.pszText = wsFilePath;
    lvItem.cchTextMax = MAX_CHAR_PATH;
    ListView_GetItem(_hWndLVSrc, &lvItem);
    PathCchCanonicalize(_wsFilePath, MAX_CHAR_PATH, wsFilePath);

    // 'Title'
    lvItem.iSubItem = _iColIndexTitle;
    lvItem.pszText = _wsTitle;
    lvItem.cchTextMax = TAG_DATA_LENGTH;
    ListView_GetItem(_hWndLVSrc, &lvItem);

    // 'Parsed' Flag
    lvItem.iSubItem = _iColIndexParsed;
    lvItem.pszText = wsText;
    lvItem.cchTextMax = 16;
    ListView_GetItem(_hWndLVSrc, &lvItem);
    _bParsed = !lstrcmpi(wsText, L"1");
    ZeroMemory(wsText, 16 * sizeof(wchar_t));

    if (_bParsed)
    {
        // 'Raw Duration'
        lvItem.iSubItem = _iColIndexDuration;
        lvItem.pszText = wsText;
        lvItem.cchTextMax = 32;
        ListView_GetItem(_hWndLVSrc, &lvItem);
        _dwDuration = _wtol(wsText);

        // Valid
        lvItem.iSubItem = _iColIndexValid;
        lvItem.pszText = wsText;
        lvItem.cchTextMax = 32;
        ListView_GetItem(_hWndLVSrc, &lvItem);
        if (!lstrcmpi(wsText, L"0"))
            _bValid = FALSE;
        else
            _bValid = TRUE;

        // Media Type
        lvItem.iSubItem = _iColIndexMediaType;
        lvItem.pszText = wsText;
        lvItem.cchTextMax = 32;
        ListView_GetItem(_hWndLVSrc, &lvItem);
        _btMediaType = (BYTE)_wtol(wsText);

    }
    else
    {
        _dwDuration = 0; // 'Raw Duration'
        _bValid = TRUE; // Valid File (since not parsed yet)
        _btMediaType = DEWMT_UNKNOWN; // (Yet ot be parsed) Unknwon Media Type
    }
    return;
}

void PLAYLISTAUDIOITEM::AutoAdjustIndex()
{
    LVFINDINFO      lvfi{ 0 };
    int             iRevisedIndex;

    if (_iRowIndex == -1)
        return;
    if (lstrlen(_wsID) <= 0)
    {
        this->ResetData();
        return;
    }

    lvfi.flags = LVFI_STRING;
    lvfi.psz = _wsID;

    iRevisedIndex = ListView_FindItem(_hWndLVSrc, -1, &lvfi);
    
    // Found and same as before: No action needed.
    if (iRevisedIndex == _iRowIndex)
        return;
    // Found, but at a new index: Revise the index
    else if (iRevisedIndex != _iRowIndex && iRevisedIndex >= 0)
        _iRowIndex = iRevisedIndex;
    // Other cases (like not found, i.e. iRevisedIndex = -1): Reset contents
    else
        this->ResetData();

    return;
}

void PLAYLISTAUDIOITEM::UpdateAll(LPCWSTR wsTitle, DWORD dwDuration, BOOL bParsed, BOOL bValid, BYTE btMediaType)
{
    this->UpdateTitle(wsTitle);
    this->UpdateDuration(dwDuration);
    this->UpdateParsed(bParsed);
    this->UpdateValid(bValid);
    this->UpdateMediaType(btMediaType);
    return;
}

void PLAYLISTAUDIOITEM::UpdateTitle(LPCWSTR wsTitle)
{
    LVITEM      lvItem{ 0 };

    StringCchPrintf(_wsTitle, TAG_DATA_LENGTH, wsTitle);
    
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;
    lvItem.iSubItem = _iColIndexTitle;
    lvItem.pszText = _wsTitle;
    ListView_SetItem(_hWndLVSrc, &lvItem);
    return;
}

void PLAYLISTAUDIOITEM::UpdateDuration(DWORD dwDuration)
{
    LVITEM      lvItem{ 0 };
    wchar_t     wsText[32];

    _dwDuration = dwDuration;

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;
    
    StringCchPrintf(wsText, 32, L"%02u:%02u", ((_dwDuration / 1000) / 60), ((_dwDuration / 1000) % 60));
    lvItem.iSubItem = _iColIndexDurationDisplay;
    lvItem.pszText = wsText;
    ListView_SetItem(_hWndLVSrc, &lvItem);

    StringCchPrintf(wsText, 32, L"%u", _dwDuration);
    lvItem.pszText = wsText;
    lvItem.iSubItem = _iColIndexDuration;
    ListView_SetItem(_hWndLVSrc, &lvItem);

    return;

}

void PLAYLISTAUDIOITEM::UpdateParsed(BOOL bParsed)
{
    LVITEM      lvItem{ 0 };

    _bParsed = bParsed;
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;
    lvItem.iSubItem = _iColIndexParsed;
    if (_bParsed)
        lvItem.pszText = L"1";
    else
        lvItem.pszText = L"0";
    ListView_SetItem(_hWndLVSrc, &lvItem);
    return;
}

void PLAYLISTAUDIOITEM::UpdateValid(BOOL bValid)
{
    LVITEM      lvItem{ 0 };
    
    _bValid = bValid;
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;
    lvItem.iSubItem = _iColIndexValid;
    if (_bValid)
        lvItem.pszText = L"1";
    else
        lvItem.pszText = L"0";
    ListView_SetItem(_hWndLVSrc, &lvItem);
    return;
}

void PLAYLISTAUDIOITEM::UpdateMediaType(BYTE btMediaType)
{
    LVITEM      lvItem{ 0 };
    wchar_t     wsText[32]{ 0 };

    _btMediaType = btMediaType;
    StringCchPrintf(wsText, 32, L"%u", _btMediaType);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = _iRowIndex;
    lvItem.iSubItem = _iColIndexMediaType;
    lvItem.pszText = wsText;
    ListView_SetItem(_hWndLVSrc, &lvItem);
    return;
}

void PLAYLISTAUDIOITEM::ResetData()
{
    // Clear the previous indicator icon
    if (_iRowIndex >= 0)
    {
        LVITEM      lvItem{ 0 };

        lvItem.mask = LVIF_IMAGE;
        lvItem.iImage = 0;
        lvItem.iItem = _iRowIndex;
        lvItem.iSubItem = _iColIndexTitle;
        ListView_SetItem(_hWndLVSrc, &lvItem);
    }

    ZeroMemory(_wsFilePath, MAX_CHAR_PATH * sizeof(wchar_t));
    ZeroMemory(_wsTitle, TAG_DATA_LENGTH * sizeof(wchar_t));
    _dwDuration = 0;
    _bParsed = FALSE;
    _bValid = FALSE;
    _btMediaType = DEWMT_UNKNOWN;

    _hWndLVSrc = NULL;
    _iRowIndex = -1;
    _iColIndexID = -1;
    _iColIndexFilePath = -1;
    _iColIndexTitle = -1;
    _iColIndexDurationDisplay = -1;
    _iColIndexDuration = -1;
    _iColIndexParsed = -1;
    _iColIndexValid = -1;
    _iColIndexMediaType = -1;

    return;
}

#pragma endregion

#endif

#ifdef DEW_THEMER_MODE
// Set a dummy list for illustration purposes
// Set the songs that are really (as Eric Cartman says: 'kewl')!
void DEWPLAYLIST::_SetDummyList()
{
    LVITEM      lvItem { 0 };
    int         iIndex;

    wchar_t* const   wsWellKnownSongs[] = { L"Coming Back To Life",
                                            L"Lift",
                                            L"Comfortably Numb",
                                            L"Crawling",
                                            L"Champagne Supernova",
                                            L"Welcome To My Life" };

    wchar_t* const   wsDurations[] =      { L"06:19",   // CBTL
                                            L"05:11",   // L
                                            L"06:23",   // CN
                                            L"03:29",   // C
                                            L"07:27",   // CS
                                            L"03:23" }; // WTML

    for (iIndex = 0; iIndex < 6; iIndex++)
    {
        lvItem.mask = LVIF_TEXT;
        lvItem.iSubItem = _COL_SEQ_AUDIO_ITEM_ID;
        lvItem.iItem = iIndex;
        lvItem.pszText = wsWellKnownSongs[iIndex];
        SendMessage(_hLstItems, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

        lvItem.mask = LVIF_TEXT;
        lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
        lvItem.iItem = iIndex;
        lvItem.pszText = wsWellKnownSongs[iIndex];
        SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
        lvItem.iImage = 0;
        lvItem.pszText = wsWellKnownSongs[iIndex];
        SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

        lvItem.mask = LVIF_TEXT;
        lvItem.iSubItem = _COL_SEQ_AUDIO_DURATION_DISPLAY;
        lvItem.pszText = wsDurations[iIndex];
        lvItem.cchTextMax = lstrlen(wsDurations[iIndex]);
        SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);
    }

    lvItem.mask = LVIF_IMAGE;
    lvItem.iItem = 1;
    lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
    lvItem.iImage = 1;
    ListView_SetItem(_hLstItems, &lvItem);

    ListView_SetItemState(_hLstItems, -1, 0, 0x00F);
    ListView_SetItemState(_hLstItems, 1, LVIS_SELECTED | LVIS_FOCUSED, 0x00F);
    SetFocus(_hLstItems);
    return;
}
#endif

DWORD WINAPI DEWPLAYLIST::_TrdQueue(LPVOID lpv)
{
    LPDEWPLAYLIST       lpPlaylist;
    LPSOURCEQUEUE       lpCurr;
    LPFILELIST          lpPLFileItem;

    lpPlaylist = (LPDEWPLAYLIST)lpv;
    lpPlaylist->_bParsing = TRUE;

    // Pick from '_lpQueueStart' entry and keep shifting down till nothing is left
    lpCurr = lpPlaylist->_lpQueueStart;
    while (lpCurr)
    {
        if (lpCurr->bDir)
        {
            lpPlaylist->_Enqueue(lpCurr->wsPath);
        }
        else
        {
            if (lstrcmpi(PathFindExtension(lpCurr->wsPath), L".pls") &&
                lstrcmpi(PathFindExtension(lpCurr->wsPath), L".cue") &&
                lstrcmpi(PathFindExtension(lpCurr->wsPath), L".m3u") &&
                lstrcmpi(PathFindExtension(lpCurr->wsPath), L".m3u8"))
            {
                lpPlaylist->_ParseFileInfoAndAdd(lpCurr->wsPath);
            }
            else
            {
                lpPlaylist->_Parser.ParsePlaylistFile(lpCurr->wsPath);
                if (!(lpPlaylist->_Parser.HasError) &&
                    (lpPlaylist->_Parser.Files) &&
                    (lpPlaylist->_Parser.FileCount > 0))
                {
                    for (lpPLFileItem = lpPlaylist->_Parser.Files; lpPLFileItem; lpPLFileItem = lpPLFileItem->next)
                        lpPlaylist->_ParseFileInfoAndAdd(lpPLFileItem->wsFileName);
                    lpPlaylist->_Parser.Reset();
                }
            }
        }
        lpPlaylist->_lpQueueStart = lpCurr->next;
        LocalFree(lpCurr);
        lpCurr = lpPlaylist->_lpQueueStart;
    }
    lpPlaylist->_lpQueueEnd = NULL;
    lpPlaylist->_AdjustColumnWidths();
    lpPlaylist->_bParsing = FALSE;
    lpPlaylist->_hTrdQueue = NULL;

    // All done. Ready to accept a fresh stack. Send the notification message to the parent:
    lpPlaylist->_ParseCompleteEvent();
    return 0;
}

LRESULT CALLBACK DEWPLAYLIST::_LstMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam,
                                             LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    LPDEWPLAYLIST       lpPlaylist;

    lpPlaylist = (LPDEWPLAYLIST)dwRefData;
    if ((HWND)nID == lpPlaylist->_hLstItems)
        return lpPlaylist->_LstWndProc(hWnd, nMsg, wParam, lParam);
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWPLAYLIST::_ContextMenu(LONG iX, LONG iY)
{
    MENUITEMINFO        mii;
    UINT                nPos = 0;

    if (!_hMnuContext)
    {
        _hMnuContext = CreatePopupMenu();

        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
        mii.fType = MFT_STRING;

        // 'Play' (only if single item is selected)
        mii.wID = IDM_CTX_PL_PLAY;
        mii.dwTypeData = (LPTSTR)L"Play";
        mii.hbmpItem = _lpIconRepo->BmpPlay;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);

        // 'Remove'
        mii.wID = IDM_CTX_PL_DELETE;
        mii.dwTypeData = (LPTSTR)L"Remove";
        mii.hbmpItem = _lpIconRepo->BmpDelete;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);

        // 'Open File Location' (only if single item is selected)
        mii.wID = IDM_CTX_PL_EXPLORE_LOCATION;
        mii.dwTypeData = (LPTSTR)L"Open File Location";
        mii.hbmpItem = _lpIconRepo->BmpOpenDir;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);

        // 'Separator'
        mii.fMask = MIIM_FTYPE | MIIM_ID;
        mii.fType = MFT_SEPARATOR;
        mii.wID = IDM_TRAY_SEP;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);
        mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
        mii.fType = MFT_STRING;

        // 'Export Selected'
        mii.wID = IDM_CTX_PL_EXPORT_SEL;
        mii.dwTypeData = (LPTSTR)L"Export Selected";
        mii.hbmpItem = _lpIconRepo->BmpExportSel;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);

        // 'Export All'
        mii.wID = IDM_CTX_PL_EXPORT_ALL;
        mii.dwTypeData = (LPTSTR)L"Export All";
        mii.hbmpItem = _lpIconRepo->BmpExportAll;
        InsertMenuItem(_hMnuContext, nPos++, TRUE, &mii);
    }

    // Play option is available only for single selection
    EnableMenuItem(_hMnuContext, IDM_CTX_PL_PLAY, MF_BYCOMMAND |
                    ((ListView_GetSelectedCount(_hLstItems) == 1) ? MF_ENABLED : MF_DISABLED));

    // Explore option is available only for single selection
    EnableMenuItem(_hMnuContext, IDM_CTX_PL_EXPLORE_LOCATION, MF_BYCOMMAND |
                    ((ListView_GetSelectedCount(_hLstItems) == 1) ? MF_ENABLED : MF_DISABLED) );
    
    TrackPopupMenuEx(_hMnuContext, TPM_RIGHTBUTTON, iX, iY, _hWnd, NULL);
    return;
}

void DEWPLAYLIST::_DeleteItems(BOOL bDeleteAll)
{

#ifndef DEW_THEMER_MODE

    int     iItem;

    if (    ListView_GetSelectedCount(_hLstItems) &&
            (ListView_GetSelectedCount(_hLstItems) == (UINT)ListView_GetItemCount(_hLstItems) ||
            bDeleteAll)   )
    {
        // In case all items are selected, remove them in a single go
        // rather than having to loop unnecessarily
        ListView_DeleteAllItems(_hLstItems);
        // Reset the active object, since playlist is now empty
        _ActiveItem.ResetData();
        _iCurrIndex = -1;
    }
    else
    {
        do
        {
            iItem = ListView_GetNextItem(_hLstItems, -1, LVNI_SELECTED);
            ListView_DeleteItem(_hLstItems, iItem);
        } while (ListView_GetSelectedCount(_hLstItems));
        // Selective deletion might have altered the index of the current object
        // Find that out and update the current index accordingly
        _ActiveItem.AutoAdjustIndex();
        _iCurrIndex = _ActiveItem.Index;
    }
   
#endif

    return;
}

void DEWPLAYLIST::_ExportItems(LPCWSTR wsExportFile, BYTE btExportFormat, BOOL bUseRelativePaths, BOOL bExportAll)
{
    LPFILELIST      lpExportList = NULL;
    int             iIndex, iNextIndex;
    int             iTotalCount, iSelectedCount, iItemCount;
    LVITEM          lvItem { 0 };
    FILE            *fpPlaylist = NULL;
    const wchar_t   *wsM3UHeader = L"#EXTM3U";
    const wchar_t   *wsPLSHeader = L"[playlist]";
    const wchar_t   *wsNEWLINE = L"\n"; // Windows locale will automatically take care of \r\n
    wchar_t         wsInfo[MAX_CHAR_PATH] { 0 }, wsAbsPath[MAX_CHAR_PATH] { 0 }, wsPlaylistExportDir[MAX_CHAR_PATH] { 0 };

    // Nothing there
    iTotalCount = (UINT)ListView_GetItemCount(_hLstItems);
    iSelectedCount = (UINT)ListView_GetSelectedCount(_hLstItems);
    if (iTotalCount <= 0) return;

    // Nothing selected for 'Selective Export'
    if (iSelectedCount == 0 && !bExportAll) return;

    CopyMemory(wsPlaylistExportDir, wsExportFile, lstrlen(wsExportFile) * sizeof(wchar_t));

    for (iIndex = (int)lstrlen(wsPlaylistExportDir) - 1; iIndex; iIndex--)
    {
        if (wsPlaylistExportDir[iIndex] == L'\\')
        {
            wsPlaylistExportDir[iIndex] = L'\0';
            break;
        }
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
    lvItem.cchTextMax = MAX_CHAR_PATH;
    iItemCount = bExportAll ? iTotalCount : iSelectedCount;
    lpExportList = (LPFILELIST)LocalAlloc(LPTR, sizeof(FILELIST) * (UINT)iItemCount);

    if (bExportAll)
    {
        for (iIndex = 0; iIndex < iItemCount; iIndex++)
        {
            lvItem.pszText = wsInfo;
            lvItem.iItem = iIndex;
            ListView_GetItem(_hLstItems, &lvItem);
            PathCchCanonicalize(wsAbsPath, MAX_CHAR_PATH, wsInfo);
            if (!bUseRelativePaths)
            {
                StringCchPrintf((lpExportList[iIndex].wsFileName), MAX_CHAR_PATH, wsAbsPath);
            }
            else
            {
                PathRelativePathTo(lpExportList[iIndex].wsFileName,
                                   wsPlaylistExportDir, FILE_ATTRIBUTE_DIRECTORY,
                                   wsAbsPath, FILE_ATTRIBUTE_NORMAL);
            }
        }
    }
    else
    {
        iNextIndex = -1;
        iIndex = 0;
        do
        {
            iNextIndex = ListView_GetNextItem(_hLstItems, iNextIndex, LVNI_SELECTED);
            if (iNextIndex != -1)
            {
                lvItem.pszText = wsInfo;
                lvItem.iItem = iNextIndex;
                ListView_GetItem(_hLstItems, &lvItem);
                PathCchCanonicalize(wsAbsPath, MAX_CHAR_PATH, wsInfo);
                if (!bUseRelativePaths)
                {
                    StringCchPrintf((lpExportList[iIndex].wsFileName), MAX_CHAR_PATH, wsAbsPath);
                }
                else
                {
                    PathRelativePathTo(lpExportList[iIndex].wsFileName,
                                   wsPlaylistExportDir, FILE_ATTRIBUTE_DIRECTORY,
                                   wsAbsPath, FILE_ATTRIBUTE_NORMAL);
                }
                iIndex++;
            }
        } while (iNextIndex != -1);
    }

    fpPlaylist = NULL;
    _wfopen_s(&fpPlaylist, wsExportFile, L"wt, ccs=UTF-8");
    if (fpPlaylist == NULL)
    {
        MessageBox(_Parent->Handle, L"An unexpected error occurred, trying to "\
                   L"export the playlist. Please try again or choose a separate location.",
                   L"Playlist Export Error", MB_OK | MB_ICONERROR);
        LocalFree(lpExportList);
        return;
    }

    if (btExportFormat == DEWUI_PL_EXPORT_FMT_M3U8)
    {
        fwrite(wsM3UHeader, lstrlen(wsM3UHeader) * sizeof(wchar_t), 1, fpPlaylist);
        fwrite(wsNEWLINE, lstrlen(wsNEWLINE) * sizeof(wchar_t), 1, fpPlaylist);
        for (iIndex = 0; iIndex < iItemCount; iIndex++)
        {
            fwrite(lpExportList[iIndex].wsFileName, lstrlen(lpExportList[iIndex].wsFileName) * sizeof(wchar_t), 1, fpPlaylist);
            fwrite(wsNEWLINE, lstrlen(wsNEWLINE) * sizeof(wchar_t), 1, fpPlaylist);
        }
    }
    else if (btExportFormat == DEWUI_PL_EXPORT_FMT_PLS)
    {
        fwrite(wsPLSHeader, lstrlen(wsPLSHeader) * sizeof(wchar_t), 1, fpPlaylist);
        fwrite(wsNEWLINE, lstrlen(wsNEWLINE) * sizeof(wchar_t), 1, fpPlaylist);
        for (iIndex = 0; iIndex < iItemCount; iIndex++)
        {
            StringCchPrintf(wsInfo, MAX_CHAR_PATH, L"File%d=", iIndex+1);
            fwrite(wsInfo, lstrlen(wsInfo) * sizeof(wchar_t), 1, fpPlaylist);
            fwrite(lpExportList[iIndex].wsFileName, lstrlen(lpExportList[iIndex].wsFileName) * sizeof(wchar_t), 1, fpPlaylist);
            fwrite(wsNEWLINE, lstrlen(wsNEWLINE) * sizeof(wchar_t), 1, fpPlaylist);
        }
    }
    else
    {
        MessageBox(_Parent->Handle, L"An invalid playlist format was provided.\n" \
                   L"Please try again with one of the supported playlist format.",
                   L"Playlist Export Error", MB_OK | MB_ICONERROR);
        fclose(fpPlaylist);
        LocalFree(lpExportList);
        return;
    }
    fclose(fpPlaylist);

    StringCchPrintf(wsInfo, MAX_CHAR_PATH, L"The playlist was successfully exported to:\n'%s'\n", wsExportFile);
    MessageBox(_Parent->Handle, wsInfo, L"Playlist export success", MB_OK | MB_ICONINFORMATION);

    if (lpExportList) LocalFree(lpExportList);
    return;
}

void DEWPLAYLIST::_ClearQueueProcess()
{
    LPSOURCEQUEUE lpCurr;

    if (_hTrdQueue) { TerminateThread(_hTrdQueue, 0x00); _hTrdQueue = NULL; _bParsing = FALSE; }

    // Do NOT clear the queue before terminating the thread!
    for (lpCurr = _lpQueueStart; lpCurr; lpCurr = lpCurr->next)
    {
        _lpQueueStart = lpCurr->next;
        LocalFree(lpCurr);
    }
    _lpQueueStart = NULL;
    _lpQueueEnd = NULL;
}

void DEWPLAYLIST::_AddSource(LPCWSTR wsPath, BOOL bDir)
{
    LPSOURCEQUEUE   lpCurr = NULL;

    lpCurr = (LPSOURCEQUEUE)LocalAlloc(LPTR, sizeof(SOURCEQUEUE));
    StringCchPrintf(lpCurr->wsPath, MAX_CHAR_PATH, wsPath);
    lpCurr->bDir = bDir;
    lpCurr->next = NULL;

    if (!_lpQueueStart)
        _lpQueueStart = lpCurr;
    else
        _lpQueueEnd->next = lpCurr;
    _lpQueueEnd = lpCurr;

    return;
}

void DEWPLAYLIST::_Enqueue(LPCWSTR wsDir)
{
    DWORD               dwAttr;
    WIN32_FIND_DATA     wfd { 0 };
    HANDLE              hFind = NULL;
    LPFILELIST          lpCurr;
    wchar_t             wsFilter[MAX_CHAR_PATH];
    wchar_t             wsFullName[MAX_CHAR_PATH];

    StringCchPrintf(wsFilter, MAX_CHAR_PATH, L"%s\\*", wsDir);
    hFind = FindFirstFile(wsFilter, &wfd);
    if (hFind && hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            ZeroMemory(wsFullName, MAX_CHAR_PATH * sizeof(wchar_t));
            StringCchPrintf(wsFullName, MAX_CHAR_PATH, L"%s\\%s", wsDir, wfd.cFileName);
            dwAttr = GetFileAttributes(wsFullName);

            // File
            if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                if (lstrcmpi(PathFindExtension(wsFullName), L".pls") &&
                    lstrcmpi(PathFindExtension(wsFullName), L".cue") &&
                    lstrcmpi(PathFindExtension(wsFullName), L".m3u") &&
                    lstrcmpi(PathFindExtension(wsFullName), L".m3u8"))
                {
                    this->_ParseFileInfoAndAdd(wsFullName);
                }
                else
                {
                    _Parser.ParsePlaylistFile(wsFullName);

                    // If the file turns out to be a playlist, push in all the contents instead
                    if (!_Parser.HasError && _Parser.Files && _Parser.FileCount > 0)
                    {
                        for (lpCurr = _Parser.Files; lpCurr; lpCurr = lpCurr->next)
                            this->_ParseFileInfoAndAdd(lpCurr->wsFileName);
                        _Parser.Reset();
                    }
                }

            }
            else
            {
                // Skip the '.' & the '..' dirs. they will cause infinite recursion
                // leading to Stack Overflow!
                if (lstrcmpi(wfd.cFileName, L".") && lstrcmpi(wfd.cFileName, L".."))
                    this->_Enqueue(wsFullName);
            }
        } while(FindNextFile(hFind, &wfd));
        FindClose(hFind);
        hFind = NULL;
    }
    return;
}

void DEWPLAYLIST::_ParseFileInfoAndAdd(LPCWSTR wsFileName)
{

#ifndef DEW_THEMER_MODE
    // WARNING! Please do not place any validation checks here.
    // This will be repeatedly called when adding items. So lesser the checks,
    // the better. It is assumed that a valid file will always be sent here
    LVITEM              lvItem { 0 };
    HANDLE              hFind;
    WIN32_FIND_DATA     wfd { 0 };
    wchar_t             wsDuration[32]{ 0 }, wsMediaType[4]{ 0 }, wsID[64];
    wchar_t             wsTitle[TAG_DATA_LENGTH]{ 0 };
    DWORD               dwDuration;
    BYTE                btMediaType;

    lvItem.mask = LVIF_TEXT;

    if (_bDeepScan)
    {
        AUDIOFACTORY::GetAudioPrelimInfo(wsFileName, wsTitle, TAG_DATA_LENGTH, &dwDuration, &btMediaType);

        if (btMediaType != DEWMT_UNKNOWN)
        {
            lvItem.mask = LVIF_TEXT;

            // Insert 'ID'
            _iLastRow++;
            lvItem.iItem = ListView_GetItemCount(_hLstItems);

            StringCchPrintf(wsID, 64, L"%s%08X", _ID_PREFIX, (DWORD)_iLastRow);
            lvItem.iSubItem = _COL_SEQ_AUDIO_ITEM_ID;
            lvItem.pszText = (LPWSTR)wsID;
            SendMessage(_hLstItems, LVM_INSERTITEM, 0, (LPARAM)&lvItem);


            // Insert 'File Path'
            lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
            lvItem.pszText = (LPWSTR)wsFileName;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Title'
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
            lvItem.pszText = wsTitle;
            lvItem.iImage = 0;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);
            lvItem.mask = LVIF_TEXT;

            // Update displayable 'Duration'
            StringCchPrintf(wsDuration, 32, L"%02u:%02u", ((dwDuration/1000) / 60), ((dwDuration/1000) % 60));
            lvItem.iSubItem = _COL_SEQ_AUDIO_DURATION_DISPLAY;
            lvItem.pszText = wsDuration;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update raw 'Duration'
            StringCchPrintf(wsDuration, 32, L"%u", dwDuration);
            lvItem.iSubItem = _COL_SEQ_AUDIO_DURATION;
            lvItem.pszText = wsDuration;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update the 'Parsed' flag to true, since we are sacnning anyways
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_PARSED;
            lvItem.pszText = L"1";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Valid' to blank. We don't know if the engine
            // will actually be able to play it unless we call the
            // Load() function. Till then, we keep it blank
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_VALID;
            lvItem.pszText = L"";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update the 'Media Type'
            StringCchPrintf(wsMediaType, 4, L"%u", btMediaType);
            lvItem.iSubItem = _COL_SEQ_AUDIO_MEDIA_TYPE;
            lvItem.pszText = wsMediaType;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);
        }
        // else: There is no else. If we did not find a suitable engine
        //       we will not add it to the list at all.
    }
    else
    {
        btMediaType = AUDIOFACTORY::GetMediaTypeFromExtension(wsFileName);

        if (btMediaType != DEWMT_UNKNOWN)
        {
            // Insert 'ID'
            _iLastRow++;
            lvItem.iItem = ListView_GetItemCount(_hLstItems);
            StringCchPrintf(wsID, 64, L"%s%08X", _ID_PREFIX, (DWORD)_iLastRow);
            lvItem.iSubItem = _COL_SEQ_AUDIO_ITEM_ID;
            lvItem.pszText = (LPWSTR)wsID;
            SendMessage(_hLstItems, LVM_INSERTITEM, 0, (LPARAM)&lvItem);


            // Insert 'File Path'
            lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
            lvItem.pszText = (LPWSTR)wsFileName;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Title' to the file name
            hFind = FindFirstFile(wsFileName, &wfd);
            FindClose(hFind);
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
            lvItem.pszText = wfd.cFileName;
            lvItem.iImage = 0;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);
            lvItem.mask = LVIF_TEXT;

            // Update 'Duration' to be empty
            lvItem.iSubItem = _COL_SEQ_AUDIO_DURATION_DISPLAY;
            lvItem.pszText = L"";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Raw Duration to be empty
            lvItem.iSubItem = _COL_SEQ_AUDIO_DURATION;
            lvItem.pszText = L"";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Parsed' as false
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_PARSED;
            lvItem.pszText = L"0";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update 'Valid' to blank. We don't know if the engine
            // will actually be able to play it unless we call the
            // Load() function. Till then, we keep it blank
            lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_VALID;
            lvItem.pszText = L"";
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);

            // Update the 'Media Type'
            StringCchPrintf(wsMediaType, 4, L"%u", DEWMT_UNKNOWN);
            lvItem.iSubItem = _COL_SEQ_AUDIO_MEDIA_TYPE;
            lvItem.pszText = wsMediaType;
            SendMessage(_hLstItems, LVM_SETITEM, 0, (LPARAM)&lvItem);
        }

        // else: There is no else. If we did not find a suitable engine
        //       we will not add it to the list at all.
    }

#endif

    return;
}

void DEWPLAYLIST::_OpenSelFileLocation(int iIndex)
{
    HRESULT         hr = S_OK;
    IShellFolder    *pShellFolder = NULL;
    ULONG           ulUsed = 0;
    LPITEMIDLIST    lpidFolder = NULL;
    int             iSelIndex = -1;
    LVITEM          lvItem { 0 };
    wchar_t         wsSelFile[MAX_CHAR_PATH] { 0 };

    if (iIndex < 0 && ListView_GetSelectedCount(_hLstItems) <= 0) return;
    if (iIndex > 0)
    {
        iSelIndex = iIndex;
    }
    else
    {
        iSelIndex = SendMessage(_hLstItems, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
        if (iSelIndex < 0) return;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iSelIndex;
    lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
    lvItem.pszText = wsSelFile;
    lvItem.cchTextMax = MAX_CHAR_PATH;
    ListView_GetItem(_hLstItems, &lvItem);

    if (!lstrlen(wsSelFile)) return;

    hr = SHGetDesktopFolder(&pShellFolder);
    if (SUCCEEDED(hr) && pShellFolder)
    {
        hr = pShellFolder->ParseDisplayName(NULL, NULL, wsSelFile, &ulUsed, &lpidFolder, NULL);
        if (SUCCEEDED(hr) && lpidFolder)
        {
            SHOpenFolderAndSelectItems(lpidFolder, 0, NULL, 0);
        }
        pShellFolder->Release();
        pShellFolder = NULL;
    }
    return;
}

void DEWPLAYLIST::_ParseCompleteEvent()
{
    _Parent->ProcessMessage(this, DEWUI_MSG_PL_PARSE_COMPLETE, 0, 0);
    return;
}

void DEWPLAYLIST::_AdjustColumnWidths()
{

#ifdef _DEBUG
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_ITEM_ID, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_FILE_PATH, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_TITLE_DISPLAY, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_DURATION_DISPLAY, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_DURATION, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_TITLE_PARSED, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_TITLE_VALID, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_MEDIA_TYPE, LVSCW_AUTOSIZE_USEHEADER);
#else
    int     iTitleColLen, iDurationColLen, iSpan;
    RECT    rctLst;

    GetClientRect(_hLstItems, &rctLst);
    iSpan = (rctLst.right - rctLst.left);
    iTitleColLen = (int)((float)iSpan * 0.9f);
    iDurationColLen = iSpan - iTitleColLen;
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_TITLE_DISPLAY, iTitleColLen);
    ListView_SetColumnWidth(_hLstItems, _COL_SEQ_AUDIO_DURATION_DISPLAY, iDurationColLen);
#endif

    return;
}

void DEWPLAYLIST::_Scale(BYTE btIconScale)
{
    if (_btObjectType != DEWUI_OBJ_TYPE_MODULE ||
        _btObjectID != DEWUI_OBJ_PLAYLIST)
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

void DEWPLAYLIST::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;
    RECT        rctLst;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    pGr->FillRectangle(_pBrBack, _F(0), _F(0), _iWidth, _iHeight);
    pGr->DrawLine(_pPenWindowOutline, _iWidth - _F(1), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    pGr->DrawLine(_pPenWindowOutline, _iWidth - _F(1), _iHeight - _F(1), _F(0), _iHeight - _F(1));
    pGr->DrawLine(_pPenWindowOutline, _F(0), _iHeight - _F(1), _F(0), _F(0));
    GetWindowRect(_hLstItems, &rctLst);
    MapWindowPoints(HWND_DESKTOP, _hWnd, (LPPOINT)&rctLst, 2);
    pGr->DrawRectangle(_pPenOutline, _F(DEWUI_DIM_PADDING_X) - _F(1), _F(DEWUI_DIM_PADDING_X) - _F(1),
                       (INT)(rctLst.right - rctLst.left) + _F(1), (INT)(rctLst.bottom - rctLst.top) + _F(1));

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWPLAYLIST::_PrepareImages()
{
    int             iIcoDim;
    Graphics        *pGr = NULL;
    Bitmap          *pImgHIML = NULL;
    Font            *pFontInd = NULL;
    PointF          ptStart(0.0f, 0.0f);
    RectF           rctPos;

    // Re-do the indicator blank/indicator icons
    iIcoDim = (int)((float)DEWUI_DIM_PL_ACTIVE_IND * _fScale * _fIconScale);
    if (_hIcoBlank) DestroyIcon(_hIcoBlank);
    if (_hIcoInd) DestroyIcon(_hIcoInd);
    if (_hImgInd) ImageList_Destroy(_hImgInd);
    _hImgInd = ImageList_Create(iIcoDim, iIcoDim, ILC_COLOR32 | ILC_MASK, 2, 2);
    pImgHIML = new Bitmap(iIcoDim, iIcoDim, PixelFormat32bppARGB);
    pImgHIML->GetHICON(&_hIcoBlank);
    delete pImgHIML;
    pImgHIML = NULL;

    pImgHIML = new Bitmap(iIcoDim, iIcoDim, PixelFormat32bppARGB);
    pGr = Graphics::FromImage(pImgHIML);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pFontInd = new Font(_lpSymbolFont->SymbolFontGDIPlus, (REAL)_F(DEWUI_FNTSPEC_INFO_SYMBOL), FontStyleRegular, UnitPoint);
    pGr->MeasureString(DEWUI_SYMBOL_ACTIVE_IND, -1, pFontInd, ptStart, &rctPos);
    ptStart.X = ((REAL)iIcoDim - rctPos.Width) / 2.0f;
    ptStart.Y = ((REAL)iIcoDim - rctPos.Height) / 2.0f;
    pGr->DrawString(DEWUI_SYMBOL_ACTIVE_IND, -1, pFontInd, ptStart, _pBrText);
    pImgHIML->GetHICON(&_hIcoInd);

    ImageList_ReplaceIcon(_hImgInd, -1, _hIcoBlank);
    ImageList_ReplaceIcon(_hImgInd, -1, _hIcoInd);

    delete pGr; pGr = NULL;
    delete pFontInd; pFontInd = NULL;
    delete pImgHIML; pImgHIML = NULL;
}

LRESULT DEWPLAYLIST::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC                 hDC = NULL;
    PAINTSTRUCT         ps { 0 };
    LPNMITEMACTIVATE    lpNMIA;
    POINT               ptCur;
    int                 iIndex;

    switch(nMsg)
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
            lpNMIA = (LPNMITEMACTIVATE)lParam;
            if (lpNMIA->iItem >= 0)
            {
                if (lpNMIA->hdr.hwndFrom == _hLstItems &&
                    lpNMIA->hdr.code == LVN_ITEMACTIVATE &&
                    _Parent)
                {
                    this->ActivateItem(lpNMIA->iItem);
                }
                else if (lpNMIA->hdr.code == NM_RCLICK)
                {
                    GetCursorPos(&ptCur);
                    this->_ContextMenu(ptCur.x, ptCur.y);
                }
            }
            return FALSE;
        }

        case WM_KEYDOWN: // Pass all keystrokes down to the listview control
        case WM_KEYUP:
        {
            if (IsWindowVisible(_hLstItems))
            {
                SetFocus(_hLstItems);
                SendMessage(_hLstItems, nMsg, wParam, lParam);
            }
            return FALSE;
        }

        case WM_COMMAND:
        {
            if (lParam == 0x00)
            {
                switch(LOWORD(wParam))
                {
                    case IDM_CTX_PL_PLAY:
                    {
                        iIndex = SendMessage(_hLstItems, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
                        if (iIndex >= 0)
                        {
                            this->ActivateItem(iIndex);
                            return FALSE;
                        }
                    }
                    case IDM_CTX_PL_DELETE:
                    {
                        this->_DeleteItems();
                        return FALSE;
                    }
                    case IDM_CTX_PL_EXPLORE_LOCATION:
                    {
                        this->_OpenSelFileLocation();
                        return FALSE;
                    }
                    case IDM_CTX_PL_EXPORT_SEL:
                    {
                        if (_Parent)
                            _Parent->ProcessMessage(this, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)FALSE);
                        return FALSE;
                    }
                    case IDM_CTX_PL_EXPORT_ALL:
                    {
                        if (_Parent)
                            _Parent->ProcessMessage(this, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)TRUE);
                        return FALSE;
                    }
                    return FALSE;
                }
            }
            break;
        }

        case WM_DEWMSG_PL_EXPORT_SEL:
        {
            LPDEWPLAYLISTEXPORTINFO     lpExportInfo;

            lpExportInfo = (LPDEWPLAYLISTEXPORTINFO)lParam;

            if (!(lpExportInfo->bOperationValid)) return FALSE;
            this->_ExportItems(lpExportInfo->wsExportFile,
                               lpExportInfo->DEWUI_PL_EXPORT_FMT,
                               lpExportInfo->bUseRelativePath,
                               (BOOL)wParam);
            return FALSE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWPLAYLIST::_LstWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{

// Block all keyboard processing if in 'Themer' mode
#ifndef DEW_THEMER_MODE
    switch (nMsg)
    {
        case WM_KEYUP:
        {
            if (_Parent && ListView_GetSelectedCount(_hLstItems))
            {
                if (wParam == VK_RETURN)
                {
                    this->ActivateItem(ListView_GetNextItem(_hLstItems, -1, LVNI_FOCUSED));
                }
                else if (wParam == VK_DELETE && !_bParsing) // Just ensure that deletion is not allowed while queuing operation is in progress!
                {
                    this->_DeleteItems();
                }
                else if (wParam == VK_A && (GetKeyState(VK_CONTROL) & 0x8000)) // <CTRL> + A
                {
                    this->SelectAllItems();
                }
                else if (wParam == VK_S && (GetKeyState(VK_CONTROL) & 0x8000) &&
                         ListView_GetNextItem(_hLstItems, -1, LVNI_SELECTED)) // <CTRL> + S / <CTRL> + <SHIFT> + S
                {
                    if (GetKeyState(VK_SHIFT) & 0x8000)
                        _Parent->ProcessMessage(this, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)TRUE); // <CTRL> + <SHIFT> + S - 'Export All'
                    else
                        _Parent->ProcessMessage(this, DEWUI_MSG_PL_EXPORT, 0, (DWORD_PTR)FALSE); // <CTRL> + S - 'Export Selected'
                }
                else if (wParam == VK_Q && (GetKeyState(VK_CONTROL) & 0x8000)) // <CTRL> + Q
                {
                    // Propagate the 'Ctrl+Q' combo to the parent, since it is the
                    // true quitting shortut
                    if (_Parent)
                        SendMessage(_Parent->Handle, WM_CLOSE, 1, 0);
                }
            }
            return FALSE;
        }

    }
#endif
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWPLAYLIST::_MoveToDefaultLocation() {} // NOP. Playlist will always remain at the bottom

DEWPLAYLIST::DEWPLAYLIST(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BOOL bDeepScan) :
_bDeepScan(bDeepScan), QueuingInProgress(_bParsing), ActiveItemIndex(_iCurrIndex)
#ifndef DEW_THEMER_MODE
, ActiveItem(_ActiveItem)
#endif
{
    const wchar_t   *wsPlaylistClass = L"DEWDROP.PLAYLIST.WND";
    WNDCLASSEX      wcex { 0 };
    LVCOLUMN        lvc { 0 };

    _btObjectID = DEWUI_OBJ_PLAYLIST;
    _btObjectType = DEWUI_OBJ_TYPE_MODULE;
    _Parent = Parent;
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;
    _lpIconRepo = new DEWICONREPOSITORY(_iDPI, _fScale);
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
    wcex.lpszClassName  = wsPlaylistClass;

    RegisterClassEx(&wcex);

    this->_Scale(Theme.IconScale);
    _ptPos.x = _F(0);
    _ptPos.y = _F(DEWUI_DIM_WINDOW_Y_NOPL);

    this->ApplyTheme(Theme, FALSE, FALSE);

    _hWnd = CreateWindowEx(0, wsPlaylistClass, _wsText, _PLSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hLstItems = CreateWindowEx(0, WC_LISTVIEW, L"", _LSTSTYLE,
                                _F(DEWUI_DIM_PADDING_X), _F(DEWUI_DIM_PADDING_X),
                                _iWidth - _F(DEWUI_DIM_PADDING_X) * 2,
                                _iHeight - _F(DEWUI_DIM_PADDING_X) * 2,
                                _hWnd, NULL, _hInstance, NULL);

    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 0;

    lvc.pszText = (LPWSTR)L"ID";
    lvc.iSubItem = _COL_SEQ_AUDIO_ITEM_ID;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_ITEM_ID, &lvc);

    lvc.pszText = (LPWSTR)L"File Name";
    lvc.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_FILE_PATH, &lvc);

    lvc.pszText = (LPWSTR)L"Title";
    lvc.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_TITLE_DISPLAY, &lvc);

    lvc.fmt = LVCFMT_RIGHT;
    lvc.pszText = (LPWSTR)L"Duration";
    lvc.iSubItem = _COL_SEQ_AUDIO_DURATION_DISPLAY;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_DURATION_DISPLAY, &lvc);

    lvc.fmt = LVCFMT_RIGHT;
    lvc.pszText = (LPWSTR)L"Raw Duration";
    lvc.iSubItem = _COL_SEQ_AUDIO_DURATION;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_DURATION, &lvc);
    lvc.fmt = LVCFMT_LEFT;

    lvc.pszText = (LPWSTR)L"Prelim Parsed?";
    lvc.iSubItem = _COL_SEQ_AUDIO_TITLE_PARSED;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_TITLE_PARSED, &lvc);

    lvc.pszText = (LPWSTR)L"Valid File?";
    lvc.iSubItem = _COL_SEQ_AUDIO_TITLE_VALID;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_TITLE_VALID, &lvc);
    

    lvc.pszText = (LPWSTR)L"Engine Type";
    lvc.iSubItem = _COL_SEQ_AUDIO_MEDIA_TYPE;
    ListView_InsertColumn(_hLstItems, _COL_SEQ_AUDIO_MEDIA_TYPE, &lvc);
    
    ListView_SetImageList(_hLstItems, _hImgInd, LVSIL_SMALL);

    this->_AdjustColumnWidths();

    SendMessage(_hLstItems, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_SUBITEMIMAGES);

#ifdef DEW_THEMER_MODE
    this->_SetDummyList();
#endif

    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();
    this->_InitiateSubclassing();

    if (_hLstItems && _hLstItems != INVALID_HANDLE_VALUE)
    {
        ListView_SetBkColor(_hLstItems, _crBack.ToCOLORREF());
        ListView_SetTextBkColor(_hLstItems, _crBack.ToCOLORREF());
        ListView_SetTextColor(_hLstItems, _crText.ToCOLORREF());
        ListView_SetOutlineColor(_hLstItems, _crOutline.ToCOLORREF());
        SendMessage(_hLstItems, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
        SetWindowSubclass(_hLstItems, DEWPLAYLIST::_LstMsgHandler, (UINT_PTR)_hLstItems, (DWORD_PTR)this);
    }
    return;
}

void DEWPLAYLIST::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle       gdiFontStyle;
    LOGFONT         lgfFont { 0 };
    Color           crWindowOutline;

    _crBack.SetFromCOLORREF(Theme.WinStyle.BackColor2);
    _crText.SetFromCOLORREF(Theme.ModuleStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.ModuleStyle.OutlineColor);
    crWindowOutline.SetFromCOLORREF(Theme.WinStyle.OutlineColor);

    if (_pBrText) delete _pBrText;
    if (_pBrBack) delete _pBrBack;
    if (_pPenOutline) delete _pPenOutline;
    if (_pPenWindowOutline) delete _pPenWindowOutline;
    if (_pPenText) delete _pPenText;

    _pBrText = new SolidBrush(_crText);
    _pBrBack = new SolidBrush(_crBack);
    _pPenOutline = new Pen(_crOutline, (REAL)_F(1));
    _pPenWindowOutline = new Pen(crWindowOutline, (REAL)_F(1));
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

    this->_PrepareImages();

    if (_hFont) { DeleteObject(_hFont); _hFont = NULL; }

    lgfFont.lfHeight = -MulDiv(_iFontSize, this->_iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = _bBold ? FW_BOLD : FW_NORMAL;
    lgfFont.lfItalic = (_bItalic ? 1 : 0);
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, _wsFontName, 32 * sizeof(wchar_t));
    _hFont = CreateFontIndirect(&lgfFont);

    if (_hLstItems && _hLstItems != INVALID_HANDLE_VALUE)
    {
        ListView_SetBkColor(_hLstItems, _crBack.ToCOLORREF());
        ListView_SetTextBkColor(_hLstItems, _crBack.ToCOLORREF());
        ListView_SetTextColor(_hLstItems, _crText.ToCOLORREF());
        ListView_SetOutlineColor(_hLstItems, _crOutline.ToCOLORREF());
        SendMessage(_hLstItems, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
        ListView_SetImageList(_hLstItems, _hImgInd, LVSIL_SMALL);
    }

    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWPLAYLIST::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

void DEWPLAYLIST::Add(LPCWSTR wsPath, BOOL bFlushAll)
{
    HANDLE              hFind = NULL;
    WIN32_FIND_DATA     wfd { 0 };
    DWORD               dwAttr = 0x00;

    if (bFlushAll)
        this->RemoveAllItems();

    if (!lstrcmpi(wsPath, L".") || !lstrcmpi(wsPath, L"..")) return;

    // Check if valid file/dir
    hFind = FindFirstFile(wsPath, &wfd);
    if (!hFind || hFind == INVALID_HANDLE_VALUE) return;
    FindClose(hFind);

    // Guaranteed a valid file/folder
    dwAttr = GetFileAttributes(wsPath);
    this->_AddSource(wsPath, ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));

    return;
}

void DEWPLAYLIST::AddFileDirect(LPCWSTR wsPath, BOOL bFlushAll)
{
    LPFILELIST      lpCurr = NULL;

    // NOTE: wsPath MUST be a file path
    // If all to be flushed, then clear the thread and queue
    if (bFlushAll)
        this->RemoveAllItems();

    // If the file turns out to be a playlist file, add the contents instead
    if (lstrcmpi(PathFindExtension(wsPath), L".pls") &&
        lstrcmpi(PathFindExtension(wsPath), L".cue") &&
        lstrcmpi(PathFindExtension(wsPath), L".m3u") &&
        lstrcmpi(PathFindExtension(wsPath), L".m3u8"))
    {
        this->_ParseFileInfoAndAdd(wsPath);
    }
    else
    {
        _Parser.ParsePlaylistFile(wsPath);
        if (!_Parser.HasError && _Parser.Files && _Parser.FileCount > 0)
        {
            for (lpCurr = _Parser.Files; lpCurr; lpCurr = lpCurr->next)
                this->_ParseFileInfoAndAdd(lpCurr->wsFileName);
            _Parser.Reset();
        }
    }
    return;
}

void DEWPLAYLIST::SelectAllItems()
{
    // Run the loop backwards. This way,
    // 1. We don't enforce the macro calculation on every iteration
    // 2. We don't need an additional variable to store the total item count
    for (int iItem = ListView_GetItemCount(_hLstItems) - 1; iItem >= 0; iItem--)
        ListView_SetItemState(_hLstItems, iItem, LVIS_SELECTED,  0x000F);
    SetFocus(_hLstItems);
    return;
}

void DEWPLAYLIST::RemoveAllItems()
{
#ifndef DEW_THEMER_MODE
    this->_ClearQueueProcess();
    SendMessage(_hLstItems, LVM_DELETEALLITEMS, 0, 0);
    _ActiveItem.ResetData();
    _iCurrIndex = -1;
#endif
}

void DEWPLAYLIST::ActivateItem(int iNewActiveIndex, BOOL bActivateAndPlay)
{

#ifndef DEW_THEMER_MODE

    LVITEM      lvItem { 0 };
    wchar_t     wsRawPath[MAX_CHAR_PATH]{ 0 };
    wchar_t     wsTitle[TAG_DATA_LENGTH]{ 0 };
    wchar_t     wsDuration[32]{ 0 }, wsMediaType[4]{ 0 };

    _ActiveItem.ResetData();
    _iCurrIndex = -1;

    if (iNewActiveIndex < 0 || iNewActiveIndex >= (int)this->GetFileCount())
    {
        if (bActivateAndPlay)
            _Parent->ProcessMessage(this, DEWUI_MSG_PL_SELECTION, 0, 0);
        return;
    }

    // Set the new index
    _iCurrIndex = iNewActiveIndex;

    lvItem.mask = LVIF_IMAGE;
    lvItem.iImage = 1;
    lvItem.iItem = _iCurrIndex;
    lvItem.iSubItem = _COL_SEQ_AUDIO_TITLE_DISPLAY;
    ListView_SetItem(_hLstItems, &lvItem);

    _ActiveItem.SetFromListView(_hLstItems, _iCurrIndex, _COL_SEQ_AUDIO_ITEM_ID,
                                _COL_SEQ_AUDIO_FILE_PATH, _COL_SEQ_AUDIO_TITLE_DISPLAY,
                                _COL_SEQ_AUDIO_DURATION_DISPLAY, _COL_SEQ_AUDIO_DURATION, 
                                _COL_SEQ_AUDIO_TITLE_PARSED, _COL_SEQ_AUDIO_TITLE_VALID, _COL_SEQ_AUDIO_MEDIA_TYPE);

    ListView_SetItemState(_hLstItems, -1, 0, 0x00F);
    ListView_SetItemState(_hLstItems, _iCurrIndex, LVIS_SELECTED | LVIS_FOCUSED, 0x00F);
    
    if (bActivateAndPlay)
        _Parent->ProcessMessage(this, DEWUI_MSG_PL_SELECTION, 0, (int)_iCurrIndex);
    // Focus to the activated item in playlist, only if the parent window is visible and
    // is currently the top-level window in focus.
    if (IsWindowVisible(_Parent->Handle) && GetForegroundWindow() == _Parent->Handle)
        SetFocus(_hLstItems);

#endif

    return;
}

void DEWPLAYLIST::DeactivateItem()
{

#ifndef DEW_THEMER_MODE
    _ActiveItem.ResetData();
    _iCurrIndex = _ActiveItem.Index;
#endif

    return;
}

UINT DEWPLAYLIST::GetFileCount()
{
    if (!_hLstItems || _hLstItems == INVALID_HANDLE_VALUE) return 0;
    return ListView_GetItemCount(_hLstItems);
}

UINT DEWPLAYLIST::GetSelectedCount()
{
    if (!_hLstItems || _hLstItems == INVALID_HANDLE_VALUE) return 0;
    return ListView_GetSelectedCount(_hLstItems);
}

UINT DEWPLAYLIST::GetFiles(LPFILELIST& lpFileList, UINT nFileCount)
{
    UINT        nLimit = 0, nTotalItems = 0, nIndex = 0;
    wchar_t     wsFileName[MAX_CHAR_PATH];
    LVITEM      lvItem { 0 };

    if (!lpFileList || nFileCount == 0) return 0;
    nTotalItems = this->GetFileCount();
    nLimit = nFileCount <= nTotalItems ? nFileCount : nTotalItems;
    nTotalItems = 0;

    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = _COL_SEQ_AUDIO_FILE_PATH;
    lvItem.pszText = wsFileName;
    lvItem.cchTextMax = MAX_CHAR_PATH;

    for (nIndex = 0; nIndex < nLimit; nIndex++)
    {
        ZeroMemory(wsFileName, MAX_CHAR_PATH * sizeof(wchar_t));
        lvItem.iItem = nIndex;
        if (ListView_GetItem(_hLstItems, &lvItem))
        {
            CopyMemory((lpFileList[nTotalItems].wsFileName), wsFileName, MAX_CHAR_PATH * sizeof(wchar_t));
            lpFileList[nTotalItems].next = NULL;
            nTotalItems++;
        }
    }
    return nTotalItems;
}

void DEWPLAYLIST::StartQueueProcessing()
{
    // If queuing thread is not already running, make sure to re-scan...
    if (!_bParsing)
        _hTrdQueue = CreateThread(NULL, 0, _TrdQueue, this, 0, NULL);
}

#ifndef DEW_THEMER_MODE

const PLAYLISTAUDIOITEM DEWPLAYLIST::GetItem(int iIndex)
{
    PLAYLISTAUDIOITEM   item;

    if (iIndex == _iCurrIndex) return ActiveItem;

    if (iIndex < 0 || iIndex >= ListView_GetItemCount(_hLstItems)) return item;

    item.SetFromListView(_hLstItems, iIndex, _COL_SEQ_AUDIO_ITEM_ID,
        _COL_SEQ_AUDIO_FILE_PATH, _COL_SEQ_AUDIO_TITLE_DISPLAY,
        _COL_SEQ_AUDIO_DURATION_DISPLAY, _COL_SEQ_AUDIO_DURATION,
        _COL_SEQ_AUDIO_TITLE_PARSED, _COL_SEQ_AUDIO_TITLE_VALID, _COL_SEQ_AUDIO_MEDIA_TYPE);
    return item;
}

const PLAYLISTAUDIOITEM DEWPLAYLIST::GetSelectedItem()
{
    PLAYLISTAUDIOITEM   item;

    if (ListView_GetSelectedCount(_hLstItems) <= 0) return item;
    return this->GetItem((int)ListView_GetNextItem(_hLstItems, -1, LVNI_SELECTED));
}

#endif

DEWPLAYLIST::~DEWPLAYLIST()
{
    this->_ClearQueueProcess();
    if (_hMnuContext) DestroyMenu(_hMnuContext);
    RemoveWindowSubclass(_hLstItems, _LstMsgHandler, (UINT_PTR)_hLstItems);
    DestroyWindow(_hLstItems);
    ImageList_RemoveAll(_hImgInd);
    DestroyIcon(_hIcoBlank);
    DestroyIcon(_hIcoInd);
    ImageList_Destroy(_hImgInd);
    if (_hFont) { DeleteObject(_hFont); _hFont = NULL; }
    if (_lpIconRepo) { delete _lpIconRepo; _lpIconRepo = NULL; }
    if (_pPenWindowOutline) { delete _pPenWindowOutline; _pPenWindowOutline = NULL; }
    return;
}

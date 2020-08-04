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

#ifndef _DEWPLAYLIST_H_
#define _DEWPLAYLIST_H_

#include "DewUIObject.h"
#include "DewPlaylistParser.h"
#include "DewIconRepository.h"

#ifndef DEW_THEMER_MODE
#include "../Engine/AudioFactory.h"

typedef struct PLAYLISTAUDIOITEM
{
    private:
        wchar_t         _wsID[64]{ 0 };
        wchar_t         _wsFilePath[MAX_CHAR_PATH]{ 0 };
        wchar_t         _wsTitle[TAG_DATA_LENGTH]{ 0 };
        DWORD           _dwDuration = 0;
        BOOL            _bParsed = FALSE;
        BOOL            _bValid = FALSE;
        BYTE            _btMediaType = DEWMT_UNKNOWN;
        
        HWND            _hWndLVSrc = NULL;
        int             _iRowIndex = -1;
        int             _iColIndexID = -1;
        int             _iColIndexFilePath = -1;
        int             _iColIndexTitle = -1;
        int             _iColIndexDurationDisplay = -1;
        int             _iColIndexDuration = -1;
        int             _iColIndexParsed = -1;
        int             _iColIndexValid = -1;
        int             _iColIndexMediaType = -1;


    public:
        const wchar_t*  ID;
        const wchar_t*  FilePath;
        const wchar_t*  Title;
        const DWORD&    Duration;
        const BOOL&     Parsed;
        const BOOL&     Valid;
        const BYTE&     MediaType;
        const int&      Index;

                        PLAYLISTAUDIOITEM();
                        PLAYLISTAUDIOITEM(const PLAYLISTAUDIOITEM& PLAudioItem);
                        PLAYLISTAUDIOITEM(HWND hWndLVSrc, const int iRow, int iColIndexID, int iColIndexFilePath, int iColIndexTitle, int iColIndexDurationDisplay, int iColIndexDuration, int iColIndexParsed, int iColIndexValid, int iColIndexMediaType);
        void            operator= (const PLAYLISTAUDIOITEM& PLAudioItem);
        void            SetFromListView(HWND hWndLVSrc, const int iRow, int iColIndexID, int iColIndexFilePath, int iColIndexTitle, int iColIndexDurationDisplay, int iColIndexDuration, int iColIndexParsed, int iColIndexValid, int iColIndexMediaType);
        void            AutoAdjustIndex();
        void            UpdateAll(LPCWSTR wsTitle, DWORD dwDuration, BOOL bParsed, BOOL bValid, BYTE btMediaType);
        void            UpdateTitle(LPCWSTR wsTitle);
        void            UpdateDuration(DWORD dwDuration);
        void            UpdateParsed(BOOL bParsed = TRUE);
        void            UpdateValid(BOOL bValid);
        void            UpdateMediaType(BYTE btMediaType);
        void            ResetData();

} PLAYLISTAUDIOITEM, *LPPLAYLISTAUDIOITEM;

#endif

typedef class DEWPLAYLIST : public DEWUIOBJECT
{
    protected:
        typedef struct STRSOURCEQUEUE
        {
            wchar_t                 wsPath[MAX_CHAR_PATH];
            BOOL                    bDir;
            struct STRSOURCEQUEUE   *next;
        } SOURCEQUEUE, *LPSOURCEQUEUE;

        const int           _COL_SEQ_AUDIO_ITEM_ID = 0; // Internal ID of each row
        const int           _COL_SEQ_AUDIO_FILE_PATH = 1; // Full path to the file [Hidden Column]
        const int           _COL_SEQ_AUDIO_TITLE_DISPLAY = 2; // Song Title [Visible Column]
        const int           _COL_SEQ_AUDIO_DURATION_DISPLAY = 3; // MM:SS formatted duration [Visible Column]
        const int           _COL_SEQ_AUDIO_DURATION= 4; // Actial ms duration [Hidden Column]
        const int           _COL_SEQ_AUDIO_TITLE_PARSED = 5; // '1': Parsed, '0: Not Parsed [Hidden Column]
        const int           _COL_SEQ_AUDIO_TITLE_VALID = 6; // '1': OK, '0': Not OK, '' (blank): Not yet parsed [Hidden Column]
        const int           _COL_SEQ_AUDIO_MEDIA_TYPE = 7; // 'DEWUI_MT_xxxx' [Hidden Column]
        const wchar_t       *_ID_PREFIX = L"ID-";
        int                 _iLastRow = 0;

        HWND                _hLstItems = NULL;
        HFONT               _hFont = NULL;
        HMENU               _hMnuContext = NULL;
        HANDLE              _hTrdQueue = NULL;
        HIMAGELIST          _hImgInd = NULL;
        HICON               _hIcoBlank = NULL;
        HICON               _hIcoInd = NULL;
        Pen                 *_pPenWindowOutline = NULL;
        LPSOURCEQUEUE       _lpQueueStart = NULL;
        LPSOURCEQUEUE       _lpQueueEnd = NULL;
        LPDEWICONREPOSITORY _lpIconRepo = NULL;
        DEWPLAYLISTPARSER   _Parser;
    #ifndef DEW_THEMER_MODE
        PLAYLISTAUDIOITEM   _ActiveItem;
    #endif
        const DWORD         _PLSTYLE = WS_CHILD | WS_VISIBLE;
#ifdef _DEBUG
        const DWORD         _LSTSTYLE = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | WS_HSCROLL | LVS_ICON;
#else
        const DWORD         _LSTSTYLE = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | WS_HSCROLL | LVS_ICON | LVS_NOCOLUMNHEADER;
#endif
        BOOL                _bParsing = FALSE;
        const BOOL          _bDeepScan;
        int                 _iCurrIndex = -1;

    #ifdef DEW_THEMER_MODE
        // Set a dummy list for illustration purposes
        virtual void        _SetDummyList();
    #endif

        static DWORD        WINAPI _TrdQueue(LPVOID lpv);
        static LRESULT      CALLBACK _LstMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);

        virtual void        _ContextMenu(LONG iX, LONG iY);
        virtual void        _DeleteItems(BOOL bDeleteAll = FALSE);
        virtual void        _ExportItems(LPCWSTR wsExportFile, BYTE btExportFormat, BOOL bUseRelativePaths, BOOL bExportAll);
        virtual void        _ClearQueueProcess();
        virtual void        _AddSource(LPCWSTR wsPath, BOOL bDir);
        virtual void        _Enqueue(LPCWSTR wsDir);
        virtual void        _ParseFileInfoAndAdd(LPCWSTR wsFileName);
        virtual void        _OpenSelFileLocation(int iIndex = -1);
        virtual void        _ParseCompleteEvent();
        virtual void        _AdjustColumnWidths();
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _Draw(HDC);
        virtual void        _PrepareImages();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT     _LstWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _MoveToDefaultLocation();

    public:
        const BOOL&         QueuingInProgress;
        const int&          ActiveItemIndex;
    #ifndef DEW_THEMER_MODE
        PLAYLISTAUDIOITEM&  ActiveItem;
    #endif

                            DEWPLAYLIST(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BOOL bDeepScan);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual void        Add(LPCWSTR wsPath, BOOL bFlushAll = FALSE);
        virtual void        AddFileDirect(LPCWSTR wsPath, BOOL bFlushAll = TRUE); // Directly add one file, without threading
        virtual void        SelectAllItems();
        virtual void        RemoveAllItems();
        virtual void        ActivateItem(int iNewActiveIndex, BOOL bActivateAndPlay = TRUE);
        virtual void        DeactivateItem();
        virtual UINT        GetFileCount();
        virtual UINT        GetSelectedCount();
        virtual UINT        GetFiles(LPFILELIST& lpFileList, UINT nFileCount);
        virtual void        StartQueueProcessing();
    #ifndef DEW_THEMER_MODE
        virtual const PLAYLISTAUDIOITEM GetItem(int iIndex);
        virtual const PLAYLISTAUDIOITEM GetSelectedItem();
    #endif
        virtual             ~DEWPLAYLIST();
} DEWPLAYLIST, *LPDEWPLAYLIST;

#endif // _DEWPLAYLIST_H_



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

#ifndef _DEWWINDOW_H_
#define _DEWWINDOW_H_

#include "DewButtonMinimize.h"
#include "DewButtonClose.h"

#include "DewAlbumArt.h"
#include "DewPlaylist.h"
#include "DewPropertyPage.h"
#include "DewSeekbar.h"
#include "DewInfoLabel.h"

#include "DewButtonDMPlayPause.h"
#include "DewButtonStop.h"
#include "DewButtonPrev.h"
#include "DewButtonNext.h"
#include "DewButtonVolume.h"
#include "DewButtonRepeat.h"
#include "DewButtonPlaylist.h"
#include "DewButtonBrowse.h"
#include "DewButtonSettings.h"
#include "DewButtonExInfo.h"
#include "DewButtonTimer.h"
#include "DewButtonDewMode.h"

#include "DewLayoutViewer.h"
#include "DewShutdownTimer.h"
#include "DewPlaylistExporter.h"
#include "DewDiscSelector.h"
#include "DewIconRepository.h"

#ifndef DEW_THEMER_MODE
#include "DewAboutWindow.h"
#endif

typedef class DEWWINDOW : public DEWUIOBJECT
{
    protected:
        
        // 'EP' prefixes indicate Engine Specific Properties
        // 'EA' prefixes indicate Engine Specific Actions
        BOOL                    _bMinToTray = FALSE;
        BOOL                    _bCloseToTray = FALSE;
        BOOL                    _bLayoutMode = FALSE;
        Color                   _crBack2;
        RECT                    _rctTitleBar;
        PointF                  _ptIconStart;
        DEWSETTINGS&            _PlayerSettings;
        int                     _iTrimmedHeight;
        int                     _iDewModeWidth, _iDewModeHeight;
        BYTE                    _btActiveLayoutObjectID;
        BYTE                    _btDewModePos;
        HANDLE                  _hMap = NULL;
        HICON                   _hIcon = NULL;
        HMENU                   _hMnuTray = NULL;
        HMENU                   _hMnuContext = NULL;
        HMENU                   _hMnuOpen = NULL;
        HMENU                   _hMnuShow = NULL;
        HMENU                   _hMnuHide = NULL;
        HMENU                   _hMnuSystem = NULL;
        Font                    *_pFntIcon = NULL;
        Bitmap                  *_pImgIcon = NULL;
        NOTIFYICONDATA          _nidTray;
        DEWUICOMPONENT          _ducComponents[DEWUI_OBJECT_COUNT];
        DWORD                   _lpdwLayoutPos[DEWUI_OBJECT_COUNT];
        BOOL                    _lpbLayoutVisibility[DEWUI_OBJECT_COUNT];
        LPMAPCONTENT            _lpMapContent = NULL;
        BOOL                    _bShowTitle = TRUE;
#ifndef DEW_THEMER_MODE
        HACCEL                  _hAccelHK = NULL;
        BOOL                    _b_EP_DeepScan = FALSE;
        const DWORD             _dw_EP_PollInterval = 250;
        HANDLE                  _hTrd_EP_Poll = NULL;
        LPIAUDIOENGINE          _lpAudioEngine = NULL;
        LPDEWABOUTWINDOW        _lpAboutWindow = NULL;
#endif

        LPDEWMINIMIZEBUTTON     _lpCmdMinimize = NULL;
        LPDEWCLOSEBUTTON        _lpCmdClose = NULL;

        LPDEWALBUMART           _lpAlbumArt = NULL;
        LPDEWPLAYLIST           _lpPlaylist = NULL;
        LPDEWSEEKBAR            _lpSeekBar = NULL;
        LPDEWINFOLABEL          _lpTitleInfo = NULL;
        LPDEWINFOLABEL          _lpArtistInfo = NULL;
        LPDEWINFOLABEL          _lpAlbumInfo = NULL;

        LPDEWPLAYPAUSEBUTTON    _lpCmdPlayPause = NULL;
        LPDEWSTOPBUTTON         _lpCmdStop = NULL;
        LPDEWPREVIOUSBUTTON     _lpCmdPrev = NULL;
        LPDEWNEXTBUTTON         _lpCmdNext = NULL;
        LPDEWVOLUMEBUTTON       _lpCmdVolume = NULL;
        LPDEWREPEATBUTTON       _lpCmdRepeat = NULL;
        LPDEWPLAYLISTBUTTON     _lpCmdPlaylist = NULL;
        LPDEWBROWSEBUTTON       _lpCmdBrowse = NULL;
        LPDEWSETTINGSBUTTON     _lpCmdSettings = NULL;
        LPDEWEXINFOBUTTON       _lpCmdExInfo = NULL;
        LPDEWTIMERBUTTON        _lpCmdTimer = NULL;
        LPDEWDEWMODEBUTTON      _lpCmdDewMode = NULL;

        LPDEWLAYOUTVIEWER       _lpLayoutViewer = NULL;

        LPDEWDMPLAYPAUSEBUTTON  _lpCmdDMPlayPause = NULL; // This will not be part fo theming
        LPDEWPROPERTYPAGE       _lpPropPage = NULL;
        LPDEWSHUTDOWNTIMER      _lpShutdownTimer = NULL;
        LPDEWPLAYLISTEXPORTER   _lpPlaylistExporter = NULL;
        LPDEWDISCSELECTOR       _lpDiscSelector = NULL;
    
        LPDEWICONREPOSITORY     _lpIconRepo = NULL;

    #ifndef DEW_THEMER_MODE
        // These are all engine specific actions
        // We'll exclude them from the 'Themer' mode
        static DWORD WINAPI     _EA_Trd_PollInfo(LPVOID lpv);
        virtual void            _EA_PollInfo();
        virtual void            _EA_FlushEngine();
        virtual int             _EA_GetPrevTrackIndex(int iCurrentIndex, BOOL bManual);
        virtual int             _EA_GetNextTrackIndex(int iCurrentIndex, BOOL bManual);
        virtual void            _EA_HandlePlayPause(BOOL bManual);
        virtual void            _EA_HandleStop();
        virtual void            _EA_HandlePrevious(BOOL bManual);
        virtual void            _EA_HandleNext(BOOL bManual);
        virtual void            _EA_HandleSeek(DWORD dwSeekTime);
        virtual void            _EA_HandleVolume(WORD wNewVolLevel);
    #endif

        virtual BOOL            _GetSharedMemFileName(LPWSTR wsPath);
        virtual void            _CreateUIComponents();
        virtual void            _SetLayoutMode();
        virtual void            _ResetLayout();
        virtual void            _SetDewMode(BOOL bDewMode = TRUE);
        virtual void            _SnapDewModeWindow(BYTE btVK_Key = 0);
        virtual void            _ShowFullView(BOOL bFullView);
        virtual void            _TrayMenu(int iX, int iY);
        virtual void            _ContextMenu(int iX, int iY);
        virtual void            _CreateOpenMenu();
        virtual void            _HideToTray();
        virtual void            _RestoreFromTray();
        virtual void            _Draw(HDC);
        virtual void            _BrowseAndSetFiles();
        virtual void            _BrowseAndSetFolder();
        virtual void            _BrowseAndSetDisc(LPCWSTR wsDiscPath);
        virtual void            _ShowAboutScreen();
        virtual LRESULT         _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void            _ApplySettings(const DEWSETTINGS& AppSettings);
        virtual void            _Scale(BYTE btIconScale);
        virtual void            _PrepareImages();
        virtual void            _MoveToDefaultLocation();

    public:
    #ifndef DEW_THEMER_MODE
        const HACCEL&           AccelHotKey;
                                DEWWINDOW(DEWSETTINGS& AppSettings, BOOL bPrepPopulateOverride = FALSE);
    #else
                                DEWWINDOW(DEWSETTINGS& AppSettings, HWND hWndContainer);
    #endif
        virtual void            ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void            Move(int iXPos, int iYPos, BOOL bCentered = FALSE); // No-Action. Override
        virtual void            Scale(BYTE btIconScale); // No Action. Override.
        virtual void            SetLocation(DWORD dwPresetLocation);
        virtual void            ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual                 ~DEWWINDOW();
} DEWWINDOW, *LPDEWWINDOW;

#endif // DEWWWINDOW_H_

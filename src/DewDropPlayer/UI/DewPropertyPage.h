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

#ifndef _DEWPROPERTYPAGE_H_
#define _DEWPROPERTYPAGE_H_

#ifndef UNICODE
#define UNICODE
#endif // UNICODE

#include "DewUIObject.h"

// Do not inherit this, please
typedef class DEWPROPERTYPAGE
{
    protected:
        const int       _GRP_FILE_INFO = 10;
        const int       _GRP_AUDIO_INFO = 20;
        const int       _GRP_TAG_INFO = 30;
        const int       _GRP_CODEC_INFO = 40;
        const int       _PREFIX_INDEX_FI_FILE_NAME = 0x00;
        const int       _PREFIX_INDEX_FI_FILE_SIZE = 0x01;
        const int       _PREFIX_INDEX_FI_FILE_PATH = 0x02;
        const int       _PREFIX_INDEX_FI_FILE_DIR = 0x03;
        const int       _PREFIX_INDEX_AI_CHANNELS = 0x04;
        const int       _PREFIX_INDEX_AI_DURATION = 0x05;
        const int       _PREFIX_INDEX_AI_SAMPLE_RATE = 0x06;
        const int       _PREFIX_INDEX_AI_BITRATE = 0x07;
        const int       _PREFIX_INDEX_TI_TITLE = 0x08;
        const int       _PREFIX_INDEX_TI_ARTIST = 0x09;
        const int       _PREFIX_INDEX_TI_ALBUM = 0x0A;
        const int       _PREFIX_INDEX_TI_TRACK = 0x0B;
        const int       _PREFIX_INDEX_TI_GENRE = 0x0C;
        const int       _PREFIX_INDEX_TI_YEAR = 0x0D;
        const int       _PREFIX_INDEX_CI_AUDIO_TYPE = 0x0E;
        const int       _PREFIX_INDEX_CI_DRLIB = 0x0F;
        // Marker index from where the codec specific info starts
        const int       _PREFIX_INDEX_CODEC_GRP = 0x0E;

        HINSTANCE       _hInstance = NULL;
        HWND            _hWndParent = NULL;
        HWND            _hWndProperty = NULL;
        HWND            _hLstMain = NULL;
        HWND            _hCmdOK = NULL;
        HFONT           _hFntUI = NULL;
        HFONT           _hFntIcon = NULL;
        HBRUSH          _hbrWndBack = NULL, _hbrBtnBack = NULL;
        HPEN            _hPenBtnFocus = NULL;
        HPEN            _hPenLstBorder = NULL, _hPenWndBorder = NULL, _hPenBtnBorder = NULL;
        COLORREF        _crBtnText = 0x00;
        UINT            _nPadding;
        int             _iDPI = 96;
        float           _fScale = 1.0f;
        const WORD      _DEFWIDTH = 360;
        const WORD      _DEFHEIGHT = 480;
        int             _iMinWidth = _DEFWIDTH;
        int             _iMinHeight = _DEFHEIGHT;

        static LRESULT  CALLBACK _DewPropPageMsgHandler(HWND hWnd, UINT nmsg, WPARAM wParam,
                                                        LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        LRESULT         _WndProc(HWND hWnd, UINT nmsg, WPARAM wParam, LPARAM lParam);
        void            _ResetGDIObjects();
        void            _Draw(HDC hDC);
        void            _DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText);
        void            _SetInfoValue(int iIndex, LPCWSTR wsValue);
        void            _SetUIFont(const wchar_t *wsFontName, int iFontSize);
        void            _HandleSizing();

    public:
                        DEWPROPERTYPAGE(HWND hWndParent, const DEWSETTINGS& settings);
        HWND            Show(const DEWTHEME& Theme);

        void            Set_FileInfo(LPCWSTR wsFile);
        
        void            Set_AudioInfo_Channels(BYTE btChannels);
        void            Set_AudioInfo_Length(DWORD dwDuration);
        void            Set_AudioInfo_Frequency(DWORD dwFrequency);
        void            Set_AudioInfo_Bitrate(DWORD dwBitrate);
        void            Set_AudioInfo(BYTE btChannels, DWORD dwDuration, DWORD dwFrequency, DWORD dwBitrate);

        void            Set_TagInfo_Title(LPCWSTR wsTitle, BOOL bSimulatedTitle);
        void            Set_TagInfo_Artist(LPCWSTR wsArtist);
        void            Set_TagInfo_Album(LPCWSTR wsAlbum);
        void            Set_TagInfo_TrackNo(LPCWSTR wsTrackNo);
        void            Set_TagInfo_Genre(LPCWSTR wsGenre);
        void            Set_TagInfo_Year(LPCWSTR wsYear);
        void            Set_TagInfo(LPCWSTR wsTitle, BOOL bSimulatedTitle, LPCWSTR wsArtist,
                                    LPCWSTR wsAlbum, LPCWSTR wsTrackNo, LPCWSTR wsGenre, LPCWSTR wsYear);

        void            Set_CodecAudioType(LPCWSTR wsAudioType);
        void            Set_CodecLibrary(LPCWSTR wsLibrary);
        void            Clear_CodecInfo();
        void            Add_CodecInfo(LPCWSTR wsKey, LPCWSTR wsValue);
        void            Reset();

                        ~DEWPROPERTYPAGE();

} DEWPROPERTYPAGE, *LPDEWPROPERTYPAGE;

#endif

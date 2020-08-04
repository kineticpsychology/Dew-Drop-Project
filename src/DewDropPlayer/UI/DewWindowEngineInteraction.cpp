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

// This file specifically deals with the window's interaction
// with the audio engine. Please do not make any UI changes here.
// UI specific changes should be placed in the DewWindow.cpp file.
#ifndef DEW_THEMER_MORE

#include "DewWindow.h"

DWORD WINAPI DEWWINDOW::_EA_Trd_PollInfo(LPVOID lpv)
{
    ((LPDEWWINDOW)lpv)->_EA_PollInfo();
    return 0x00;
}

void DEWWINDOW::_EA_PollInfo()
{
    do
    {
        if ( (_lpAudioEngine != NULL) &&
             (_lpAudioEngine->Status == DEWS_MEDIA_PLAYING || _lpAudioEngine->Status == DEWS_MEDIA_PAUSED) )
        {
            _lpSeekBar->SetTime(_lpAudioEngine->Tell());
            _lpPropPage->Set_AudioInfo_Bitrate(_lpAudioEngine->Bitrate);
        }
        Sleep(_dw_EP_PollInterval);
    } while (_hTrd_EP_Poll != NULL);
    return;
}

// Quick, re-usable function
void DEWWINDOW::_EA_FlushEngine()
{
    if (_lpAudioEngine != NULL)
    {
        _lpAudioEngine->Stop();
        _lpAudioEngine->Unload();
        delete _lpAudioEngine;
        _lpAudioEngine = NULL;
    }

    if (_hTrd_EP_Poll)
    {
        TerminateThread(_hTrd_EP_Poll, 0x00);
        _hTrd_EP_Poll = NULL;
    }
    return;
}

// Util function to figure out the 'Previous' track index w.r.t. the 'Repeat' button settings
int DEWWINDOW::_EA_GetPrevTrackIndex(int iCurrentIndex, BOOL bManual)
{
    int         iPrevIndex = -1;

    if (iCurrentIndex < 0) return iPrevIndex;

    // Find the 'Next' index based on the repeat state
    switch (_lpCmdRepeat->State)
    {
        case DEWOPT_REPEAT_OFF:
        {
            // Proceed till the first item only.
            if (iCurrentIndex > 0)
                iPrevIndex = iCurrentIndex - 1;
            // If explicitly asked for, by the user, then allow to jump
            // back to the last index
            if (bManual && iCurrentIndex == 0)
                iPrevIndex = ((int)_lpPlaylist->GetFileCount() - 1);
            break;
        }
        case DEWOPT_REPEAT_ALL:
        {
            iPrevIndex = iCurrentIndex - 1;
            // If the end has reached, cycle back to the beginning as we are repeating all
            if (iPrevIndex < 0)
                iPrevIndex = (int)_lpPlaylist->GetFileCount() - 1;
            break;
        }
        case DEWOPT_REPEAT_ONE:
        {
            // Same audio
            if (!bManual)
            {
                iPrevIndex = iCurrentIndex;
            }
            else
            {
                iPrevIndex = iCurrentIndex - 1;
                // If the end has reached, cycle back to the beginning as we are repeating all
                if (iPrevIndex < 0)
                    iPrevIndex = (int)_lpPlaylist->GetFileCount() - 1;
            }
            break;
        }
        default:
        {
            iPrevIndex = -1;
            break;
        }
    }
    return iPrevIndex;
}

// Util function to figure out the 'Next' track index w.r.t. the 'Repeat' button settings
int DEWWINDOW::_EA_GetNextTrackIndex(int iCurrentIndex, BOOL bManual)
{
    int         iNextIndex = -1;

    if (iCurrentIndex < 0) return iNextIndex;

    // Find the 'Next' index based on the repeat state
    switch (_lpCmdRepeat->State)
    {
        case DEWOPT_REPEAT_OFF:
        {
            // Proceed till the last item only.
            if (iCurrentIndex < (int)_lpPlaylist->GetFileCount() - 1)
                iNextIndex = iCurrentIndex + 1;
            // If explicitly asked for, by the user, then allow to jump
            // back to the first index
            if (bManual && iCurrentIndex == ((int)_lpPlaylist->GetFileCount() - 1))
                iNextIndex = 0;
            break;
        }
        case DEWOPT_REPEAT_ALL:
        {
            iNextIndex = iCurrentIndex + 1;
            // If the end has reached, cycle back to the beginning as we are repeating all
            if (iNextIndex >= (int)_lpPlaylist->GetFileCount())
                iNextIndex = 0;
            break;
        }
        case DEWOPT_REPEAT_ONE:
        {
            // Same audio
            if (!bManual)
            {
                iNextIndex = iCurrentIndex;
            }
            else // behave in the same as DEWOPT_REPEAT_ALL
            {
                iNextIndex = iCurrentIndex + 1;
                // If the end has reached, cycle back to the beginning as we are repeating all
                if (iNextIndex >= (int)_lpPlaylist->GetFileCount())
                    iNextIndex = 0;
            }
            break;
        }
        default:
        {
            iNextIndex = -1;
            break;
        }
    }
    return iNextIndex;
}

// Main *Star* function to handle the playing/pausing/resuming
void DEWWINDOW::_EA_HandlePlayPause(BOOL bManual)
{
    int                     iIndex = -1, iNextIndex = -1;
    BOOL                    bPreActivated = FALSE, bReady = FALSE;
    PLAYLISTAUDIOITEM       pliNextItem;
    wchar_t                 wsAudioFormat[TAG_DATA_LENGTH];


    _lpCmdPlayPause->SetTip(L"Ready to play");

    // Toggling state: If a song is playing/paused, it will be
    //                 paused/resumed respectively. Nothing more
    //                 will be done in those situations.
    if (_lpAudioEngine != NULL &&
        _lpAudioEngine->Status == DEWS_MEDIA_PLAYING)
    {
        if (_lpAudioEngine->Pause() == DEWERR_SUCCESS)
        {
            _lpCmdPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);
            _lpCmdDMPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);
            _lpCmdPlayPause->SetTip(L"Resume");
        }
        return;
    }
    else if (_lpAudioEngine != NULL &&
             _lpAudioEngine->Status == DEWS_MEDIA_PAUSED)
    {
        if (_lpAudioEngine->Resume() == DEWERR_SUCCESS)
        {
            _lpCmdPlayPause->SetState(DEWUI_STATE_PLAYING);
            _lpCmdDMPlayPause->SetState(DEWUI_STATE_PLAYING);
            _lpCmdPlayPause->SetTip(L"Pause");
        }
        return;
    }

    // Figure out the 'current' and 'next' song indexes
    // Try seeing the activated item
    iIndex = _lpPlaylist->ActiveItemIndex;
    // If unavailable, try selecting the 'selected' item index
    if (iIndex < 0)
    {
        iIndex = _lpPlaylist->GetSelectedItem().Index;
        // If nothing is selected, try picking the first file from the index
        if (iIndex < 0 && _lpPlaylist->GetFileCount() > 0)
        {
            iIndex = 0;
        }
    }
    else
    {
        // The item is already activated. We will not re-activate it unnecessarily
        bPreActivated = TRUE;
    }
    
    // If still nothing, bail
    if (iIndex < 0) return;

    this->_EA_FlushEngine();

    // Activate the item (without playing), if not already activated
    // Be Careful! Do not miss the second 'FALSE' argument, or this will go into an eternal recursive loop!
    if (!bPreActivated) _lpPlaylist->ActivateItem(iIndex, FALSE);

    // Proceed only if the file is valid
    if (_lpPlaylist->ActiveItem.Valid)
    {
        AUDIOFACTORY::GetAudioEngine(_lpPlaylist->ActiveItem.FilePath, &_lpAudioEngine);
        // Proceed only if engine was found
        if (_lpAudioEngine)
        {
            // Proceed only if the file gets loaded successfully
            if (_lpAudioEngine->Load(_hWnd, _lpPlaylist->ActiveItem.FilePath) == DEWERR_SUCCESS)
            {
                bReady = TRUE;
            }
            // Otherwise, flag them
            else
            {
                _lpPlaylist->ActiveItem.UpdateParsed();
                _lpPlaylist->ActiveItem.UpdateValid(FALSE);
            }
        }
    }

    // If file could not be loaded, try the next file
    if (!bReady)
    {
        iNextIndex = this->_EA_GetNextTrackIndex(iIndex, bManual);
        _lpPlaylist->DeactivateItem();
        this->_EA_FlushEngine();
        // If the 'next' file points back to the same 'broken' file
        // or if the 'next' index is invalid, then bail
        if (iNextIndex == iIndex || iNextIndex < 0) return;
        _lpPlaylist->ActivateItem(iNextIndex, FALSE);
        this->_EA_HandlePlayPause(bManual);
        return;
    }
    
    // Update the Playlist columns, even if they are already parsed
    // This will take care of situations where the tag library
    // might not have to been able to get the length, but the
    // dedicated library engine was able to do it, 
    // after calling the 'Load()' function.
    _lpPlaylist->ActiveItem.UpdateAll(_lpAudioEngine->Tag.Title, _lpAudioEngine->Duration, TRUE, TRUE, _lpAudioEngine->MediaType);
    _lpPlaylist->ActiveItem.UpdateValid(TRUE);

    // Just before starting to play, make sure the volume level is consulted
    // We'll not capture the error here
    _lpAudioEngine->SetVolume((WORD)_lpCmdVolume->VolumeLevel);

    // Update the icons on the two Play/Pause buttons
    if (_lpAudioEngine->Play() == DEWERR_SUCCESS)
    {
        _lpCmdPlayPause->SetState(DEWUI_STATE_PLAYING);
        _lpCmdDMPlayPause->SetState(DEWUI_STATE_PLAYING);
        _lpCmdPlayPause->SetTip(L"Pause");
    }
    else
    {
        _lpCmdPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);
        _lpCmdDMPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);
    }

    // Update the File/Tag/Codec information section
    _lpPropPage->Set_FileInfo(_lpAudioEngine->SourceFileName);
    _lpPropPage->Set_AudioInfo(_lpAudioEngine->Channels, _lpAudioEngine->Duration, _lpAudioEngine->SampleRate, _lpAudioEngine->Bitrate);
    _lpPropPage->Set_TagInfo(_lpAudioEngine->Tag.Title, _lpAudioEngine->Tag.SimulatedTitle, _lpAudioEngine->Tag.Artist, _lpAudioEngine->Tag.Album, _lpAudioEngine->Tag.Track, _lpAudioEngine->Tag.Genre, _lpAudioEngine->Tag.Year);
    ZeroMemory(wsAudioFormat, TAG_DATA_LENGTH * sizeof(wchar_t));
    IAUDIOENGINE::GetAudioFormatString(_lpAudioEngine->MediaType, wsAudioFormat, TAG_DATA_LENGTH);
    _lpPropPage->Set_CodecAudioType(wsAudioFormat);
    _lpPropPage->Set_CodecLibrary(_lpAudioEngine->Library);
    _lpPropPage->Clear_CodecInfo();
    for (DWORD dwProp = 0; dwProp < _lpAudioEngine->ExtendedInfoCount; dwProp++)
        _lpPropPage->Add_CodecInfo(_lpAudioEngine->ExtendedInfo[dwProp].Type, _lpAudioEngine->ExtendedInfo[dwProp].Value);

    // Update the Album Art
    _lpAlbumArt->SetAlbumArt(_lpAudioEngine->Tag.AlbumArt);

    // Update the UI Info Blocks
    _lpTitleInfo->SetInfo(_lpAudioEngine->Tag.Title);
    _lpArtistInfo->SetInfo(_lpAudioEngine->Tag.Artist);
    _lpAlbumInfo->SetInfo(_lpAudioEngine->Tag.Album);

    // Set the Seekbar
    _lpSeekBar->SetLength(_lpAudioEngine->Duration);
    _hTrd_EP_Poll = CreateThread(NULL, 0x00, _EA_Trd_PollInfo, this, 0, NULL);

    // Set the Title and redraw the title-bar
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, L"%s [%s]", _lpAudioEngine->Tag.Title, DEWUI_CAP_DEFAULT);
    SetWindowText(_hWnd, _wsText);
    InvalidateRect(_hWnd, &_rctTitleBar, TRUE);

    // Finally, if it is in tray, alter the title
    if (!IsWindowVisible(_hWnd))
    {
        if (_PlayerSettings.NotifyOnSongChange == 1)
        {
            _nidTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
            StringCchPrintf(_nidTray.szInfoTitle, 64, L"Now Playing...");
            StringCchPrintf(_nidTray.szInfo, 256, _lpAudioEngine->Tag.Title);
            _nidTray.dwInfoFlags = NIIF_INFO;
        }
        else
        {
            _nidTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        }
        StringCchPrintf(_nidTray.szTip, 128, _wsText);
        Shell_NotifyIcon(NIM_MODIFY, &_nidTray);
    }
    return;
}

// Along with a regular 'Stop' function, this is also
// a UI reset function
void DEWWINDOW::_EA_HandleStop()
{
    this->_EA_FlushEngine();

    _lpPlaylist->DeactivateItem();

    // Set the icons to 'Play'
    _lpCmdPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);
    _lpCmdDMPlayPause->SetState(DEWUI_STATE_NOT_PLAYING);

    // Set the generic tooltip
    _lpCmdPlayPause->SetTip(L"Ready to play");

    // Clear the seekbar
    _lpSeekBar->Reset();

    // Clear the Property Page
    _lpPropPage->Reset();

    // Clear the Album Art
    _lpAlbumArt->SetAlbumArt(NULL);

    // Clear the UI Info Blocks
    _lpTitleInfo->SetInfo(NULL);
    _lpArtistInfo->SetInfo(NULL);
    _lpAlbumInfo->SetInfo(NULL);

    // Clear the header & redraw
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, L"%s", DEWUI_CAP_DEFAULT);
    SetWindowText(_hWnd, _wsText);
    InvalidateRect(_hWnd, &_rctTitleBar, TRUE);

    // Finally, if it is in tray, alter the title
    if (!IsWindowVisible(_hWnd))
    {
        StringCchPrintf(_nidTray.szTip, 128, _wsText);
        Shell_NotifyIcon(NIM_MODIFY, &_nidTray);
    }
    return;
}

// Find the previous track, stop the currently playing and jump to that track
void DEWWINDOW::_EA_HandlePrevious(BOOL bManual)
{
    int                 iIndex, iPrevIndex;
    PLAYLISTAUDIOITEM   prevItem;

    iIndex = _lpPlaylist->ActiveItem.Index;
    // Proceed only if a valid index is there
    if (iIndex >= 0)
    {
        // Bring playback to a halt and start finding the previous track
        this->_EA_HandleStop();
        iPrevIndex = iIndex;

        // Keep checking until a valid 'previous' file is found
        // or the previous track cycles back to the current track
        do
        {
            iPrevIndex = this->_EA_GetPrevTrackIndex(iPrevIndex, bManual);
            prevItem = _lpPlaylist->GetItem(iPrevIndex);
        } while (!(prevItem.Valid) && iPrevIndex != iIndex);

        // Activate and play only if the track has a valid file
        if (prevItem.Valid)
            _lpPlaylist->ActivateItem(iPrevIndex, TRUE);
    }
    return;
}

// Find the next track, stop the currently playing and jump to that track
void DEWWINDOW::_EA_HandleNext(BOOL bManual)
{
    int                 iIndex, iNextIndex;
    PLAYLISTAUDIOITEM   nextItem;

    iIndex = _lpPlaylist->ActiveItem.Index;
    
    // Bring playback to a halt and start finding the next track
    this->_EA_HandleStop();

    // Proceed only if a valid index is there
    if (iIndex >= 0)
    {
        iNextIndex = iIndex;

        // Keep checking until a valid 'next' file is found
        // or the next track cycles back to the current track
        do
        {
            iNextIndex = this->_EA_GetNextTrackIndex(iNextIndex, bManual);
            if (iNextIndex >= 0)nextItem = _lpPlaylist->GetItem(iNextIndex);
        } while (!(nextItem.Valid) && iNextIndex != iIndex && iNextIndex != -1);

        // Activate and play only if the track has a valid file
        if (nextItem.Valid)
            _lpPlaylist->ActivateItem(iNextIndex, TRUE);
    }
    return;
}

// Simple method to execute the seeking using the engine
void DEWWINDOW::_EA_HandleSeek(DWORD dwSeekTime)
{
    if (_lpAudioEngine)
    {
        if (_lpAudioEngine->Seek(dwSeekTime) == DEWERR_SUCCESS)
        {
            _lpSeekBar->SetTime(dwSeekTime);
        }
    }
    return;
}

void DEWWINDOW::_EA_HandleVolume(WORD wNewVolLevel)
{
    if (_lpAudioEngine)
    {
        if (_lpAudioEngine->SetVolume(wNewVolLevel) == DEWERR_SUCCESS)
        {
            this->_PlayerSettings.VolumeLevel = wNewVolLevel;
            _lpCmdVolume->SetVolumeLevel(wNewVolLevel);
        }
    }
    return;
}

// This will the main inter-DEWUIOBJECT message handler
// This has been kept separate to logically isolate the engine interaction part
void DEWWINDOW::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue)
{
    RECT        rctOpenBtn;

    // The below if-clauses are arranged in the same order as their
    // respective object IDs. Hope it helps out for easy reading/reference
    
    // [MINIMIZE BUTTON] =============================
    if (lpObjSender->ObjectID == DEWUI_OBJ_MIN_BUTTON)
    {
        ShowWindow(_hWnd, SW_MINIMIZE);
    }
    
    // [CLOSE BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_CLOSE_BUTTON)
    {
        PostMessage(_hWnd, WM_CLOSE, 0, 0);
    }
    
    // [SEEKBAR MODULE] ================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_SEEKBAR)
    {
        // The message must be 'seek' with accompanying code as the 'Next Logical Value'
        if (btMessage == DEWUI_MSG_SEEKED && btNotificationCode == DEWUI_NCODE_NEXT_LOGICAL_VALUE)
            this->_EA_HandleSeek(((DWORD)dwValue));
    }

    // [PLAY/PAUSE & 'DEW MODE' PLAY/PAUSE BUTTONS] =========================================================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_PLAY_PAUSE_BUTTON || lpObjSender->ObjectID == DEWUI_OBJ_DM_PLAY_PAUSE_BUTTON)
    {
        this->_EA_HandlePlayPause(TRUE); // These buttons can only be pressed manually
    }

    // [STOP BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_STOP_BUTTON)
    {
        this->_EA_HandleStop(); // This button can only be pressed manually
    }

    // [PREVIOUS BUTTON] ===================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_PREV_BUTTON)
    {
        this->_EA_HandlePrevious(TRUE); // This button can only be pressed manually
    }

    // [NEXT BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_NEXT_BUTTON)
    {
        this->_EA_HandleNext(TRUE); // This button can only be pressed manually
    }

    // [VOLUME BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_VOLUME_BUTTON)
    {
        // dwValue is the next (logival) value. But it has to be set after consulting with engine.
        this->_EA_HandleVolume((WORD)dwValue);
    }

    // [REPEAT BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_REPEAT_BUTTON)
    {
        // No action needed per se, but we'd like to store that into the setting
        _PlayerSettings.RepeatLevel = ((LPDEWREPEATBUTTON)lpObjSender)->State;
    }

    // [PLAYLIST BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_PLAYLIST_BUTTON)
    {
        this->_ShowFullView(!(this->_PlayerSettings.PlaylistVisible == 1));
    }

    // [BROWSE BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_BROWSE_BUTTON)
    {
        // Proceed only if there is no queuing, going on
        if (!_lpPlaylist->QueuingInProgress)
        {
            GetWindowRect(_lpCmdBrowse->Handle, &rctOpenBtn);
            TrackPopupMenuEx(_hMnuOpen, TPM_RIGHTBUTTON, rctOpenBtn.left, rctOpenBtn.bottom, _hWnd, NULL);
        }
    }
    
    // [SETTINGS BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_SETTINGS_BUTTON)
    {
        _PlayerSettings.FocusWindowHandle = _PlayerSettings.ShowSettings(_hWnd);
    }

    // [EXTRA INFO BUTTON] ===================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_EXINFO_BUTTON)
    {
        _PlayerSettings.FocusWindowHandle = _lpPropPage->Show(_PlayerSettings.Theme);
    }

    // [TIMER BUTTON] =======================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_TIMER_BUTTON)
    {
        _PlayerSettings.FocusWindowHandle = _lpShutdownTimer->Show(_PlayerSettings.Theme);
    }

    // ['DEW MODE' BUTTON] ====================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_DEWMODE_BUTTON)
    {
        this->_SetDewMode(!_PlayerSettings.DewMode);
    }

    // [PLAYLIST MODULE] ================================
    else if (lpObjSender->ObjectID == DEWUI_OBJ_PLAYLIST)
    {
        // -- An ongoing enqueuing process has completed -----------------------
        if (btMessage == DEWUI_MSG_PL_PARSE_COMPLETE)
        {
            if (_lpMapContent)
            {
                // If items are there and needs to play immediately
                // select the first item and start with it
                if (_lpMapContent->btCmdlineOpt == DEWUI_CMDLINE_OPT_PLAY &&
                    _lpPlaylist->GetFileCount() > 0)
                    _lpPlaylist->ActivateItem(0);

                // Signal the event that we are done here
                if (_lpMapContent->hEvtStartEnqueue)
                    SetEvent(_lpMapContent->hEvtStartEnqueue);

                // Clear the event flag (irrespective)
                _lpMapContent->hEvtStartEnqueue = NULL;
            }
            DragAcceptFiles(_hWnd, TRUE);
        }
        // -- User has opted to export the playlist item(s) --------------------
        else if (btMessage == DEWUI_MSG_PL_EXPORT)
        {
            _PlayerSettings.FocusWindowHandle = _lpPlaylistExporter->Show((DWORD)dwValue);
        }
        // -- User has activated a playlist item (double-click/enter key) ------
        else if (btMessage == DEWUI_MSG_PL_SELECTION)
        {
            // Reset any eng that might be playing etc...
            if (_lpAudioEngine != NULL)
                this->_EA_HandleStop();
            this->_EA_HandlePlayPause(TRUE); // Manually double-clicked from th eplaylist
        }
    }

    return;
    
}

#endif // DEW_THEMER_MORE

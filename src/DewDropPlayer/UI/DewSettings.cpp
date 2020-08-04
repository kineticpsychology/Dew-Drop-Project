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

#include "DewSettings.h"

// If any theme file is present in the same location as that of the exe
// then we can safely use a 'no-backslash' path, so that the exe+dth
// package can be moved around in different PCs, without absolute path dependencies
void DEWSETTINGS::_TrimPathForLocalPath(LPWSTR wsPath)
{
    wchar_t     wsAppPath[MAX_CHAR_PATH] { 0 };
    wchar_t     wsCompPath[MAX_CHAR_PATH] { 0 };
    size_t      nIndex;

    GetModuleFileName(NULL, wsAppPath, MAX_CHAR_PATH);
    wsAppPath[(lstrlen(wsAppPath) - lstrlen(StrRStrI(wsAppPath, NULL, L"\\")))] = L'\0';

    CopyMemory(wsCompPath, wsPath, lstrlen(wsPath) * sizeof(wchar_t));
    nIndex = (lstrlen(wsCompPath) - lstrlen(StrRStrI(wsCompPath, NULL, L"\\")));
    wsCompPath[nIndex] = L'\0';

    // Path is actually a local path (w.r.t. the exe path)
    if (nIndex > 0 && !lstrcmpi(wsAppPath, wsCompPath))
    {
        CopyMemory(wsCompPath, wsPath, lstrlen(wsPath) * sizeof(wchar_t));
        ZeroMemory(wsPath, MAX_CHAR_PATH * sizeof(wchar_t));
        CopyMemory(wsPath, &(wsCompPath[nIndex + 1]),
                   (lstrlen(wsCompPath) - (nIndex + 1)) * sizeof(wchar_t));
    }
    return;
}

void DEWSETTINGS::_ReadSettingsWithDefaults()
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;
    HANDLE      hSettingsFile = NULL;

    DWORD       dwPassLen = 0, dwSrcLen = 0, dwStructLen = 0;
    DWORD       dwFileByteLen = 0, dwOffset = 0, dwIndex = 0;
    WORD        wThemeVerMajor, wThemeVerMinor;
    LPBYTE      lpData = NULL;
    UINT        nIndex;
    BYTE        btApplyTheme;

    dwPassLen = lstrlenA(_lpDefPass);

    hSettingsFile = CreateFile(_wsSettingsFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSettingsFile && hSettingsFile != INVALID_HANDLE_VALUE)
    {
        dwSrcLen = GetFileSize(hSettingsFile, NULL);
        if (dwSrcLen > 0)
        {
            lpData = (LPBYTE)LocalAlloc(LPTR, dwSrcLen);
            ReadFile(hSettingsFile, lpData, dwSrcLen, &dwSrcLen, NULL);
        }
        CloseHandle(hSettingsFile);
    }

    if (dwSrcLen > 0)
    {
        bResult = CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
        if (bResult && hProvider)
        {
            bResult = CryptCreateHash(hProvider, CALG_MD5, 0, 0, &hHash);
            if(bResult && hHash)
            {
                bResult = CryptHashData(hHash, (LPBYTE)_lpDefPass, dwPassLen, 0);
                if (bResult)
                {
                    bResult = CryptDeriveKey(hProvider, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey);
                    if (bResult && hKey)
                    {
                        bResult = CryptDecrypt(hKey, 0, TRUE, 0, lpData, &dwSrcLen);
                        if(bResult)
                        {
                            dwStructLen = sizeof(SETTINGS);
                            CopyMemory(&_settings, lpData, dwStructLen);
                            _lpSongList = NULL;

                            if (_settings._bRememberHistory && _settings._dwHistoryFileCount)
                            {
                                dwOffset = dwStructLen;
                                _lpSongList = (LPFILELIST)LocalAlloc(LPTR,
                                               (_settings._dwHistoryFileCount) * sizeof(FILELIST));
                                for (dwIndex = 0; dwIndex < _settings._dwHistoryFileCount; dwIndex++)
                                {
                                    CopyMemory(&dwFileByteLen, &(lpData[dwOffset]), sizeof(DWORD));
                                    dwOffset += sizeof(DWORD);
                                    CopyMemory((_lpSongList[dwIndex].wsFileName), &(lpData[dwOffset]), dwFileByteLen);
                                    dwOffset += dwFileByteLen;
                                }
                            }
                            if (lstrcmpi(_settings._wsThemeFile, L"."))
                            {
                                btApplyTheme = _dewTheme.LoadThemeFile(_settings._wsThemeFile, &wThemeVerMajor, &wThemeVerMinor);
                                if (btApplyTheme != DEWTHEME_ACTION_THEME_APPLIED &&
                                    btApplyTheme != DEWTHEME_ACTION_DEFAULT &&
                                    btApplyTheme != DEWTHEME_ACTION_THEME_OVERRIDE)
                                        _dewTheme.Reset();
                            }
                        }
                        CryptDestroyKey(hKey);
                    }
                }
                CryptDestroyHash(hHash);
            }
            CryptReleaseContext(hProvider, 0);
        }
        LocalFree(lpData);
        lpData = NULL;
    }

    // Defaults (either settings file is not present or invalid settings file)
    if (!bResult)
    {
        _settings._btMinimizeAction = DEWOPT_MINIMIZE_MINIMIZE;
        _settings._btCloseAction = DEWOPT_CLOSE_QUIT;
        _settings._dwVolLevel = 0xFFFF;
        _settings._btRepeatLevel = DEWOPT_REPEAT_OFF;
        _settings._btPlaylistVisible = 0;
        _settings._wsThemeFile[0] = L'.';  _settings._wsThemeFile[1] = L'\0';
        _settings._bRememberHistory = 1; // Default: 1 (Always remember history)
        _settings._dwHistoryFileCount = 0; // No history for starters
        _lpSongList = NULL;
        _settings._bTopmostInDewMode = 1; // Default: 1 (Topmost)
        _settings._bDeepScan = 0; // Default: 0 (Use only extension to understand audio file type)
        _settings._bNotifyOnSongChange = 0; // Default: 0 (Do not notify user when a new song starts playing)
        // Set to default Positions (MAKELPARAM of (0,0)) & make all visible
        for (nIndex = 0; nIndex < DEWUI_OBJECT_COUNT - 1; nIndex++)
        {
            _settings._lpdwComponentPos[nIndex] = 0x00; // Default locations (set to 0)
            _settings._lpbComponentVisibility[nIndex] = TRUE; // Default: TRUE (All Visible)
        }
        _settings._bShowTitle = 1; // Default: 1 (Always Show Title)
        _settings._bDewMode = 0; // Default: 0 (Non-Dew mode)
    }
    // Layout viewer is always invisible
    _settings._lpdwComponentPos[DEWUI_OBJECT_COUNT - 1] = 0x00; // Default location
    _settings._lpbComponentVisibility[DEWUI_OBJECT_COUNT - 1] = FALSE; // Default: FALSE (Hide)
    return;
}

void DEWSETTINGS::_SaveSettings()
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;

    DWORD       dwPassLen = 0, dwEncLen = 0, dwStructLen = 0, dwSrcLen = 0;
    DWORD       dwIndex, dwFileByteLen, dwOffset;
    LPBYTE      lpSrcData = NULL, lpEncData = NULL;
    HANDLE      hSettingsFile = NULL;

    dwStructLen = sizeof(_settings);
    dwSrcLen = dwStructLen;
    if (_settings._bRememberHistory && _settings._dwHistoryFileCount > 0)
    {
        // Each entry will be preceded by its length. That way, we do not spend
        // additional 'MAX_CHAR_PATH - lstrlen(file_name)' blank spaces
        for (dwIndex = 0; dwIndex < _settings._dwHistoryFileCount; dwIndex++)
            dwSrcLen += ((lstrlen(_lpSongList[dwIndex].wsFileName) * sizeof(wchar_t)) + sizeof(DWORD));
    }
    dwPassLen = lstrlenA(_lpDefPass);
    lpSrcData = (LPBYTE)LocalAlloc(LPTR, dwSrcLen);
    CopyMemory(lpSrcData, &_settings, dwStructLen);
    dwOffset = dwStructLen;
    if (_settings._bRememberHistory && _settings._dwHistoryFileCount > 0)
    {
        for (dwIndex = 0; dwIndex < _settings._dwHistoryFileCount; dwIndex++)
        {
            dwFileByteLen = lstrlen(_lpSongList[dwIndex].wsFileName) * sizeof(wchar_t);
            CopyMemory(&(lpSrcData[dwOffset]), &dwFileByteLen, sizeof(DWORD));
            dwOffset += sizeof(DWORD);
            CopyMemory(&(lpSrcData[dwOffset]), (_lpSongList[dwIndex].wsFileName), dwFileByteLen);
            dwOffset += dwFileByteLen;
        }
    }

    bResult = CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    if (bResult && hProvider)
    {
        bResult = CryptCreateHash(hProvider, CALG_MD5, 0, 0, &hHash);
        if(bResult && hHash)
        {
            bResult = CryptHashData(hHash, (LPBYTE)_lpDefPass, dwPassLen, 0);
            if (bResult)
            {
                bResult = CryptDeriveKey(hProvider, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey);
                if (bResult && hKey)
                {
                    dwEncLen = dwSrcLen;
                    bResult = CryptEncrypt(hKey, 0, TRUE, 0, NULL, &dwEncLen, dwSrcLen);
                    if(bResult)
                    {
                        lpEncData = (LPBYTE)LocalAlloc(LPTR, dwEncLen);
                        CopyMemory(lpEncData, lpSrcData, dwSrcLen);
                        bResult = CryptEncrypt(hKey, 0, TRUE, 0, lpEncData, &dwSrcLen, dwEncLen);
                        if (bResult)
                        {
                            dwEncLen = dwSrcLen;
                            hSettingsFile = CreateFile(_wsSettingsFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hSettingsFile && hSettingsFile != INVALID_HANDLE_VALUE)
                            {
                                WriteFile(hSettingsFile, lpEncData, dwEncLen, &dwEncLen, NULL);
                                CloseHandle(hSettingsFile);
                            }
                        }
                        LocalFree(lpEncData);
                        lpEncData = NULL;
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProvider, 0);
    }
    LocalFree(lpSrcData);

    return;
}

DEWSETTINGS::DEWSETTINGS() : Theme (_dewTheme), MinimizeAction (_settings._btMinimizeAction),
CloseAction (_settings._btCloseAction), VolumeLevel (_settings._dwVolLevel),
RepeatLevel (_settings._btRepeatLevel), PlaylistVisible (_settings._btPlaylistVisible),
ThemeFile(_settings._wsThemeFile), RememberHistory(_settings._bRememberHistory),
HistoryFileCount(_settings._dwHistoryFileCount), SongList(_lpSongList),
TopmostInDewMode(_settings._bTopmostInDewMode), DeepScan(_settings._bDeepScan),
NotifyOnSongChange(_settings._bNotifyOnSongChange),
ComponentPos(_settings._lpdwComponentPos), ComponentVisibility(_settings._lpbComponentVisibility),
ShowTitle(_settings._bShowTitle), DewMode(_settings._bDewMode)
{
    size_t      nIndex = 0, nNameLength = 0, nPadding = 0;

    ZeroMemory(&_settings, sizeof(SETTINGS));
    ZeroMemory(_wsSettingsFile, MAX_CHAR_PATH * sizeof(wchar_t));

    // Don't try to read any settings file, if running in 'Themer' mode
#ifndef DEW_THEMER_MODE
    GetModuleFileName(NULL, _wsSettingsFile, MAX_CHAR_PATH);
    nNameLength = lstrlen(_wsSettingsFileName) * sizeof(wchar_t);
    for (nIndex = lstrlen(_wsSettingsFile) - 2; nIndex; nIndex--)
    {
        if (_wsSettingsFile[nIndex + 1] == L'\\')
        {
            nIndex += 2;
            break;
        }
    }
    nPadding = (lstrlen(_wsSettingsFile) - nIndex) * sizeof(wchar_t);
    ZeroMemory(&(_wsSettingsFile[nIndex]), nPadding);
    CopyMemory(&(_wsSettingsFile[nIndex]), _wsSettingsFileName, nNameLength);
#endif

    this->_ReadSettingsWithDefaults();
    _hInstance = GetModuleHandle(NULL);
    _iDPI = _dewTheme.DPI;
    _fScale = _dewTheme.Scale;
    _iMinWidth = _F(480);
    _iMinHeight = _F(520);
    return;
}

// Auto trigger saving the settings, without the headache
// for the application to call an explicit function
DEWSETTINGS::~DEWSETTINGS()
{
#ifndef DEW_THEMER_MODE
    // Do not save settings in 'Theme' mode
    this->_SaveSettings();
#endif
    if (_lpSongList) { LocalFree(_lpSongList); _lpSongList = NULL; }
    _bThemeNeedsRefresh = FALSE;
    this->_DeleteUIComponents();
    return;
}

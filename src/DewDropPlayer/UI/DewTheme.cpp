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

#include "DewTheme.h"

void DEWTHEME::_SetDefaultTheme(BOOL bCalcDPI)
{
    const wchar_t   *wsDefaultFontName = L"Tahoma";

    const COLORREF  crBack = RGB(0x30, 0x30, 0x30);
    const COLORREF  crWinText = RGB(0xFF, 0xFF, 0xFF);
    const COLORREF  crWinOutline = RGB(0x00, 0x00, 0x00);
    const COLORREF  crText = RGB(0x00, 0xFF, 0xFF);

    const int       iDefaultTitleFontSize = 10;
    const int       iDefaultTextFontSize = 8;
    const BOOL      bDefaultItalic = FALSE;
    const BOOL      bDefaultTitleBold = TRUE;
    const BOOL      bDefaultTextBold = FALSE;

    ZeroMemory((_theme._wsName), DEWTHEME_ATTR_LENGTH  * sizeof(wchar_t));
    ZeroMemory((_theme._wsAuthor), DEWTHEME_ATTR_LENGTH  * sizeof(wchar_t));
    ZeroMemory((_theme._titleFontStyle.FontName), 32  * sizeof(wchar_t));
    ZeroMemory((_theme._textFontStyle.FontName), 32  * sizeof(wchar_t));

    if (bCalcDPI) this->_SetDPIInfo();
    CopyMemory((this->_theme._wsName), DEWTHEME_DEFAULT_NAME, sizeof(wchar_t) * lstrlen(DEWTHEME_DEFAULT_NAME));
    CopyMemory((this->_theme._wsAuthor), DEWTHEME_DEFAULT_AUTHOR, sizeof(wchar_t) * lstrlen(DEWTHEME_DEFAULT_AUTHOR));

    this->_theme._btIconMode = DEWTHEME_ICON_MODE_DEFAULT; // 'DEFAULT' icon mode
    this->_theme._btIconScale = DEWTHEME_ICON_SCALE_MEDIUM; // 'MEDIUM' icon scale
    this->_theme._btTransparentIcons = 0; // Opaque icons
    this->_theme._btShowSeekbarTimes = 1; // Times will always be displayed on seekbar

    this->_theme._winStyle.BackColor = crBack;
    this->_theme._winStyle.BackColor2 = crBack;
    this->_theme._winStyle.OutlineColor = crWinOutline;
    this->_theme._winStyle.TextColor = crWinText;

    this->_theme._mmButtonStyle.BackColor = crBack;
    this->_theme._mmButtonStyle.OutlineColor = crText;
    this->_theme._mmButtonStyle.TextColor = crText;

    this->_theme._uiButtonStyle.BackColor = crBack;
    this->_theme._uiButtonStyle.OutlineColor = crWinText;
    this->_theme._uiButtonStyle.TextColor = crWinText;

    this->_theme._moduleStyle.BackColor = crBack;
    this->_theme._moduleStyle.OutlineColor = crText;
    this->_theme._moduleStyle.TextColor = crText;

    CopyMemory((this->_theme._titleFontStyle.FontName), wsDefaultFontName, 32 * sizeof(wchar_t));
    this->_theme._titleFontStyle.FontSize = iDefaultTitleFontSize;
    this->_theme._titleFontStyle.IsBold = bDefaultTitleBold;
    this->_theme._titleFontStyle.IsItalic = bDefaultItalic;

    CopyMemory((this->_theme._textFontStyle.FontName), wsDefaultFontName, 32 * sizeof(wchar_t));
    this->_theme._textFontStyle.FontSize = iDefaultTextFontSize;
    this->_theme._textFontStyle.IsBold = bDefaultTextBold;
    this->_theme._textFontStyle.IsItalic = bDefaultItalic;

    // No  background image, by default
    this->_theme._dwBackImgSize = 0x00; // No image
    this->_lpThemeBackImgData = NULL;

    // Dew Icon by default
    this->_theme._dwIconImgSize = 0x00;
    this->_lpIconImgData = NULL;

    // Default password is blank
    ZeroMemory(_sCurrPass, DEWTHEME_ATTR_LENGTH * sizeof(char));

    // Theme version is the EXE's current major.minor version
    DEWTHEME::GetDefaultThemeVersion(&_wThemeVerMajor, &_wThemeVerMinor);

    _bDefaultTheme = TRUE;
    return;
}

void DEWTHEME::_SetDPIInfo()
{
    LOGFONT         lgfFont { 0 };
    HDC             hDCDesk = NULL;
    int             iFontSize = 10;

    hDCDesk = GetDC(HWND_DESKTOP);
    this->_iDPI = GetDeviceCaps(hDCDesk, LOGPIXELSY);
    ReleaseDC(HWND_DESKTOP, hDCDesk);
    hDCDesk = NULL;
    this->_fScale = (float)(this->_iDPI)/96.0f;
    iFontSize = (int)(((float)iFontSize) * this->_fScale);

    lgfFont.lfHeight = -MulDiv(iFontSize, this->_iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    this->_hFntUI = CreateFontIndirect(&lgfFont);
    return;
}

BOOL DEWTHEME::_CreateHeader(const char* sPassword, LPBYTE *lplpData, LPDWORD lpdwEncLen)
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;
    DWORD       dwPassLen = 0, dwSrcLen = 0, dwEncLen = 0, dwHeader;

    dwSrcLen = lstrlenA(sPassword);
    dwPassLen = lstrlenA(_lpDefPass);

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
                        *lplpData = (LPBYTE)LocalAlloc(LPTR, dwEncLen + _HEADER_BLOCK_SIZE);
                        CopyMemory(&((*lplpData)[_HEADER_BLOCK_SIZE]), sPassword, dwSrcLen);
                        bResult = CryptEncrypt(hKey, 0, TRUE, 0, &((*lplpData)[_HEADER_BLOCK_SIZE]), &dwSrcLen, dwEncLen);
                        if (bResult)
                        {
                            dwHeader = DEWTHEME_HEADER;
                            *lpdwEncLen = dwSrcLen + _HEADER_BLOCK_SIZE;
                            CopyMemory(*lplpData, &dwHeader, 0x04);
                            CopyMemory(&((*lplpData)[0x04]), &_wThemeVerMajor, 0x02);
                            CopyMemory(&((*lplpData)[0x06]), &_wThemeVerMinor, 0x02);
                            (*lplpData)[0x08] = (BYTE)dwSrcLen;
                        }
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProvider, 0);
    }
    return bResult;
}

BOOL DEWTHEME::_EncryptTheme(const char* sPassword, const LPBYTE lpSrcData, const DWORD dwSrcDataLen, LPBYTE *lplpEncData, LPDWORD lpdwEncLen)
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;

    LPBYTE      lpHeaderData = NULL;
    DWORD       dwPassLen = 0, dwSrcLen = 0, dwEncLen = 0, dwHeaderSize = 0;

    dwPassLen = lstrlenA(sPassword);
    dwSrcLen = dwSrcDataLen;

    bResult = CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    if (bResult && hProvider)
    {
        bResult = CryptCreateHash(hProvider, CALG_MD5, 0, 0, &hHash);
        if(bResult && hHash)
        {
            bResult = CryptHashData(hHash, (LPBYTE)sPassword, dwPassLen, 0);
            if (bResult)
            {
                bResult = CryptDeriveKey(hProvider, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey);
                if (bResult && hKey)
                {
                    dwEncLen = dwSrcLen;
                    bResult = CryptEncrypt(hKey, 0, TRUE, 0, NULL, &dwEncLen, dwSrcLen);
                    if(bResult)
                    {
                        if (this->_CreateHeader(sPassword, &lpHeaderData, &dwHeaderSize))
                        {
                            *lplpEncData = (LPBYTE)LocalAlloc(LPTR, dwEncLen + dwHeaderSize);
                            CopyMemory(&((*lplpEncData)[dwHeaderSize]), lpSrcData, dwSrcLen);
                            bResult = CryptEncrypt(hKey, 0, TRUE, 0, &((*lplpEncData)[dwHeaderSize]), &dwSrcLen, dwEncLen);
                            if (bResult)
                            {
                                CopyMemory(*lplpEncData, lpHeaderData, dwHeaderSize);
                                *lpdwEncLen = dwSrcLen + dwHeaderSize;
                            }
                            else
                            {
                                LocalFree(*lplpEncData);
                                *lplpEncData = NULL;
                                *lpdwEncLen = 0;
                            }
                            LocalFree(lpHeaderData);
                            lpHeaderData = NULL;
                        }
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProvider, 0);
    }

    LocalFree(lpSrcData);
    return bResult;
}

BOOL DEWTHEME::_GetThemeUnlockKey(LPBYTE lpMainData, LPBYTE *lplpKey, LPDWORD lpdwKey)
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;

    DWORD       dwPassLen = 0, dwDecLen;

    dwPassLen = lstrlenA(_lpDefPass);
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
                    dwDecLen = lpMainData[_HEADER_BLOCK_SIZE - 1];
                    *lplpKey = (LPBYTE)LocalAlloc(LPTR, dwDecLen);
                    CopyMemory(*lplpKey, &(lpMainData[_HEADER_BLOCK_SIZE]), dwDecLen);
                    bResult = CryptDecrypt(hKey, 0, TRUE, 0, *lplpKey, &dwDecLen);
                    if(bResult)
                    {
                        *lpdwKey = dwDecLen;
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProvider, 0);
    }

    return bResult;
}

BOOL DEWTHEME::_DecryptTheme(const LPBYTE lpEncData, const DWORD dwEncLen, LPBYTE *lplpDecData, LPDWORD lpdwDecLen)
{
    BOOL        bResult = FALSE;
    HCRYPTPROV  hProvider = NULL;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;

    LPBYTE      lpKey = NULL;
    DWORD       dwPassLen = 0, dwSrcLen, dwOffset;

    if (!this->_GetThemeUnlockKey(lpEncData, &lpKey, &dwPassLen)) return FALSE;
    ZeroMemory(_sCurrPass, DEWTHEME_ATTR_LENGTH * sizeof(char));
    CopyMemory(_sCurrPass, lpKey, dwPassLen); // Store the password
    dwOffset = _HEADER_BLOCK_SIZE + lpEncData[_HEADER_BLOCK_SIZE - 1];
    dwSrcLen = dwEncLen - dwOffset;

    bResult = CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    if (bResult && hProvider)
    {
        bResult = CryptCreateHash(hProvider, CALG_MD5, 0, 0, &hHash);
        if(bResult && hHash)
        {
            bResult = CryptHashData(hHash, (LPBYTE)lpKey, dwPassLen, 0);
            if (bResult)
            {
                bResult = CryptDeriveKey(hProvider, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey);
                if (bResult && hKey)
                {
                    *lplpDecData = (LPBYTE)LocalAlloc(LPTR, dwSrcLen);
                    CopyMemory(*lplpDecData, &(lpEncData[dwOffset]), dwSrcLen);
                    bResult = CryptDecrypt(hKey, 0, TRUE, 0, *lplpDecData, &dwSrcLen);
                    if(bResult)
                    {
                        *lpdwDecLen = dwSrcLen;
                    }
                    else
                    {
                        LocalFree(*lplpDecData);
                        *lplpDecData = NULL;
                        *lpdwDecLen = 0;
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProvider, 0);
    }

    LocalFree(lpKey);
    return bResult;
}

void DEWTHEME::GetDefaultThemeVersion(LPWORD lpwThemeVerMajor, LPWORD lpwThemeVerMinor)
{
    wchar_t             wsProcessImgFile[MAX_CHAR_PATH] { 0 };
    DWORD               dwVISize = 0, dwStub = 0;
    UINT                nFFInfoSize = 0;
    LPBYTE              lpVI;
    VS_FIXEDFILEINFO    *pffInfo = NULL;

    GetModuleFileName(NULL, wsProcessImgFile, MAX_CHAR_PATH);
    dwVISize = GetFileVersionInfoSize(wsProcessImgFile, &dwStub);
    if (dwVISize > 0)
    {
        lpVI = (LPBYTE)LocalAlloc(LPTR, dwVISize);
        if (GetFileVersionInfo(wsProcessImgFile, 0, dwVISize, lpVI))
        {
            if (VerQueryValue(lpVI, L"\\", (LPVOID*)&pffInfo, &nFFInfoSize))
            {
                *lpwThemeVerMajor = HIWORD((pffInfo->dwFileVersionMS));
                *lpwThemeVerMinor = LOWORD((pffInfo->dwFileVersionMS));
            }
        }
        LocalFree(lpVI);
    }
    return;
}

#ifdef DEW_THEMER_MODE
// Theme cannot be exported if not in 'Theme' mode
BYTE DEWTHEME::WriteThemeFile(LPCWSTR wsThemeFile)
{
    HANDLE      hFile = NULL;
    DWORD       dwSrcLen = 0, dwEncLen = 0, dwGen = 0, dwStructLen;
    LPBYTE      lpSrcData = NULL, lpEncData = NULL;
    BOOL        bBackImg = FALSE, bIcoImg = FALSE;

    if (!_sCurrPass || lstrlenA(_sCurrPass) <= 0) return DEWTHEME_SAVE_NO_PASSWORD;

    hFile = CreateFile(wsThemeFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (!hFile || hFile == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
            return DEWTHEME_SAVE_NO_ACCESS;
        return DEWTHEME_SAVE_GENERIC_ERROR;
    }
    dwStructLen = sizeof(this->_theme);
    // If image data is there, it will follow after the struct
    bBackImg = this->_theme._dwBackImgSize > 0;
    bIcoImg = this->_theme._dwIconImgSize > 0;
    dwSrcLen = dwStructLen + // Core struct
               (bBackImg ? this->_theme._dwBackImgSize : 0) + // Back image size
               (bIcoImg ? this->_theme._dwIconImgSize : 0); // Icon image size
    lpSrcData = (LPBYTE)LocalAlloc(LPTR, dwSrcLen);
    CopyMemory(lpSrcData, &(this->_theme), dwStructLen); // Copy the struct
    // Now copy the remaining image data, if any
    if (bBackImg)
        CopyMemory(&(lpSrcData[dwStructLen]), (this->_lpThemeBackImgData), this->_theme._dwBackImgSize);
    // And copy the icon image data, if any
    if (bIcoImg)
        CopyMemory(&(lpSrcData[dwStructLen + this->_theme._dwBackImgSize]),
                   (this->_lpIconImgData), this->_theme._dwIconImgSize);

    if(this->_EncryptTheme(_sCurrPass, lpSrcData, dwSrcLen, &lpEncData, &dwEncLen))
    {
        WriteFile(hFile, lpEncData, dwEncLen, &dwGen, NULL);
        LocalFree(lpEncData);
    }
    CloseHandle(hFile);
    hFile = NULL;
    if (dwGen != dwEncLen)
        return DEWTHEME_SAVE_GENERIC_ERROR;
    return DEWTHEME_SAVE_OK;
}

void DEWTHEME::SetPassword(const char* sPassword)
{
    size_t  nSize;

    if (!sPassword || lstrlenA(sPassword) <= 0) return;

    nSize = (lstrlenA(sPassword) > DEWTHEME_ATTR_LENGTH ? DEWTHEME_ATTR_LENGTH : lstrlenA(sPassword));
    ZeroMemory(_sCurrPass, DEWTHEME_ATTR_LENGTH * sizeof(char));
    CopyMemory(_sCurrPass, sPassword, nSize * sizeof(char));
    return;
}

#endif

BYTE DEWTHEME::LoadThemeFile(LPCWSTR wsThemeFile, LPWORD lpwThemeVerMajor, LPWORD lpwThemeVerMinor, BOOL bTestOnly, LPWSTR wsName, LPWSTR wsAuthor, LPWSTR wsVersion)
{
    HANDLE      hFile = NULL;
    DWORD       dwEncLen = 0, dwThemeLen = 0, dwSig = 0, dwStructLen = 0, dwOffset = 0;
    LPBYTE      lpEncData = NULL, lpThemeData = NULL;
    THEME       testTheme { 0 };
    wchar_t     wsAbsPathFile[MAX_CHAR_PATH] { 0 };
    size_t      nIndex = 0;
    WORD        wThisThemeVerMajor, wThisThemeVerMinor;

    // Sanity Checks
    if (!wsThemeFile || !lpwThemeVerMajor || !lpwThemeVerMinor)
        return DEWTHEME_ACTION_THEME_FAIL;

    // Check if the file is simply local file. In that case,
    // prefix the local exe's path to the theme file path
    if (!StrRStrI(wsThemeFile, NULL, L"\\"))
    {
        GetModuleFileName(NULL, wsAbsPathFile, MAX_CHAR_PATH);
        nIndex = lstrlen(wsAbsPathFile) - lstrlen(StrRStrI(wsAbsPathFile, NULL, L"\\")) + 1;
        ZeroMemory(&(wsAbsPathFile[nIndex]), (lstrlen(wsAbsPathFile) - nIndex) * sizeof(wchar_t));
        CopyMemory(&(wsAbsPathFile[nIndex]), wsThemeFile, lstrlen(wsThemeFile) * sizeof(wchar_t));
    }
    else
    {
        CopyMemory(wsAbsPathFile, wsThemeFile, lstrlen(wsThemeFile) * sizeof(wchar_t));
    }

    hFile = CreateFile(wsAbsPathFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (!hFile || hFile == INVALID_HANDLE_VALUE) return DEWTHEME_ACTION_THEME_FAIL;
    dwEncLen = GetFileSize(hFile, NULL);

    // Check if the size id beyond the header size
    if (dwEncLen <= _HEADER_BLOCK_SIZE) { CloseHandle(hFile); return DEWTHEME_ACTION_THEME_FAIL; }

    // Read the full encrypted contents
    lpEncData = (LPBYTE)LocalAlloc(LPTR, dwEncLen);
    ReadFile(hFile, lpEncData, dwEncLen, &dwEncLen, NULL);
    CloseHandle(hFile);

    // Check file signature
    CopyMemory(&dwSig, lpEncData, 4);
    if (dwSig != DEWTHEME_HEADER)
    {
        LocalFree(lpEncData);
        return DEWTHEME_ACTION_THEME_FAIL;
    }

    CopyMemory(lpwThemeVerMajor, &(lpEncData[0x04]), 2);
    CopyMemory(lpwThemeVerMinor, &(lpEncData[0x06]), 2);
    DEWTHEME::GetDefaultThemeVersion(&wThisThemeVerMajor, &wThisThemeVerMinor);

    // Verify the version compatibility
    if (*lpwThemeVerMajor != wThisThemeVerMajor ||
        *lpwThemeVerMinor != wThisThemeVerMinor)
    {
        LocalFree(lpEncData);
        return DEWTHEME_ACTION_THEME_VER_MISMATCH;
    }

    if (!this->_DecryptTheme(lpEncData, dwEncLen, &lpThemeData, &dwThemeLen))
    {
        LocalFree(lpEncData);
        return DEWTHEME_ACTION_THEME_FAIL;
    }

    if (bTestOnly)
    {
        if (!wsName || !wsAuthor) return DEWTHEME_ACTION_THEME_FAIL;
        ZeroMemory((testTheme._wsName), DEWTHEME_ATTR_LENGTH  * sizeof(wchar_t));
        ZeroMemory((testTheme._wsAuthor), DEWTHEME_ATTR_LENGTH  * sizeof(wchar_t));
        CopyMemory(&testTheme, lpThemeData, sizeof(THEME));
        LocalFree(lpEncData);
        LocalFree(lpThemeData);
        CopyMemory(wsName, testTheme._wsName, DEWTHEME_ATTR_LENGTH * sizeof(wchar_t));
        CopyMemory(wsAuthor, testTheme._wsAuthor, DEWTHEME_ATTR_LENGTH * sizeof(wchar_t));
        return DEWTHEME_ACTION_THEME_APPLIED;
    }

    dwStructLen = sizeof(this->_theme);
    // First copy the struct
    CopyMemory(&(this->_theme), lpThemeData, dwStructLen);
    dwOffset = dwStructLen;

    // Now check if theme image data is there
    if (this->_theme._dwBackImgSize > 0x00)
    {
        if (this->_lpThemeBackImgData) { LocalFree(this->_lpThemeBackImgData); this->_lpThemeBackImgData = NULL; }
        this->_lpThemeBackImgData = (LPBYTE)LocalAlloc(LPTR, this->_theme._dwBackImgSize);
        CopyMemory((this->_lpThemeBackImgData), &(lpThemeData[dwOffset]), this->_theme._dwBackImgSize);
        dwOffset += this->_theme._dwBackImgSize;
    }

    // With the adjusted offset, now check if icon data is there
    if (this->_theme._dwIconImgSize > 0x00)
    {
        if (this->_lpIconImgData) { LocalFree(this->_lpIconImgData); this->_lpIconImgData = NULL; }
        this->_lpIconImgData = (LPBYTE)LocalAlloc(LPTR, this->_theme._dwIconImgSize);
        CopyMemory((this->_lpIconImgData), &(lpThemeData[dwOffset]), this->_theme._dwIconImgSize);
    }

    LocalFree(lpEncData);
    LocalFree(lpThemeData);
    _bDefaultTheme = FALSE;
    return DEWTHEME_ACTION_THEME_APPLIED;
}

DEWTHEME::DEWTHEME() : Name(_theme._wsName), Author(_theme._wsAuthor),
WinStyle (_theme._winStyle), MMButtonStyle (_theme._mmButtonStyle),
UIButtonStyle (_theme._uiButtonStyle), ModuleStyle (_theme._moduleStyle),
TitleFontStyle(_theme._titleFontStyle), TextFontStyle(_theme._textFontStyle),
IconMode(_theme._btIconMode), IconScale(_theme._btIconScale),
TransparentIcons(_theme._btTransparentIcons), ShowSeekbarTimes(_theme._btShowSeekbarTimes),
BackgroundImageSize(_theme._dwBackImgSize), BackgroundImageData(_lpThemeBackImgData),
IconImageSize (_theme._dwIconImgSize), IconImageData (_lpIconImgData),
DPI (_iDPI), Scale (_fScale), DefaultUIFont(_hFntUI),
VersionMajor(_wThemeVerMajor), VersionMinor(_wThemeVerMinor), DefaultTheme(_bDefaultTheme)
#ifdef DEW_THEMER_MODE
, Password(_sCurrPass)
#endif
{
    this->_SetDefaultTheme();
    return;
}

DEWTHEME::DEWTHEME(LPCWSTR wsThemeFile) : Name(_theme._wsName), Author(_theme._wsAuthor),
WinStyle (_theme._winStyle), MMButtonStyle (_theme._mmButtonStyle),
UIButtonStyle (_theme._uiButtonStyle), ModuleStyle (_theme._moduleStyle),
TitleFontStyle(_theme._titleFontStyle), TextFontStyle(_theme._textFontStyle),
IconMode(_theme._btIconMode), IconScale(_theme._btIconScale),
TransparentIcons(_theme._btTransparentIcons), ShowSeekbarTimes(_theme._btShowSeekbarTimes),
BackgroundImageSize(_theme._dwBackImgSize), BackgroundImageData(_lpThemeBackImgData),
IconImageSize (_theme._dwIconImgSize), IconImageData (_lpIconImgData),
DPI (_iDPI), Scale (_fScale), DefaultUIFont(_hFntUI),
VersionMajor(_wThemeVerMajor), VersionMinor(_wThemeVerMinor), DefaultTheme(_bDefaultTheme)
#ifdef DEW_THEMER_MODE
, Password(_sCurrPass)
#endif
{
    if (this->LoadThemeFile(wsThemeFile, &_wThemeVerMajor, &_wThemeVerMinor) != DEWTHEME_ACTION_THEME_APPLIED)
        this->_SetDefaultTheme();
    return;
}

void DEWTHEME::Reset()
{
    this->_SetDefaultTheme(FALSE);
}

#ifdef DEW_THEMER_MODE
// Theme copying is prohibited if not in 'Theme' mode
void DEWTHEME::operator= (const DEWTHEME& Theme)
{
    StringCchPrintf(this->_theme._wsName, DEWTHEME_ATTR_LENGTH, Theme._theme._wsName);
    StringCchPrintf(this->_theme._wsAuthor, DEWTHEME_ATTR_LENGTH, Theme._theme._wsAuthor);

    this->_theme._btIconMode = Theme._theme._btIconMode;
    this->_theme._btIconScale = Theme._theme._btIconScale;
    this->_theme._btTransparentIcons = Theme._theme._btTransparentIcons;
    this->_theme._btShowSeekbarTimes = Theme._theme._btShowSeekbarTimes;

    CopyMemory(&(this->_theme._winStyle), &(Theme._theme._winStyle), sizeof(WINSTYLE));
    CopyMemory(&(this->_theme._mmButtonStyle), &(Theme._theme._mmButtonStyle), sizeof(BUTTONSTYLE));
    CopyMemory(&(this->_theme._uiButtonStyle), &(Theme._theme._uiButtonStyle), sizeof(BUTTONSTYLE));
    CopyMemory(&(this->_theme._moduleStyle), &(Theme._theme._moduleStyle), sizeof(MODULESTYLE));
    CopyMemory(&(this->_theme._titleFontStyle), &(Theme._theme._titleFontStyle), sizeof(FONTSTYLE));
    CopyMemory(&(this->_theme._textFontStyle), &(Theme._theme._textFontStyle), sizeof(FONTSTYLE));

    this->_theme._dwBackImgSize = Theme._theme._dwBackImgSize;
    this->_theme._dwIconImgSize = Theme._theme._dwIconImgSize;
    LocalFree(this->_lpThemeBackImgData); this->_lpThemeBackImgData = NULL;
    LocalFree(this->_lpIconImgData); this->_lpIconImgData = NULL;

    if (this->_theme._dwBackImgSize > 0)
    {
        this->_lpThemeBackImgData = (LPBYTE)LocalAlloc(LPTR, this->_theme._dwBackImgSize);
        CopyMemory((this->_lpThemeBackImgData), (Theme._lpThemeBackImgData), this->_theme._dwBackImgSize);
    }
    if (this->_theme._dwIconImgSize > 0)
    {
        this->_lpIconImgData = (LPBYTE)LocalAlloc(LPTR, this->_theme._dwIconImgSize);
        CopyMemory((this->_lpIconImgData), (Theme._lpIconImgData), this->_theme._dwIconImgSize);
    }

    CopyMemory(this->_sCurrPass, Theme._sCurrPass, DEWTHEME_ATTR_LENGTH * sizeof(char));
    this->_wThemeVerMajor = Theme._wThemeVerMajor;
    this->_wThemeVerMinor = Theme._wThemeVerMinor;
    this->_bDefaultTheme = Theme._bDefaultTheme;

    return;
}
#endif

// Free up any background image data that was used
DEWTHEME::~DEWTHEME()
{
    if (this->_theme._dwBackImgSize > 0 && this->_lpThemeBackImgData != NULL)
        LocalFree(this->_lpThemeBackImgData);
    this->_lpThemeBackImgData = NULL;
    if (this->_theme._dwIconImgSize > 0 && this->_lpIconImgData != NULL)
        LocalFree(this->_lpIconImgData);
    this->_lpIconImgData = NULL;

    return;
}

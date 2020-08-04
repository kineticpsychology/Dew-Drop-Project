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

#ifndef _DEWDROPTHEMERCOMMON_H_
#define _DEWDROPTHEMERCOMMON_H_

#ifndef DEW_THEMER_MODE
#define DEW_THEMER_MODE
#endif

#include "../DewDropPlayer/UI/DewWindow.h"
#include "DewDropThemerResource.h"

#define IDM_FILE_NEW            0x11
#define IDM_FILE_OPEN           0x12
#define IDM_FILE_SAVE           0x13
#define IDM_FILE_SAVEAS         0x14
#define IDM_FILE_CLOSE          0x15
#define IDM_FILE_EXIT           0x16

#define IDM_THEME_RESET         0x21
#define IDM_THEME_DEFAULT       0x22

#define IDM_HELP_ABOUT          0x31


#define THEMER_PW_MODE_CHECK    0x01 // One field. Needed when opening Theme file
#define THEMER_PW_MODE_SET      0x02 // Two fields. Needed when saving Theme file
#define THEMER_PW_MODE_CHANGE   0x03 // Thee fields. Needed when changing password

#define THEMER_RET_OK                       0x00

#define THEMER_CHECK_OK                     THEMER_RET_OK
#define THEMER_CHECK_LOAD_FAIL              0x01
#define THEMER_CHECK_NO_PASSWORD            0x02
#define THEMER_CHECK_INCORRECT_PASSWORD     0x03

#define THEMER_SET_OK                       THEMER_RET_OK
#define THEMER_SET_NO_PASSWORD              THEMER_CHECK_NO_PASSWORD
#define THEMER_SET_INCORRECT_PASSWORD       THEMER_CHECK_INCORRECT_PASSWORD

#define THEMER_CHANGE_OK                    THEMER_RET_OK
#define THEMER_CHANGE_NO_PASSWORD           THEMER_CHECK_NO_PASSWORD
#define THEMER_CHANGE_INCORRECT_PASSWORD    THEMER_CHECK_INCORRECT_PASSWORD
#define THEMER_CHANGE_SAME_PASSWORD         0x03

typedef struct _BROWSE_THEME_ACTION
{
    wchar_t     ThemeFile[MAX_CHAR_PATH];
    BYTE        ActionMode;
    BYTE        ReturnCode;
} BROWSE_THEME_ACTION, *LPBROWSE_THEME_ACTION;

#endif // _DEWDROPTHEMERCOMMON_H_

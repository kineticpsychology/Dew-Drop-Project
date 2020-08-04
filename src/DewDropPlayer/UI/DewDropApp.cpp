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

#include "DewDropApp.h"

ULONG_PTR DEWDROPAPP::_sgdiToken = 0x00;
GdiplusStartupInput DEWDROPAPP::_sgsi;
UINT DEWDROPAPP::_snInstance = 0x00;
BOOL DEWDROPAPP::_sbRunExecuted = FALSE;

int DEWDROPAPP::_siRun()
{
    DEWSETTINGS     dsDefault;
    DEWSTARTUP      Startup;
    LPDEWWINDOW     lpDewWindow;
    MSG             msg { 0 };

    if (Startup.Status == DEWSTARTUP_STATUS_ERROR)
        return (int)0xD1E;
    if (Startup.Status == DEWSTARTUP_STATUS_CLOSE)
        return 0;

    if (Startup.Status == DEWSTARTUP_STATUS_IMMEDIATE)
    {
        lpDewWindow = new DEWWINDOW(dsDefault, TRUE);
    }
    else if (Startup.Status == DEWSTARTUP_STATUS_CONTINUE)
    {
        lpDewWindow = new DEWWINDOW(dsDefault);
    }
    else
    {
        // Unknown status
        return (int)0xD1E;
    }

    lpDewWindow->SetVisible();


    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (dsDefault.FocusWindowHandle == lpDewWindow->Handle)
        {
            // The accelerators are applicable only for the main UI
            // Other windows should not be impacted by this
            if(!TranslateAccelerator(dsDefault.FocusWindowHandle, lpDewWindow->AccelHotKey, &msg) &&
               !IsDialogMessage(dsDefault.FocusWindowHandle, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(!IsDialogMessage(dsDefault.FocusWindowHandle, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    delete lpDewWindow;

    return (int)msg.wParam;
}

DEWDROPAPP::DEWDROPAPP()
{
    if (DEWDROPAPP::_snInstance == 0x00)
    {
        CoInitialize(NULL); // Needed for Shell APIs
        GdiplusStartup(&(DEWDROPAPP::_sgdiToken), &(DEWDROPAPP::_sgsi), NULL); // Needed BEFORE any GDI+ operation
    }
    (DEWDROPAPP::_snInstance)++;
}

int DEWDROPAPP::Run()
{
    if (DEWDROPAPP::_sbRunExecuted) return 0xDEAD;

    DEWDROPAPP::_sbRunExecuted = TRUE;
    return DEWDROPAPP::_siRun();
}

DEWDROPAPP::~DEWDROPAPP()
{
    (DEWDROPAPP::_snInstance)--;
    if (DEWDROPAPP::_snInstance == 0x00)
    {
        GdiplusShutdown(DEWDROPAPP::_sgdiToken); // Shutdown GDI+ AFTER all traces of GDI+ have been destroyed
        CoUninitialize();
    }
}

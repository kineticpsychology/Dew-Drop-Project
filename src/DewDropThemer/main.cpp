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

#include "ThemerWindow.h"

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR wsCmd, int nCmd)
#endif
{
    int                 iRet;
    ULONG_PTR           gdiToken;
    GdiplusStartupInput gsi;
    LPTHEMERWINDOW      lpThemerWindow;

    GdiplusStartup(&gdiToken, &gsi, NULL);

    lpThemerWindow = new THEMERWINDOW();
    iRet = lpThemerWindow->Run();
    delete lpThemerWindow;

    GdiplusShutdown(gdiToken);
    return iRet;
}

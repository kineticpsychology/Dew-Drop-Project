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

#ifndef _DEWREPEATBUTTON_H_
#define _DEWREPEATBUTTON_H_

#include "DewButton.h"

typedef class DEWREPEATBUTTON : public DEWBUTTON
{
    protected:
        Bitmap              *_pImgRepeatOffRaised = NULL;
        Bitmap              *_pImgRepeatOffSunken = NULL;
        Bitmap              *_pImgRepeatAllRaised = NULL;
        Bitmap              *_pImgRepeatAllSunken = NULL;
        Bitmap              *_pImgRepeatOneRaised = NULL;
        Bitmap              *_pImgRepeatOneSunken = NULL;
        BYTE                _btCurrState;
        BYTE                _btNextState;

        virtual void        _UpdateImgPointers();
        virtual void        _ClickedEvent();
        virtual void        _PrepareImages();
        virtual void        _MoveToDefaultLocation();
        virtual void        _Cleanup();

    public:
        const BYTE&         State;
        const BYTE&         NextState;
                            DEWREPEATBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BYTE btInitState = DEWOPT_REPEAT_OFF);
        void                SetNextState();
        virtual             ~DEWREPEATBUTTON();
} DEWREPEATBUTTON, *LPDEWREPEATBUTTON;
#endif // _DEWREPEATBUTTON_H_


#include "./UI/DewDropApp.h"

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR wsCmd, int nCmd)
#endif
{
    DEWDROPAPP      App;

    return App.Run();
}

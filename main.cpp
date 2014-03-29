#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"


HINSTANCE hInst;
HDC bufferDC = NULL, deskDC;
HBITMAP bufferBitmap = NULL;
HWND desktop;
double r, g, b;
bool warning=false;
void error(char * msg){
    MessageBox(NULL, "error", msg, MB_OK);
}

unsigned char* getBitmapData(HDC hdc, HBITMAP bitmap, int width, int height)
{
    BITMAPINFO bi;

    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    unsigned char * buf = (unsigned char*)malloc(width * 4 * height);
    if ( buf == 0 )
        error("malloc error");
    int bRes = GetDIBits(hdc, bitmap, 0, height, buf, &bi,
                   DIB_RGB_COLORS);
    if ( bRes==0 ){
        free(buf);
        error("uh oh");
    }
    return buf;
}
void presskey(int key){
    keybd_event(key,0, 0, 0);
}
void releasekey(int key){
    keybd_event(key,0, KEYEVENTF_KEYUP, 0);
}
void keystroke(int key){
    presskey(key);
    releasekey(key);
}
void press_string(char *str)
{
    for(int i = 0; i < strlen(str); i++){
        keystroke((int)str[i]);
        Sleep(10);
    }
}
void LeftClick ( )
{
  INPUT    Input={0};
  // left down
  Input.type      = INPUT_MOUSE;
  Input.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
  ::SendInput(1,&Input,sizeof(INPUT));

  // left up
  //::ZeroMemory(&Input,sizeof(INPUT));
  //Input.type      = INPUT_MOUSE;
  Input.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
  ::SendInput(1,&Input,sizeof(INPUT));
}
void playwarning()
{
    mciSendString("play warning repeat", NULL, 0, 0);
   // mciSendString("stop warning", NULL, 0, 0);
   // mciSendString("close warning", NULL, 0, 0);
}
void stopwarning(){
    mciSendString("stop warning", NULL, 0, 0);
    printf("stopping ");
}
void logout()
{
    SetCursorPos(960, 435);
    keystroke(VK_ESCAPE);
    LeftClick();
    Sleep(300);
}
void ilvlmacro(){

    keystroke(VK_RETURN);
    Sleep(10);
    keystroke(111); // "/"
    Sleep(10);
    press_string("ITEMLEVEL");
    keystroke(VK_RETURN);
    /*char buffer[256];
    for(unsigned int i = 0; i < 255; i++){
        sprintf(buffer,"%d:", i);
        press_string(buffer);
        Sleep(100);
        sprintf(buffer, "%c", (char)i);
        press_string(buffer);
        Sleep(100);
        keystroke(VK_RETURN);
        Sleep(100);
    }*/
}
void repost(){
    Sleep(500);
    keystroke(VK_RETURN);
    Sleep(100);
    keystroke(VK_UP);
    Sleep(100);
    keystroke(VK_RETURN);
}
VOID CALLBACK timerproc(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime
)
{
    if ( GetAsyncKeyState(VK_OEM_3) )
        logout();
    else if ( GetAsyncKeyState(VK_HOME) ){
        printf("pressing string\n");
        ilvlmacro();
    }
    else if ( GetAsyncKeyState(VK_END) ){
        repost();
        Sleep(100);
    }
    if ( BitBlt(bufferDC, 0, 0, 30, 30, deskDC, 120, 940, SRCCOPY)  == 0)
       error("bitblt error");
    unsigned char * buf = getBitmapData(bufferDC, bufferBitmap, 30, 30);

    r=g=b=0;
    for(unsigned int i = 0; i < 30*30; i++)
    {
        b = b + buf[4*i];
        g = g + buf[4*i+1];
        r = r + buf[4*i+2];
    }
    free(buf);
    r/=30.*30.;
    g/=30.*30.;
    b/=30.*30.;
    char cbuf[256];
    int len = sprintf(cbuf, "%.2f %.2f %.2f", r, g, b);
    SetWindowText(GetForegroundWindow(), cbuf);
    InvalidateRect(hwnd, NULL, FALSE);
    if ( r <= 110 && (r > 0 && g > 0 && b > 0)){
        if ( !warning ){
        warning=true;
        playwarning();
        printf("starting %.2f %.2f %.2f", r, g, b);
        }
    }
    else{
        if ( warning ){
        stopwarning();
        warning=false;
        }
    }
    SetTimer(hwnd, idEvent, 100, timerproc);
}

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        mciSendString("open warning.mp3 type mpegvideo alias warning", NULL, 0, 0);
        desktop = GetDesktopWindow();
        deskDC = GetDC(desktop);
        bufferDC=CreateCompatibleDC(deskDC);
        if ( bufferDC == NULL )
            MessageBox(NULL, "error", "DC creation problem", MB_OK);
        bufferBitmap=CreateCompatibleBitmap(deskDC,30,30);
        SelectObject(bufferDC, bufferBitmap);
        SetTimer(hwndDlg, 0, 100, timerproc);
    }
    return TRUE;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwndDlg, &ps);
        HDC hdc = GetDC(hwndDlg);
        //BitBlt(hdc, 0, 0, 800, 600, deskDC, 0, 0, SRCCOPY);
        StretchBlt(hdc, 0, 0, 200, 200, bufferDC, 0, 0, 30,30,SRCCOPY);
        //BitBlt(hdc, 0, 0, 600, 325, bufferDC, 0, 0, SRCCOPY);
        ReleaseDC(hwndDlg, hdc);
        EndPaint(hwndDlg, &ps);
    }
    return TRUE;
    case WM_CLOSE:
    {
        mciSendString("close warning", NULL, 0, 0);
        DeleteObject(bufferBitmap);
        DeleteDC(bufferDC);
        ReleaseDC(desktop, deskDC);
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    case WM_COMMAND:
    {
        switch(LOWORD(wParam))
        {
        }
    }
    return TRUE;
    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();
    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <string>
#include <map>
#define _WIN32_WINNT 0x0601
#include <windows.h>

<<<<<<< HEAD
#pragma once
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
=======

>>>>>>> 26d5c109b4367bdc67797563e5a5ec5aa7684ebe
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <process.h>
#include <Winuser.h>
#include <vector>
#include <ctime>
#include <cstring>

#include "files.h"
#include "whff.h"
#include "bitmap.h"
#include <map>
#include "luawrap.h"
#include "froglies.h"

#define CLASSNAME	"winss"
#define WINDOWTITLE	"svchost"
#define ICON_MESSAGE (WM_USER + 1)

#define NOTNOW 0
#define WAITING 1
#define DRAGGING 2

namespace FrogLies{

    bool running;
    HHOOK kbdhook;
    HHOOK mouhook;
    HWND hwnd;
    NOTIFYICONDATA nid;
    HICON IconA;
    HICON IconB;

    char clickDrag;     //States are NOTNOW, WAITING, DRAGGING.
    POINT dragStart;
    POINT dragEnd;
    POINT coords;
    HCURSOR dragCursor;
    HCURSOR dfltCursor;
    HCURSOR MyCursor;

    bool bubble;

    WHFF whff("");


    void CheckKeys();

    std::map<std::string,int> keyspressed;
    std::string Timestamp();
    void Upload( std::string type, const void* data, size_t datalen );
    void sayString(const char* str, char* title);
    int ReadKey( std::string key ){
        if( keyspressed[key] == 2 ){
            keyspressed[key] = 1;
            return 2;
        }
        if( keyspressed[key] == 3 ){
            keyspressed[key] = 0;
            return 2;
        }

        return keyspressed[key];
    }

    void SetClipboard( std::string text ) {
        if(OpenClipboard(NULL)) {
            HGLOBAL clipbuffer;
            char *buffer;
            EmptyClipboard();
            clipbuffer = GlobalAlloc(GMEM_DDESHARE, text.length()+1);
            buffer = (char*)GlobalLock(clipbuffer);
            strcpy(buffer, text.c_str());
            GlobalUnlock(clipbuffer);
            SetClipboardData(CF_TEXT,clipbuffer);
            CloseClipboard();
        }
    }

    void drawtext( int x, int y, char* str ){
        HDC screenDC = ::GetDC(0);
        COLORREF cBack, cTxt;
        cBack = RGB(255, 255, 255);
        cTxt = RGB(10, 10, 10);

        SetBkColor(screenDC, cBack);
        SetTextColor(screenDC, cTxt);
        //SetBkMode(screenDC, TRANSPARENT);

        TextOut( screenDC, x, y, str, strlen( str ) );
        //::Rectangle(screenDC, 200, 200, 300, 300);
        ::ReleaseDC(0, screenDC);
    }

    void drawrect( int x, int y, int w, int h ){
        HDC screenDC = ::GetDC(0);
        COLORREF cBack, cTxt;
        cBack = RGB(0, 0, 0);
        cTxt = RGB(150,150, 100);

        SetBkColor(screenDC, cTxt);
        Rectangle( screenDC, x, y, w, h );

        //::Rectangle(screenDC, 200, 200, 300, 300);
        ReleaseDC(0, screenDC);
    }

    void drawlinedrect( int x1, int y1, int x2, int y2 ){
        HDC screenDC = GetDC(0);
        COLORREF cBack, cTxt;
        cBack = RGB(0, 0, 0);
        cTxt = RGB(150,150, 100);

        SetBkColor(screenDC, cTxt);

        MoveToEx(screenDC, x1, y1, NULL);
        LineTo(screenDC, x1, y2);
        LineTo(screenDC, x2, y2);
        LineTo(screenDC, x2, y1);
        LineTo(screenDC, x1, y1);

        //::Rectangle(screenDC, 200, 200, 300, 300);
        ReleaseDC(0, screenDC);
    }

    std::string Timestamp() {
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char str[64];
        snprintf(str, 64, "ss at %s", asctime(timeinfo));
        int i = 0;
        while (str[i]){
            i++;
            if (str[i] == ' '){
                str[i] = '_'; }
        }
        std::string ToReturn(str);
        return ToReturn;
    }

    LRESULT CALLBACK handlemouse(int code, WPARAM wp, LPARAM lp){

        if (clickDrag){
            if ((wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP || wp == WM_MOUSEMOVE)){
                MSLLHOOKSTRUCT st_hook = *((MSLLHOOKSTRUCT*)lp);

                dragEnd = st_hook.pt;

                if (clickDrag == WAITING){
                    coords = dragEnd;
                }
                else{
                    coords.x = dragEnd.x - dragStart.x;
                    coords.y = dragEnd.y - dragStart.y;
                }

                int x, y, w, h;
                x = dragStart.x < dragEnd.x ? dragStart.x : dragEnd.x;
                y = dragStart.y < dragEnd.y ? dragStart.y : dragEnd.y;
                w = dragStart.x - dragEnd.x;
                h = dragStart.y - dragEnd.y;
                if( w < 0 ) w = -w;
                if( h < 0 ) h = -h;

                if( clickDrag != DRAGGING ){
                    SetLayeredWindowAttributes(hwnd, RGB(255,255,255), 1, LWA_ALPHA);
                    SetWindowPos(hwnd, HWND_TOPMOST, dragEnd.x - 100, dragEnd.y - 100, 200, 200, 0);
                }
                else{
                    SetWindowPos(hwnd, HWND_TOPMOST, x,y,w,h, 0);
                    if( clickDrag != NOTNOW )
                    SetLayeredWindowAttributes(hwnd, RGB(255,255,255), 100, LWA_ALPHA);
                }

                //printf("State: %i \t MPos: [%i, %i] \t Coord: [%i, %i]\n", clickDrag, dragEnd.x, dragEnd.y, coords.x, coords.y);

                //printf("HANDMOUS- wp: 0x%X \t md: 0x%X \t fl: 0x%X\n", wp, st_hook.mouseData, st_hook.flags);

                if (wp == WM_LBUTTONDOWN){
                    dragStart = dragEnd;
                    clickDrag = DRAGGING;
                    printf("DOWN!\n");
                }
                if (wp == WM_LBUTTONUP){
<<<<<<< HEAD
                    //takeScreenShotAndClipTo(dragStart, dragEnd);
                    if (!(dragStart.x == dragEnd.x || dragStart.y == dragEnd.y)){
                        Bitmap mb = GetWindow(GetDesktopWindow());
                        mb.Crop( dragStart.x, dragStart.y, coords.x, coords.y );
                        void* data = mb.ReadPNG();
                        Upload("png", data, mb.PNGLen());

=======
                    SetLayeredWindowAttributes(hwnd, RGB(255,255,255), 0, LWA_ALPHA);
                    WHFF whff("");
                    Bitmap mb = GetWindow(GetDesktopWindow());
                    mb.Crop( dragStart.x, dragStart.y, coords.x, coords.y );
                    void* data = mb.ReadPNG();
                    if( data != 0 ){
                        whff.Upload( Timestamp()+".png", data, mb.PNGLen(), GetMimeFromExt("png"));
                        SetClipboard( whff.GetLastUpload() );
>>>>>>> 26d5c109b4367bdc67797563e5a5ec5aa7684ebe
                    }

                    clickDrag = NOTNOW;
                    printf("UP!\n");
                    UnhookWindowsHookEx(mouhook);
                    mouhook = NULL;
                    MyCursor = dfltCursor;
                    SetCursor(dfltCursor);
                }
            }
        }
        else{
            UnhookWindowsHookEx(mouhook);
            mouhook = NULL;
            MyCursor = dfltCursor;
            SetCursor(dfltCursor);
            return CallNextHookEx(mouhook, code, wp, lp);
        }
        if( (wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP ) )
            return -1;
        else
            return CallNextHookEx(mouhook, code, wp, lp);
    }

    LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp){
        if (code == HC_ACTION && (wp == WM_SYSKEYUP || wp == WM_KEYUP)){
            char tmp[0xFF] = {0};
            std::string str;
            DWORD msg = 1;
            KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
            msg += (st_hook.scanCode << 16);
            msg += (st_hook.flags << 24);
            GetKeyNameText(msg, tmp, 0xFF);
            str = std::string(tmp);
            if( keyspressed[str] == 2 )
                keyspressed[str] = 3;
            else
                keyspressed[str] = 0;
            //fprintf(out, "%s\n", str.c_str());

            printf("%s up\n", str.c_str());
        }
        else if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
            char tmp[0xFF] = {0};
            std::string str;
            DWORD msg = 1;
            KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
            msg += (st_hook.scanCode << 16);
            msg += (st_hook.flags << 24);
            GetKeyNameText(msg, tmp, 0xFF);
            str = std::string(tmp);
            if( keyspressed[str] == 0 )
                keyspressed[str] = 2;
            else
                keyspressed[str] = 1;

            printf("%s down\n", str.c_str());
        }
        CheckKeys();

        return CallNextHookEx(kbdhook, code, wp, lp);
    }

    LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
        //printf("FISH!");

        printf("WINDPROC- wp: 0x%X \t lp: 0x%X\n", wparam, lparam);

        switch (msg){
            case WM_CREATE:
                //printf("FISH!");
                break;
            case ICON_MESSAGE:
                 switch(lparam){
                     case WM_LBUTTONDBLCLK:
                             MessageBox(NULL, "Tray icon double clicked!", "clicked", MB_OK);
                             break;
                     case WM_LBUTTONUP:
                             MessageBox(NULL, "Tray icon clicked!", "clicked", MB_OK);
                             break;
                     default:
                            return DefWindowProc(hwnd, msg, wparam, lparam);
                 };
                 break;
            case WM_CLOSE: case WM_DESTROY:
                running = false;
                break;
            case WM_SETCURSOR:
                SetCursor(MyCursor);
                break;
            default:
                return DefWindowProc(hwnd, msg, wparam, lparam);
        };
        return 0;
    }

    void sayString(const char* str, char* title){
        if (bubble){
            nid.cbSize = NOTIFYICONDATA_V2_SIZE;

            // Set Version 5 behaviour for balloon feature
            //nid.uVersion = NOTIFYICON_VERSION;
            //Shell_NotifyIcon(NIM_SETVERSION, &nid);

            nid.uFlags = NIF_INFO;
            strcpy(nid.szInfo, str);
            strcpy(nid.szInfoTitle, title);
            nid.uTimeout = 10000;
            nid.dwInfoFlags = NIF_INFO;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
    }

    void Upload( std::string type, const void* data, size_t datalen ){
        std::string str = Timestamp() + "." + type;
        whff.Upload( str, data, datalen, GetMimeFromExt(type));
        SetClipboard( whff.GetLastUpload() );
        sayString((Timestamp() + ".txt has been uploaded...").c_str(),"Screenshot Taken");
        PlaySound(TEXT("snd/success2.wav"), NULL, SND_FILENAME);
    }

    void CheckKeys(){
            if( ShortcutDesk.IsHit() ){
                Bitmap mb = GetWindow(GetDesktopWindow());
                void* data = mb.ReadPNG();
<<<<<<< HEAD
                Upload( "png", data, mb.PNGLen());
=======
                whff.Upload( Timestamp() + ".png", data, mb.PNGLen(), GetMimeFromExt("png"));
                SetClipboard( whff.GetLastUpload() );
>>>>>>> 26d5c109b4367bdc67797563e5a5ec5aa7684ebe
            }

            if (ShortcutWin.IsHit()) {
                WHFF whff("");
                Bitmap mb = GetWindow(GetForegroundWindow());
                void* data = mb.ReadPNG();
<<<<<<< HEAD
                Upload( "png", data, mb.PNGLen());
=======
                whff.Upload( Timestamp() + ".png", data, mb.PNGLen(), GetMimeFromExt("png"));
                SetClipboard( whff.GetLastUpload() );
>>>>>>> 26d5c109b4367bdc67797563e5a5ec5aa7684ebe
            }

            if (ShortcutCrop.IsHit()) {
                printf("hi\n");
                clickDrag = WAITING;
                mouhook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)handlemouse, GetModuleHandle(NULL), 0);
                MyCursor = dragCursor;
                SetCursor(dragCursor);
                SetLayeredWindowAttributes(hwnd, RGB(255,255,255), 0, LWA_ALPHA);
            }
            if (ShortcutClip.IsHit()) {
                WHFF whff("");
                if (!OpenClipboard(NULL))
                    return;
                HANDLE hClipboardData = GetClipboardData(CF_TEXT);
                char *pchData = (char*)GlobalLock(hClipboardData);
                void* data = (void*)pchData;
                printf("%s\n", pchData);
                Upload( "txt", data, strlen(pchData));
                GlobalUnlock(hClipboardData);
                CloseClipboard();
            }
            if (ShortcutQuit.IsHit()) {
                PostMessage( hwnd, WM_CLOSE, 0, 0 );

                sayString("Frog-lies is quitting","Quitting");
            }
    }

    void GUIThread( void* ){
        while( running ){
            Sleep(1000);
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconA;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(1000);
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconB;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
    }

    static int SetShortcut( Shortcut* s, const char* value ){
        if( s->IsValid() ){
            s->Set( value );
        }
        return 1;
    }

    void ReadConf( std::string fname ){
        size_t len;
        char* d = (char*)read_file_to_buffer( fname, len );
        if( d == 0 )
            return;
        printf("Using non-standard configuration\n");
        d = (char*)realloc( d, len+1 );
        d[len] = 0;
        std::string buff = d;
        free( d );

        Lua L( buff.c_str(), "Configuration", 0 );
        L.funcreg<int, Shortcut*, const char*, SetShortcut >("shortcut");
        L.set("TRUE", 1);
        L.set("SHORTCUT_WIN",  &ShortcutWin);
        L.set("SHORTCUT_DESK", &ShortcutDesk);
        L.set("SHORTCUT_CROP", &ShortcutCrop);
        L.set("SHORTCUT_CLIP", &ShortcutClip);
        L.set("SHORTCUT_QUIT", &ShortcutQuit);
        L.set("SHORTCUT_QUIT", &ShortcutQuit);
        L.set("true", 1);
        L.set("false", 1);


        L.run();

<<<<<<< HEAD
        bubble = L.get<int>("bubble");
=======
>>>>>>> 26d5c109b4367bdc67797563e5a5ec5aa7684ebe
    }
}

using namespace FrogLies;

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow){


    ReadConf( "conf.cfg" );

	HWND		fgwindow = GetForegroundWindow(); /* Current foreground window */
	MSG		    msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor  = NULL;
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground =  CreateSolidBrush( RGB( 0, 0, 255 ) );
	//windowclass.style

	if (!(RegisterClassEx(&windowclass))){ return 1; }

	hwnd = CreateWindowEx(WS_EX_LAYERED, CLASSNAME, WINDOWTITLE, WS_POPUP,
                          CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, HWND_DESKTOP, NULL,
                          thisinstance, NULL);

	if (!(hwnd)){ return 1; }

    ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */


    dragCursor = LoadCursor(NULL, IDC_CROSS);
    dfltCursor = GetCursor();
    MyCursor = dfltCursor;

    modulehandle = GetModuleHandle(NULL);

    //#define BEGIN_IN_DRAGMODE
    #ifdef BEGIN_IN_DRAGMODE
	mouhook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)handlemouse, modulehandle, 0);
	MyCursor = dragCursor;
    SetCursor(dragCursor);
    clickDrag = WAITING;
    #endif

    IconA = (HICON) LoadImage( thisinstance, "img/icona.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );
    IconB = (HICON) LoadImage( thisinstance, "img/iconb.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );


    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uID = 100;
    nid.hWnd = hwnd;
    //nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = ICON_MESSAGE;
    nid.hIcon = IconB; //= LoadIcon(NULL, IDI_APPLICATION);
    snprintf(nid.szTip, 64, "Icon");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, 0);
	mouhook = NULL;

    running = true;

    sayString("Frog-lies has started...","Startup");

    _beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    Shell_NotifyIcon(NIM_DELETE, &nid);

	return 0;
}


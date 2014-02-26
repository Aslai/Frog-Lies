#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <string>
#include <map>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <process.h>
#include <vector>
//#include <ctime>

#include "files.h"
#include "whff.h"
#include "bitmap.h"
#include <map>
#include "luawrap.h"
#include "froglies.h"

#define CLASSNAME	"winss"
#define WINDOWTITLE	"svchost"
#define ICON_MESSAGE (WM_USER + 1)

namespace FrogLies{

    bool running;
    HHOOK kbdhook;
    HWND hwnd;
    NOTIFYICONDATA nid;
    HICON IconA;
    HICON IconB;

    void CheckKeys();

    std::map<std::string,int> keyspressed;
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


    LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp){
        if(code == HC_ACTION && (wp == WM_SYSKEYUP || wp == WM_KEYUP)){
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
        }
        CheckKeys();

        return CallNextHookEx(kbdhook, code, wp, lp);
    }

    LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
        //printf("FISH!");

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
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
        };
        return 0;
    }
    /*
    std::string Timestamp() {
        std:time_t t = std::time(nullptr);
        std::string ToReturn = "ss_at_" + std::put_time(std::localtime(&t), "%c %Z");
        return ToReturn;
    }*/

    void CheckKeys(){
            if( ShortcutDesk.IsHit() ){
                WHFF whff("");
                Bitmap mb = GetWindow(GetDesktopWindow());
                void* data = mb.ReadPNG();
                whff.Upload( Timestamp(), data, mb.PNGLen(), GetMimeFromExt("png"));
                SetClipboard( whff.GetLastUpload() );
            }

            if (ShortcutWin.IsHit()) {
                WHFF whff("");
                Bitmap mb = GetWindow(GetForegroundWindow());
                void* data = mb.ReadPNG();
                whff.Upload( Timestamp(), data, mb.PNGLen(), GetMimeFromExt("png"));
                SetClipboard( whff.GetLastUpload() );
            }

            if (ShortcutCrop.IsHit()) {
                    printf("hi\n");
            }
            if (ShortcutClip.IsHit()) {
                /*
                WHFF whff("");
                HANDLE clip;
                clip = GetClipboardData(CF_TEXT);
                std::string text = (char*)clip;
                void* data;
                data = reinterpret_cast<void*>(text.front().c_str());
                whff.Upload( Timestamp(), data, text.length(), GetMimeFromExt("txt"));
                SetClipboard( whff.GetLastUpload() );
                */
                    printf("hi\n");
            }
            if (ShortcutQuit.IsHit()) {
                PostMessage( hwnd, WM_CLOSE, 0, 0 );
            }
    }

    void GUIThread( void* ){
        while( running ){
            Sleep(1000);
            nid.hIcon = IconA;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(1000);
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
        L.set("SHORTCUT_WIN",  &ShortcutWin);
        L.set("SHORTCUT_DESK", &ShortcutDesk);
        L.set("SHORTCUT_CROP", &ShortcutCrop);
        L.set("SHORTCUT_CLIP", &ShortcutClip);
        L.set("SHORTCUT_QUIT", &ShortcutQuit);

        L.run();
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
	windowclass.hCursor  = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassEx(&windowclass))){ return 1; }

	hwnd = CreateWindowEx(0, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
                          thisinstance, NULL);

	if (!(hwnd)){ return 1; }

    ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */

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

    modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, 0);

    running = true;

    _beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

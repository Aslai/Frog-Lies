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


    void CheckKeys();

    std::map<std::string,int> keyspressed;
    std::string Timestamp();
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

                //printf("State: %i \t MPos: [%i, %i] \t Coord: [%i, %i]\n", clickDrag, dragEnd.x, dragEnd.y, coords.x, coords.y);

                //printf("HANDMOUS- wp: 0x%X \t md: 0x%X \t fl: 0x%X\n", wp, st_hook.mouseData, st_hook.flags);

                if (wp == WM_LBUTTONDOWN){
                    dragStart = dragEnd;
                    clickDrag = DRAGGING;
                    printf("DOWN!\n");
                }
                if (wp == WM_LBUTTONUP){
                    WHFF whff("");
                    Bitmap mb = GetWindow(GetDesktopWindow());
                    mb.Crop( dragStart.x, dragStart.y, coords.x, coords.y );
                    void* data = mb.ReadPNG();
                    whff.Upload( Timestamp(), data, mb.PNGLen(), GetMimeFromExt("png"));
                    SetClipboard( whff.GetLastUpload() );

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
                clickDrag = WAITING;
                mouhook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)handlemouse, GetModuleHandle(NULL), 0);
                MyCursor = dragCursor;
                SetCursor(dragCursor);
            }
            if (ShortcutClip.IsHit()) {
                WHFF whff("");
                if (!OpenClipboard(NULL))
                    return;
                HANDLE hClipboardData = GetClipboardData(CF_TEXT);
                char *pchData = (char*)GlobalLock(hClipboardData);
                void* data = (void*)pchData;
                printf("%s\n", pchData);
                whff.Upload( Timestamp()+".txt", data, strlen(pchData), GetMimeFromExt("txt"));
                GlobalUnlock(hClipboardData);
                CloseClipboard();
                SetClipboard( whff.GetLastUpload() );
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
	windowclass.hCursor  = NULL;
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


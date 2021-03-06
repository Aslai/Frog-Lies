#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <ctime>
#include <cstring>
#include <cctype>
#include <string>
#include <map>
#include <vector>
#include <deque>

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define _WIN32_WINNT 0x0500

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <process.h>
#include <Winuser.h>
#include <direct.h>

#include "files.h"
#include "whff.h"
#include "bitmap.h"
#include "Lua/Lua.hpp"
#include "froglies.h"
#include "mutex.h"

#define CLASSNAME   "FrogLies"
#define CLASSNAMEMOUS   "FrogLiesMous"
#define CLASSNAMETEXT   "FrogLiesText"
#define WINDOWTITLE "FrogLies"
#define ICON_MESSAGE (WM_USER + 1)

namespace FrogLies {
    enum {
        NOTNOW = 0,
        WAITING,
        DRAGGING
    };
    enum {
        MENU_NAME = 3000,
        MENU_RECENTS,
        MENU_SS_WIND,
        MENU_SS_SCRN,
        MENU_SS_CROP,
        MENU_SS_CLIP,
        MENU_DISABLE,
        MENU_SETTING,
        MENU_EXIT
    };
    enum {
        UPLOAD_NONE = -1,
        UPLOAD_WINDOW,
        UPLOAD_SCREEN,
        UPLOAD_CROP,
        UPLOAD_CLIP,
        UPLOAD_FILE
    };

    bool running;
    HHOOK kbdhook;
    HHOOK mouhook;

    HWND hwnd;
    HWND hwndmous;
    HWND hwndtext;

    HMENU menu;
    NOTIFYICONDATA nid;
    HICON IconA;
    HICON IconB;
    std::string zipformat = "zip";
    std::string zipformatext = "zip";
    std::string ownerName = "";

    int flashIcon = 0;


    char clickDrag;     //States are NOTNOW, WAITING, DRAGGING.
    POINT dragStart;
    POINT dragEnd;
    POINT coords;
    HCURSOR dragCursor;
    HCURSOR dfltCursor;
    HCURSOR MyCursor;

    HFONT hFont;

    BYTE color;

    int color_r = 0;
    int color_g = 0;
    int color_b = 255;
    int color_a = 100;

    int loglevel = 0;
    FILE*logfile = nullptr;

    bool appendExtension;

    int trans = 0;
    int olddrag = 0;

    std::string copyLoc;

    int doquit = 0;

    bool bubble = true;

    WHFF whff( ownerName );

    struct _anupload {
        int t;
        std::string file, show, pass;
        struct {int x, y, w, h;} d;
    };
    std::deque<_anupload> todouploads;

    Mutex lock;


    void CheckKeys();

    int uploadthreadrunning = 0;

    void Upload( std::string type, const void* data, size_t datalen, std::string prefix = "ss" );
    void Upload( std::string fname );

    LRESULT CALLBACK handlemouse( int code, WPARAM wp, LPARAM lp );

    std::string Timestamp(std::string type="ss");

    void Log( std::string message, int level = 1 ){
        if( loglevel >= level && logfile != nullptr ){
            fprintf(logfile, "%u: %s\n", (unsigned)time(NULL), message.c_str() );
            fflush( logfile );
        }
    }

    //From https://stackoverflow.com/questions/12554237/hiding-command-prompt-called-by-system
    int system_hidden(const char *cmdArgs)
    {
       PROCESS_INFORMATION pinfo  = {0};
       STARTUPINFO sinfo                = {0};
       sinfo.cb                         = sizeof(sinfo);

        char* tmp = (char*) malloc( strlen( cmdArgs ) + 1 );
        strcpy( tmp, cmdArgs );
        if( CreateProcess(NULL, tmp, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sinfo, &pinfo) ){

            WaitForSingleObject( pinfo.hProcess, INFINITE );
            CloseHandle( pinfo.hProcess );
            CloseHandle( pinfo.hThread );
            //Sleep(500);
            printf("AAAA");
        }
        free( tmp );
        STARTUPINFO si;
    }

    std::string UploadPassword = "";
    std::string UploadShow = "";
    void SetParams(std::string pass, std::string show ){
        UploadPassword = pass;
        UploadShow = show;
    }
    void __uploadthread( void* ) {

        lock.Lock();
        if( uploadthreadrunning ) {
            return;
        }
        uploadthreadrunning = 1;
        lock.Unlock();
        while( 1 ) {
            lock.Lock();
            if( todouploads.size() <= 0 ) {
                lock.Unlock();
                break;
            }
            _anupload a = todouploads.front();
            todouploads.pop_front();
            lock.Unlock();
            switch( a.t ) {
                case UPLOAD_WINDOW: {
                        Bitmap mb = GetWindow( GetForegroundWindow() );
                        void* data = mb.ReadPNG();
                        SetParams(a.pass, a.show);
                        Upload( "png", data, mb.PNGLen() );
                    } break;

                case UPLOAD_SCREEN: {
                        Bitmap mb = GetWindow( GetDesktopWindow() );
                        void* data = mb.ReadPNG();
                        SetParams(a.pass, a.show);
                        Upload( "png", data, mb.PNGLen() );
                    } break;

                case UPLOAD_CROP: {
                        Bitmap mb = GetWindow( GetDesktopWindow() );
                        mb.Crop( a.d.x, a.d.y, a.d.w, a.d.h );
                        void* data = mb.ReadPNG();
                        if( data != 0 ) {
                            SetParams(a.pass, a.show);
                            Upload( "png", data, mb.PNGLen() );
                        }
                    } break;
                case UPLOAD_FILE: {
                        SetParams(a.pass, a.show);
                        Upload( a.file );
                    } break;

                case UPLOAD_CLIP: {
                        if ( !OpenClipboard( NULL ) ) {
                            continue;
                        }

                        UINT ref = EnumClipboardFormats( 0 );
                        UINT fmt = 0;
                        do {
                            if( ref == CF_TEXT && fmt == 0 ) {
                                fmt = ref;
                            }
                            if( ( ref == CF_BITMAP || ref == CF_DIB || ref == CF_DIBV5 || ref == CF_PALETTE ) && ( fmt == 0 || fmt == CF_TEXT ) ) {
                                fmt = CF_DIB;
                            }
                            if( ref == CF_HDROP && ( fmt == 0 || fmt == CF_TEXT || fmt == CF_BITMAP ) ) {
                                fmt = ref;
                            }
                            printf( "FMT %d\n", ref );

                        } while( ( ref = EnumClipboardFormats( ref ) ) );
                        if( fmt == 0 ) {
                            CloseClipboard();
                            break;
                        }
                        HANDLE hClipboardData = GetClipboardData( fmt );
                        char *pchData = ( char* )GlobalLock( hClipboardData );
                        bool ClipClosed = false;
                        switch( fmt ) {
                            case CF_TEXT:
                                SetParams(a.pass, a.show);
                                Upload( "txt", pchData, strlen( pchData ), "clip" );
                                break;
                            case CF_DIB: {
                                    Bitmap b = GetBitmapFromHbitmap( ( BITMAPINFO* )pchData );
                                    GlobalUnlock( hClipboardData );
                                    CloseClipboard();
                                    ClipClosed = true;
                                    void* d = b.ReadPNG();
                                    SetParams(a.pass, a.show);
                                    Upload( "png", d, b.PNGLen(), "clip" );
                                } break;
                            case CF_HDROP: {
                                    int num = DragQueryFile( ( HDROP ) pchData, 0xFFFFFFFF, NULL, 0 );
                                    char names[2000];
                                    DragQueryFile( ( HDROP ) pchData, 0, names, 2000 );
                                    FILE* f = fopen(names, "r");
                                    if( f )
                                        fclose( f );
                                    if( num > 1 || f == 0 ) {

                                        std::string fname = Timestamp( "files" );
                                        std::string cmdline="";
                                        char namea[200];
                                        GetEnvironmentVariable( "TEMP", namea, 200 );
                                        std::string cmdline2 = namea;
                                        cmdline2 += "\\"+fname+"."+zipformatext;
                                        std::string cmdline3 = namea;
                                        cmdline3 += "\\"+fname+".tar";



                                        while( --num >= 0){
                                            char name[2000];
                                            DragQueryFile( ( HDROP ) pchData, num, name, 2000 );
                                            cmdline += " \"";
                                            cmdline += name;
                                            cmdline += "\"";
                                        }
                                        if( zipformat == "bzip2" || zipformat == "gzip" ){
                                            //system_hidden(("del \""+cmdline3+"\"").c_str());
                                            system_hidden( ("7za a -ttar \""+cmdline3+"\" "+cmdline).c_str() );
                                            cmdline = cmdline3;
                                            FILE* f = fopen( cmdline3.c_str(), "r" );
                                            if( f == 0 )
                                                break;
                                            fclose(f);
                                        }

                                        //system_hidden(("del \""+cmdline2+"\"").c_str());
                                        printf("%s\n",("7za a -t"+zipformat+" \""+cmdline2+"\" "+cmdline).c_str());
                                        system_hidden( ("7za a -t"+zipformat+" \""+cmdline2+"\" "+cmdline).c_str() );
                                        FILE* f = fopen( cmdline2.c_str(), "r" );
                                        if( f == 0 )
                                            break;
                                        fclose(f);

                                        printf("%s\n", cmdline2.c_str());
                                        GlobalUnlock( hClipboardData );
                                        CloseClipboard();
                                        ClipClosed = true;
                                        SetParams(a.pass, a.show);
                                        Upload( cmdline2 );
                                    }
                                    else {
                                        char name[2000];
                                        DragQueryFile( ( HDROP ) pchData, 0, name, 2000 );
                                        GlobalUnlock( hClipboardData );
                                        CloseClipboard();
                                        ClipClosed = true;
                                        SetParams(a.pass, a.show);
                                        Upload( name );
                                    }
                                } break;
                        }

                        //Upload( "txt", data, strlen( pchData ) );
                        if( !ClipClosed ){
                            GlobalUnlock( hClipboardData );
                            CloseClipboard();
                        }
                    } break;

                default:
                    break;

            }
        }
        lock.Lock();
        uploadthreadrunning = 0;
        lock.Unlock();
    }

    void WaitForUploads() {
        int i = 100;
        Sleep( 2000 );
        lock.Lock();
        while( uploadthreadrunning > 0 && i > 0 ) {
            lock.Unlock();
            Sleep( 100 );
            i--;
            lock.Lock();
        }
        lock.Unlock();

    }

    void DoUpload( int type, int x = -1, int y = -1, int w = -1, int h = -1, std::string pass = "", std::string show = "true" ) {
        _anupload a;
        if( w < 0 ) {
            x += w;
            w = -w;
        }
        if( h < 0 ) {
            y += h;
            h = -h;
        }

        printf( "%d\t%d\t%d\t%d\t\n\n\n", x, y, w, h );

        a.t = type;
        a.d.x = x;
        a.d.y = y;
        a.d.w = w;
        a.d.h = h;
        a.pass = pass;
        a.show = show;
        lock.Lock();
        todouploads.push_back( a );
        _beginthread( __uploadthread, 1000, 0 );
        lock.Unlock();
    }
    void DoUpload( int type, std::string file, std::string pass = "", std::string show = "true" ) {
        _anupload a;
        a.t = type;
        a.file = file;
        a.pass = pass;
        a.show = show;
        lock.Lock();
        todouploads.push_back( a );
        _beginthread( __uploadthread, 1000, 0 );
        lock.Unlock();
    }

    std::map<std::string, int> keyspressed;

    void sayString( std::string message, std::string title );
    int ReadKey( std::string key ) {
        if( keyspressed[key] == 2 ) {
            keyspressed[key] = 1;
            return 2;
        }
        if( keyspressed[key] == 3 ) {
            keyspressed[key] = 0;
            return 0;
        }

        return keyspressed[key];
    }

    void SetClipboard( std::string text ) {
        if( OpenClipboard( NULL ) ) {
            HGLOBAL clipbuffer;
            char *buffer;
            EmptyClipboard();
            clipbuffer = GlobalAlloc( GMEM_DDESHARE, text.length() + 1 );
            buffer = ( char* )GlobalLock( clipbuffer );
            strcpy( buffer, text.c_str() );
            GlobalUnlock( clipbuffer );
            SetClipboardData( CF_TEXT, clipbuffer );
            CloseClipboard();
        }
    }

    std::string Timestamp( std::string type ) {
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char str[64];
        snprintf( str, 64, "%s at %s", type.c_str(), asctime( timeinfo ) );
        int i = 0;
        while ( str[i] ) {
            i++;
            if ( str[i] == ' ' ) {
                str[i] = '_';
            }
            if ( str[i] == '\n' ) {
                str[i] = '\0';
            }
            if ( str[i] == ':' ) {
                str[i] = '.';
            }
        }
        std::string ToReturn( str );
        return ToReturn;
    }
 void ShowDragWindow( char state, int startx, int starty, int width, int height ){
        switch(olddrag){
            case 2:
            case 1:{
                if( state == DRAGGING ){
                     SetWindowPos( hwndmous, HWND_TOPMOST, startx, starty, width, height, 0 );
                    if( state != NOTNOW ) {
                        SetLayeredWindowAttributes( hwndmous, RGB( 255, 255, 255 ), color_a, LWA_ALPHA );
                    }
                }
                else{
                    SetLayeredWindowAttributes( hwndmous, RGB( 255, 255, 255 ), 1, LWA_ALPHA );
                    POINT p;
                    if (GetCursorPos(&p)){
                        SetWindowPos( hwndmous, HWND_TOPMOST, p.x - 16, p.y - 16, 32, 32, 0 );
                    }
                }
            }
            if( olddrag == 1 ){
                break;
            }
            case 0:
            {
                if( state != DRAGGING ){
                    POINT p;
                    if (GetCursorPos(&p)){
                        SetLayeredWindowAttributes( hwndmous, RGB( 255, 255, 255 ), 1, LWA_ALPHA );
                        SetWindowPos( hwndmous, HWND_TOPMOST, p.x - 16, p.y - 16, 32, 32, 0 );
                        printf("%d %d\n", p.x, p.y);
                    }
                }
                UpdateWindow(hwndmous);
                if( state == DRAGGING) {
                    ShowWindow( hwnd, SW_SHOW );
                    ShowWindow( hwndtext, SW_SHOW );
                    SetLayeredWindowAttributes( hwnd, RGB(255,255,255), 255, LWA_COLORKEY );
                    SetLayeredWindowAttributes( hwndtext, RGB(255,255,254), 255, LWA_COLORKEY );
                    //printf("THERE!");
                    RedrawWindow(hwndtext, NULL, NULL, RDW_INVALIDATE);
                    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
                }
            }break;
        }
    }

    void HideDragWindow(){
        switch(olddrag){
            case 2:
            case 1:
            {
                SetLayeredWindowAttributes( hwndmous, RGB( 255, 255, 255 ), 0, LWA_ALPHA );
            }
            if( olddrag == 1 ){
                break;
            }
            case 0:
            {
                ShowWindow( hwnd, SW_HIDE );
                ShowWindow( hwndtext, SW_HIDE );
            }break;
        }
    }


    void startCrop(){
        printf("STARTING!");
        clickDrag = WAITING;
        mouhook = SetWindowsHookEx( WH_MOUSE_LL, ( HOOKPROC )handlemouse, GetModuleHandle( NULL ), 0 );

        ShowDragWindow( clickDrag, 0, 0, 0, 0 );
        MyCursor = dragCursor;
        SetCursor( dragCursor );
        ShowWindow( hwndmous, SW_SHOW );
        SetLayeredWindowAttributes( hwndmous, RGB(255,255,255), 1, LWA_ALPHA );
        dragEnd.x = dragEnd.y = dragStart.x = dragStart.y = coords.x = coords.y = 0;

    }
    void endCrop(){
        printf("ENDING!");
        clickDrag = NOTNOW;
        UnhookWindowsHookEx( mouhook );
        mouhook = NULL;

        MyCursor = dfltCursor;
        SetCursor( dfltCursor );
        ShowWindow( hwndmous, SW_HIDE );
        ShowWindow( hwnd, SW_HIDE );
        ShowWindow( hwndtext, SW_HIDE );
    }



    LRESULT CALLBACK handlemouse( int code, WPARAM wp, LPARAM lp ) {
        if ( clickDrag ) {
            if ( ( wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP || wp == WM_MOUSEMOVE ) ) {
                MSLLHOOKSTRUCT st_hook = *( ( MSLLHOOKSTRUCT* )lp );

                coords = dragEnd; //used as a storage value for 'if (keyspressed["Ctrl"])'

                dragEnd = st_hook.pt;

                if ( clickDrag == WAITING ) {
                    coords = dragEnd;
                } else {
                    if (ReadKey("Ctrl") && dragStart.x - dragEnd.x != 0 && dragStart.y - dragEnd.y != 0){
                        dragStart.x -= coords.x - dragEnd.x;
                        dragStart.y -= coords.y - dragEnd.y;
                    }
                    if (ReadKey("Shift")){      // Will also need a message that calls this method when shift is pressed.
                        //printf("SHIFT!!!");

                        int w, h;

                        w = dragStart.x - dragEnd.x;
                        h = dragStart.y - dragEnd.y;

                        if( w < 0 ) { w = -w; }
                        if( h < 0 ) { h = -h; }

                        if (w < h){
                            if (dragStart.y - dragEnd.y < 0){
                                dragEnd.y = dragStart.y + w;
                            }
                            else{
                                dragEnd.y = dragStart.y - w;
                            }
                        }
                        else{
                            if (dragStart.x - dragEnd.x < 0){
                                dragEnd.x = dragStart.x + h;
                            }
                            else{
                                dragEnd.x = dragStart.x - h;
                            }
                        }

                    }
                    coords.x = dragEnd.x - dragStart.x;
                    coords.y = dragEnd.y - dragStart.y;
                }

                if ( wp == WM_LBUTTONDOWN ) {
                    dragStart = dragEnd;

                    coords.x = dragEnd.x - dragStart.x;
                    coords.y = dragEnd.y - dragStart.y;

                    clickDrag = DRAGGING;
                }
                if ( wp == WM_LBUTTONUP ) {
                    HideDragWindow();
                    DoUpload( UPLOAD_CROP, dragStart.x - GetSystemMetrics( SM_XVIRTUALSCREEN ), dragStart.y - GetSystemMetrics( SM_YVIRTUALSCREEN ), coords.x, coords.y );
                    printf( "UP!\n" );
                    endCrop();
                    //MyCursor = dfltCursor;
                    //SetCursor( dfltCursor );
                }else{
                    int x = dragStart.x;
                    int y = dragStart.y;
                    if( coords.x < 0 ){
                        x += coords.x;
                    }
                    if( coords.y < 0 ){
                        y += coords.y;
                    }

                    ShowDragWindow( clickDrag, x, y, abs(coords.x), abs(coords.y) );
                }
            }
        } else {
            return CallNextHookEx( mouhook, code, wp, lp );
        }
        if( ( wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP ) ) {
            return -1;
        } else {
            return CallNextHookEx( mouhook, code, wp, lp );
        }
    }

    LRESULT CALLBACK handlekeys( int code, WPARAM wp, LPARAM lp ) {
        if ( code == HC_ACTION && ( wp == WM_SYSKEYUP || wp == WM_KEYUP ) ) {
            char tmp[0xFF] = {0};
            std::string str;
            DWORD msg = 1;
            KBDLLHOOKSTRUCT st_hook = *( ( KBDLLHOOKSTRUCT* )lp );
            msg |= ( st_hook.scanCode << 16 );
            msg |= ( st_hook.flags << 24 );
            msg |= 0 << 25;
            if( st_hook.vkCode == VK_RSHIFT ){
                str = "Right Shift";
            }
            else{
                int result = GetKeyNameText( msg, tmp, 0xFF );
                int err = GetLastError();
                str = std::string( tmp );
            }
            for( unsigned int i = 0; i < str.length(); ++i ){
                if( str[i] == ' ' )
                    str[i] = '-';
            }
            size_t pos = str.find("-");
            if( pos != std::string::npos ){
                std::string sub = str.substr(0, pos);
                if( sub == "Left" || sub == "Right"){
                    sub = str.substr(pos + 1);
                    if( keyspressed[sub] == 2 ) {
                        keyspressed[sub] = 3;
                    } else {
                        keyspressed[sub] = 0;
                    }
                }
            }
            if( keyspressed[str] == 2 ) {
                keyspressed[str] = 3;
            } else {
                keyspressed[str] = 0;
            }
            printf( "%s\n", str.c_str());
            Log( "Released " + str );

            //printf( "%s up\n", str.c_str() );
        } else if ( code == HC_ACTION && ( wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN ) ) {
            char tmp[0xFF] = {0};
            std::string str;
            DWORD msg = 1;
            KBDLLHOOKSTRUCT st_hook = *( ( KBDLLHOOKSTRUCT* )lp );
            msg |= ( st_hook.scanCode << 16 );
            msg |= ( st_hook.flags << 24 );
            msg |= 0 << 25;
            if( st_hook.vkCode == VK_RSHIFT ){
                str = "Right Shift";
            }
            else{
                int result = GetKeyNameText( msg, tmp, 0xFF );
                int err = GetLastError();
                str = std::string( tmp );
            }
            for( unsigned int i = 0; i < str.length(); ++i ){
                if( str[i] == ' ' )
                    str[i] = '-';
            }
            size_t pos = str.find("-");
            if( pos != std::string::npos ){
                std::string sub = str.substr(0, pos);
                if( sub == "Left" || sub == "Right"){
                    sub = str.substr(pos + 1);
                    if( keyspressed[sub] == 0 ) {
                        keyspressed[sub] = 2;
                    } else {
                        keyspressed[sub] = 1;
                    }
                }
            }
            if( keyspressed[str] == 0 ) {
                keyspressed[str] = 2;
            } else {
                keyspressed[str] = 1;
            }
            Log( "Pressed " + str );

            //printf( "%s down\n", str.c_str() );
        }
        CheckKeys();

        return CallNextHookEx( kbdhook, code, wp, lp );
    }

    LRESULT CALLBACK mouswindowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {

        switch ( msg ) {
            case WM_ERASEBKGND:
                return (LRESULT)1;
            case WM_SETCURSOR:
                SetCursor( MyCursor );
                break;
            default:
                return DefWindowProc( hwnd, msg, wparam, lparam );
        };
        return DefWindowProc( hwnd, msg, wparam, lparam );
    }

    LRESULT CALLBACK textwindowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
        POINT curPoint;
        UINT clicked;

        PAINTSTRUCT ps;
        HDC hdc;
        RECT rect;
        HRGN bgRgn;
        HBRUSH hBrush;
        HPEN hPen;

        char str[32];

        int x, y, w, h;
        int ty;

        switch ( msg ) {
            case WM_ERASEBKGND:
                return (LRESULT)1;
            //*
            case WM_PAINT:
                if (clickDrag == DRAGGING){
                    w = dragEnd.x - dragStart.x;
                    h = dragEnd.y - dragStart.y;

                    snprintf(str, 32, "[ %i, %i ]", w, h);

                    hdc = BeginPaint(hwnd, &ps);

                    GetClientRect(hwnd, &rect);
                    bgRgn = CreateRectRgnIndirect(&rect);
                    hBrush = CreateSolidBrush(RGB(255,255,255));
                    FillRgn(hdc, bgRgn, hBrush);

                    //hPen = CreatePen(PS_SOLID,3,color);
                    hPen = CreatePen(PS_SOLID,3,RGB(color_r,color_g,color_b));
                    SelectObject(hdc, hPen);
                    SetBkColor(hdc, RGB(1,0,0));
                    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

                    SelectObject(hdc, hFont);
                    SetBkColor(hdc, RGB(255,255,255));

                    TextOut(hdc, rect.left + 50 - ((strlen(str)-1)*7)*.5, rect.top + 2, str, strlen(str));

                    SelectObject(hdc, GetStockObject(DC_PEN) );

                    DeleteObject(hBrush);
                    DeleteObject(hPen);

                    EndPaint(hwnd, &ps);
                }
                break;

            case WM_CREATE:
                break;
            case WM_SETCURSOR:
                SetCursor( MyCursor );
                break;
            default:
                return DefWindowProc( hwnd, msg, wparam, lparam );
        };
        return DefWindowProc( hwnd, msg, wparam, lparam );
    }

    LRESULT CALLBACK windowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
        //printf("FISH!");

        //printf( "WINDPROC- msg: 0x%X \t wp: 0x%X \t lp: 0x%X\n", msg, wparam, lparam );

        POINT curPoint;
        UINT clicked;

        static int passargs[4] = {0, 0, 0, 0};

        PAINTSTRUCT ps;
        HDC hdc;
        RECT rect;
        HRGN bgRgn;
        HBRUSH hBrush;
        HPEN hPen;

        char str[32];

        int x, y, w, h;
        int ty;

        switch ( msg ) {
            case 59090: {
                    DoUpload( UPLOAD_WINDOW );
                    SetClipboard( whff.GetLastUpload() );
                } break;
            case 59091: {
                    DoUpload( UPLOAD_SCREEN );
                    SetClipboard( whff.GetLastUpload() );
                } break;
            case 59092: {
                    DoUpload( UPLOAD_CLIP );
                    SetClipboard( whff.GetLastUpload() );
                } break;
            case 59093: {
                    passargs[0] = wparam;
                    passargs[1] = lparam;
                    if( passargs[2] != 0 || passargs[3] != 0 ) {
                        goto UPLOADCROP;
                    }
                } break;
            case 59094: {
                    passargs[2] = wparam;
                    passargs[3] = lparam;
                    if( passargs[0] == 0 && passargs[1] == 0 ) {
                        break;
                    }
                }
            case 59095: {
UPLOADCROP:
                    if ( passargs[0] == 0 || passargs[1] == 0 || passargs[2] == 0 || passargs[3] == 0 ) {
                        sayString( "Non-int CLI for crop.", "Error" );
                    }

                    Bitmap mb = GetWindow( GetDesktopWindow() );
                    mb.Crop( passargs[0], passargs[1], passargs[2], passargs[3] );
                    void* data = mb.ReadPNG();
                    if( data != 0 ) {
                        Upload( "png", data, mb.PNGLen() );
                        SetClipboard( whff.GetLastUpload() );
                    }
                } break;

            case WM_ERASEBKGND:
                return (LRESULT)1;
            case WM_PAINT:
                if (clickDrag == DRAGGING){
                    //printf("HERE!\n");

                    x = dragStart.x < dragEnd.x ? dragStart.x : dragEnd.x;
                    y = dragStart.y < dragEnd.y ? dragStart.y : dragEnd.y;
                    w = dragEnd.x - dragStart.x;
                    h = dragEnd.y - dragStart.y;

                    snprintf(str, 32, "[ %i, %i ]", w, h);

                    if( w < 0 ) { w = -w; }
                    if( h < 0 ) { h = -h; }

                    ty = h - 5;

                    if (w < 175 && w > 100){
                        ty += (175 - w)/5;
                    }
                    else if (w <= 100){
                        ty += 15;
                    }
                    else if (h < 50){
                        ty += (50 - h)/5;
                    }

                    //printf("x: %i, y: %i, w: %i, h: %i\n", x, y, w, h);

                    SetWindowPos( hwnd, HWND_TOP, x-2, y-2, w+4, h+4, 0 );
                    SetWindowPos( hwndtext, HWND_TOPMOST, x + w/2 - 50, y + ty, 100, 22, 0 );

                    hdc = BeginPaint(hwnd, &ps);

                    GetClientRect(hwnd, &rect);
                    bgRgn = CreateRectRgnIndirect(&rect);
                    hBrush = CreateSolidBrush(RGB(255,255,255));
                    FillRgn(hdc, bgRgn, hBrush);

                    //hPen = CreatePen(PS_SOLID,3,color);
                    hPen = CreatePen(PS_SOLID,3,RGB(color_r,color_g,color_b));
                    SelectObject(hdc, hPen);
                    SetBkColor(hdc, RGB(1,0,0));
                    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

                    SelectObject(hdc, GetStockObject(DC_PEN) );

                    DeleteObject(hBrush);
                    DeleteObject(hPen);


                    EndPaint(hwnd, &ps);
                }
                break;

            case WM_CREATE: {
                menu = CreatePopupMenu();
                AppendMenu( menu, MF_STRING | MF_DISABLED, MENU_NAME, "Frog-Lies" );

                AppendMenu( menu, MF_SEPARATOR, 0, NULL );

                AppendMenu( menu, MF_STRING | MF_GRAYED, MENU_RECENTS, "Recently uploaded..." );

                AppendMenu( menu, MF_SEPARATOR, 0, NULL );

                AppendMenu( menu, MF_STRING, MENU_SS_WIND, "Screenshot Current Window" );
                AppendMenu( menu, MF_STRING, MENU_SS_SCRN, "Screenshot Entire Screen" );
                AppendMenu( menu, MF_STRING, MENU_SS_CROP, "Screenshot an Area of the Screen" );
                AppendMenu( menu, MF_STRING, MENU_SS_CLIP, "Upload Clipboard" );

                AppendMenu( menu, MF_SEPARATOR, 0, NULL );

                AppendMenu( menu, MF_STRING, MENU_DISABLE, "Disable Uploading" ); // | MF_CHECKED
                AppendMenu( menu, MF_STRING, MENU_SETTING, "Settings" );

                AppendMenu( menu, MF_SEPARATOR, 0, NULL );

                AppendMenu( menu, MF_STRING, MENU_EXIT, "Exit" );
                }
                break;
            case ICON_MESSAGE:
                switch( lparam ) {
                    case WM_RBUTTONDOWN:
                        GetCursorPos( &curPoint ) ;
                        //SetForegroundWindow(hwnd);
                        //SetCursor( dfltCursor );
                        clicked = TrackPopupMenuEx( menu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, hwnd, NULL );
                        //SetCursor( dfltCursor );
                        SendMessage( hwnd, WM_NULL, 0, 0 );

                        switch ( clicked ) {
                            case MENU_EXIT:
                                sayString( "Frog-lies is quitting", "Quitting" );
                                doquit = 6000;
                                break;
                            case MENU_SS_WIND:
                                DoUpload( UPLOAD_WINDOW );
                                break;
                            case MENU_SS_SCRN:
                                DoUpload( UPLOAD_SCREEN );
                                break;
                            case MENU_SS_CROP:
                                clickDrag = WAITING;
                                ShowDragWindow( clickDrag, 0, 0, 0, 0 );
                                mouhook = SetWindowsHookEx( WH_MOUSE_LL, ( HOOKPROC )handlemouse, GetModuleHandle( NULL ), 0 );
                                MyCursor = dragCursor;
                                SetCursor( dragCursor );

                                //SetLayeredWindowAttributes( hwnd, RGB( 255, 255, 255 ), 0, LWA_ALPHA );
                                ShowWindow( hwnd, SW_SHOW );
                                break;
                            case MENU_SS_CLIP:
                                DoUpload( UPLOAD_CLIP );
                                break;
                            default:
                                return DefWindowProc( hwnd, msg, wparam, lparam );
                        };

                        //SendMessage(hwnd, WM_NULL, 0, 0);
                        break;
                        /*
                        case WM_LBUTTONDBLCLK:
                        MessageBox( NULL, "Tray icon double clicked!", "clicked", MB_OK );
                        break;
                        case WM_LBUTTONUP:
                        MessageBox( NULL, "Tray icon clicked!", "clicked", MB_OK );
                        break;
                        */
                    case 0x404:
                        if( doquit != 0 ) {
                            //PostMessage( hwnd, WM_CLOSE, 0, 0 );
                        }
                        break;
                    case 0x405:
                        if( whff.GetStatus() == 200 ) {
                            system_hidden( ( "start " + whff.GetLastUpload() ).c_str() );
                        }
                        break;
                    default:
                        return DefWindowProc( hwnd, msg, wparam, lparam );
                };
                break;
            case WM_CLOSE:
            case WM_DESTROY:
                running = false;
                break;
            case WM_SETCURSOR:
                SetCursor( MyCursor );
                break;
            default:
                return DefWindowProc( hwnd, msg, wparam, lparam );
        };
        return DefWindowProc( hwnd, msg, wparam, lparam );
    }

    void sayString( std::string str, std::string title ) {
        Log( "Start Say String" );
        if ( bubble ) {
            nid.cbSize = NOTIFYICONDATA_V2_SIZE;

            // Set Version 5 behaviour for balloon feature
            //nid.uVersion = NOTIFYICON_VERSION;
            //Shell_NotifyIcon(NIM_SETVERSION, &nid);

            nid.uFlags = NIF_INFO;
            strcpy( nid.szInfo, str.c_str() );
            strcpy( nid.szInfoTitle, title.c_str() );
            nid.uTimeout = 10000;
            nid.dwInfoFlags = NIF_INFO;
            Shell_NotifyIcon( NIM_MODIFY, &nid );
        }
        Log( "Finish Say String" );
    }

    void Upload( std::string type, const void* data, size_t datalen, std::string prefix ) {
        Log( "Start Upload A" );
        std::string str = Timestamp(prefix) + "." + type;

        //printf("%s", str.c_str());

        #ifndef DEBUGGING
        whff.Upload( str, data, datalen, GetMimeFromExt( type ), UploadPassword );
        #endif

        if ( copyLoc.length() > 0 ) {    //empty quotes to not copy...
            Log( "Copying..." );
            mkdir( copyLoc.c_str() );

            char str2[1000];
            snprintf( str2, 999, "%s%s", copyLoc.c_str(), str.c_str() );
            printf( "\n\nSAVING AS: '%s'\n\n", str2 );

            FILE* f = fopen( str2, "wb" );
            Log( "Saving..." );

            if ( !f ) { printf( "WARNING: Copy could not be saved because the directory could not be found!\n\n" ); }
            else { fwrite( data, datalen, 1, f ); }

            fclose( f );
            Log( "Saved." );
            //system("pause");
        }


        Log( "Setting Clipboard" );
        #ifndef DEBUGGING
        if( appendExtension ){
            SetClipboard( whff.GetLastUpload() + "." + type );
        }
        else{
            SetClipboard( whff.GetLastUpload() );
        }
        #endif

        sayString( Timestamp(prefix) + "." + type + " has been uploaded...", "Screenshot Taken" );
        Log( "Playing Sound" );
        PlaySound( MAKEINTRESOURCE( 3 ), GetModuleHandle( NULL ), SND_ASYNC | SND_RESOURCE );
        Log( "Finished Uploading B" );
    }
    void Upload( std::string fname ) {

        //printf("%s", str.c_str());
        Log( "Start Upload B" );
        whff.Upload( fname, UploadPassword );

        if ( copyLoc.length() > 0 ) {    //empty quotes to not copy...
            Log( "Copying..." );
            mkdir( copyLoc.c_str() );

            char str2[1000];
            snprintf( str2, 999, "%s%s", copyLoc.c_str(), basename( fname ).c_str() );
            printf( "\n\nSAVING AS: '%s'\n\n", str2 );
            Log( "Saving..." );
            CopyFile( fname.c_str(), str2, 1 );

            /*FILE* f = fopen(str2, "wb");

            if (!f){ printf("WARNING: Copy could not be saved because the directory could not be found!\n\n"); }
            else{ fwrite(data, datalen, 1, f); }

            fclose(f);*/
            //system("pause");
        }
        Log( "Setting Clipboard" );
        size_t dotPos = fname.find_last_of('.');
        if( appendExtension && dotPos != std::string::npos ){

            SetClipboard( whff.GetLastUpload() + fname.substr( dotPos ) );
        }
        else{
            SetClipboard( whff.GetLastUpload() );
        }
        sayString( fname + " has been uploaded...", "Screenshot Taken" );
        Log( "Playing Sound" );
        PlaySound( MAKEINTRESOURCE( 3 ), GetModuleHandle( NULL ), SND_ASYNC | SND_RESOURCE );
        Log( "Finished Uploading B" );
    }

    void CheckKeys() {
        if( ShortcutDesk.IsHit() ) {
            Log( "Uploading Screen" );
            DoUpload( UPLOAD_SCREEN );
        }

        if ( ShortcutWin.IsHit() ) {
            Log( "Uploading Window" );
            DoUpload( UPLOAD_WINDOW );
        }

        if ( ShortcutCrop.IsHit() ) {
            Log( "Start Crop" );
            startCrop();
        }
        if ( ShortcutStop.IsHit() && DRAGGING ) {
            Log( "End Crop" );
            endCrop();
        }
        if ( ShortcutClip.IsHit() ) {
            Log( "Uploading Clipboard" );
            DoUpload( UPLOAD_CLIP );
        }
        if ( ShortcutQuit.IsHit() ) {
            Log( "Quitting" );
            if( doquit > 0 ){
                PostMessage( hwnd, WM_CLOSE, 0, 0 );
                Log( "Hard Quit" );

            }
            else {
                sayString( "Frog-lies is quitting", "Quitting" );
                doquit = 6000;
                Log( "Soft Quit" );
            }
        }
    }

    void GUIThread( void* ) {
        int tick = 0;
        while( running ) {
            if( doquit > 0 ) {
                doquit -= 2000;
                if( doquit <= 0 ) {
                    PostMessage( hwnd, WM_CLOSE, 0, 0 );
                }
            }

            ++tick;

            Sleep( 1000 );
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconA;
            Shell_NotifyIcon( NIM_MODIFY, &nid );
            if( flashIcon ){
            ++tick;
            Sleep( 1000 );
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconB;
            Shell_NotifyIcon( NIM_MODIFY, &nid );
            }
            if( tick > 10 ){
                tick = 0;
                Log( "Heartbeat" );
            }
        }
    }

    static int SetShortcut( Shortcut* s, const char* value ) {
        if( s->IsValid() ) {
            s->Set( value );
        }
        return 1;
    }

    void ReadConf( std::string fname ) {
        size_t len;
        char* d = ( char* )read_file_to_buffer( fname, len );
        if( d == 0 ) {
            return;
        }
        printf( "Using non-standard configuration\n" );
        d = ( char* )realloc( d, len + 1 );
        d[len] = 0;
        std::string buff = d;
        free( d );

        Lua L( buff.c_str(), "Configuration", 0 );

        //L.RegisterFunction( SetShortcut, "shortcut" );

        L.Set( "TRUE", 1 );
        /*L.Set( "SHORTCUT_WIN",  &ShortcutWin );
        L.Set( "SHORTCUT_DESK", &ShortcutDesk );
        L.Set( "SHORTCUT_CROP", &ShortcutCrop );
        L.Set( "SHORTCUT_CLIP", &ShortcutClip );
        L.Set( "SHORTCUT_QUIT", &ShortcutQuit );
        L.Set( "SHORTCUT_STOP", &ShortcutStop );*/
        L.Set( "true", 1 );
        L.Set( "false", 0 );

        L.Run();

        Lua::Value T = L.Get<Lua::Value>("Shortcuts");
        ShortcutWin.Set( T["Window"].GetString() );
        ShortcutDesk.Set( T["Desktop"].GetString() );
        ShortcutCrop.Set( T["Crop"].GetString() );
        ShortcutClip.Set( T["Clipboard"].GetString() );
        ShortcutQuit.Set( T["Quit"].GetString() );
        ShortcutStop.Set( T["Stop"].GetString() );

        bubble = L.Get<int>( "bubble" );

        olddrag = L.Get<int>( "dragtype" );
        loglevel = L.Get<int>( "LogLevel" );
        flashIcon = L.Get<int>("flashIcon");
        appendExtension = L.Get<int>("AutomaticallyAppendExtension");

        Lua::Value C = L.Get<Lua::Value>("SelectionColor");
        color_r = C[1].GetNumber();
        color_g = C[2].GetNumber();
        color_b = C[3].GetNumber();
        color_a = C[4].GetNumber();

        if( L.Get<char*>("copyLoc")[0]!=0 )
            copyLoc = L.Get<std::string>( "copyLoc" );
        if( L.Get<char*>("zipFormat")[0]!=0 )
            zipformat = L.Get<std::string>("zipFormat");
        if( L.Get<char*>("zipFormatExtension")[0]!=0 )
            zipformatext = L.Get<std::string>("zipFormatExtension");


        if( L.Get<char*>("OwnerName")[0]!=0 ){
            ownerName = L.Get<std::string>("OwnerName");
            whff.SetOwner( ownerName );
        }

        if( loglevel > 0 ){
            logfile = fopen( "log.txt", "a" );
            Log( "Starting up." );
        }



        //printf("%s", copyLoc);
    }
}

using namespace FrogLies;

std::vector<std::string> ParseCmdLine( const char* cmdline ) {
    char* buffer = ( char* ) malloc( strlen( cmdline ) + 1 );
    strcpy( buffer, cmdline );
    std::vector<std::string> args;
    int len = strlen( cmdline );
    int pos = 0;
    int isquote = 0;
    int isescaped = 0;
    int isreading = 0;
    for( int i = 0; i <= len; ++i ) {
        if( !isescaped ) {
            if( buffer[i] == '\\' ) {
                isescaped = 1;
                if( !isreading ) {
                    isreading = 1;
                    pos = i;
                }
                continue;
            } else if( buffer[i] == '\"' ) {
                if( isquote ) {
                    buffer[i] = 0;
                    args.push_back( buffer + pos );
                    buffer[i] = ' ';
                    pos = i + 1;
                    isquote = 0;
                    isreading = 0;
                    continue;
                } else {
                    if( isreading ) {
                        buffer[i] = 0;
                        args.push_back( buffer + pos );
                        buffer[i] = ' ';
                    }
                    pos = i + 1;
                    isquote = 1;
                    isreading = 0;
                    continue;
                }
            } else if( isspace( buffer[i] ) || buffer[i] == 0 ) {
                if( !isquote && isreading ) {
                    buffer[i] = 0;
                    args.push_back( buffer + pos );
                    buffer[i] = ' ';
                    isreading = 0;
                }
            } else {
                if( !isreading ) {
                    isreading = 1;
                    pos = i;
                }
            }
        } else {
            int replacement = ' ';
            int newpos = 1;
            switch( buffer[i] ) {
                case 'r': replacement = '\r'; break;
                case 'n': replacement = '\n'; break;
                case 't': replacement = '\t'; break;
                case '\\': replacement = '\\'; break;
                case 'o': sscanf( buffer + i, "o%o%n", &replacement, &newpos ); break;
                case 'x': sscanf( buffer + i, "x%x%n", &replacement, &newpos ); break;
                case ' ': replacement = ' '; break;
            }

            for( int j = 0; newpos + i + j - 1 < len; ++j ) {
                buffer[i + j] = buffer[newpos + i + j];
            }
            len -= newpos;
            buffer[i - 1] = replacement;
            isescaped = 0;
            i -= 1;
            continue;
        }
    }
    if( isreading ) {
        args.push_back( buffer + pos );
    }
    free( buffer );
    return args;
}
#include <cctype>

std::string UrlEncode(std::string input){
	std::string ret = "";
	char output = 0;
	int readingCode = 0;
	for (char c2 : input){
		int c = c2;

		bool escape = false;
		if (c < '0' || c > 'z'){
			escape = true;
		}
		else{
			if (!isalnum(c)){
				escape = true;
			}
		}
		if (escape){
			if (c == ' '){
				ret += '+';
			}
			else{
				char buffer[100];
                snprintf(buffer,100, "%%%02x", (unsigned char)c);
                ret += buffer;
			}
		}
		else{
			ret += c;
		}
	}
	return ret;
}

int HandleArgs( const char* cmdline ) {
    std::vector< std::string > args = ParseCmdLine( cmdline );
    if ( args.size() == 0 ) {
        return 0;
    }
    HWND parent = FindWindowEx( NULL, NULL, "FrogLies", NULL );
    if ( args[0] == "--window" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59090, 0, 0 );
        } else {
            DoUpload( UPLOAD_WINDOW );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--screen" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59091, 0, 0 );
        } else {
            DoUpload( UPLOAD_SCREEN );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--clip" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59092, 0, 0 );
        } else {
            DoUpload( UPLOAD_CLIP );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--file" ) {
        SetParams(args[2], args[2]);
        Upload(  args[1] );
        std::string clip = whff.GetLastUpload();
        clip += "?password=" + UrlEncode( args[2]);
        printf("\n%s\n", clip.c_str());
        SetClipboard( clip );
        return 1;
    }
    if ( args[0] == "--crop" ) {
        int passargs[4];
        if ( args.size() == 5 ) {
            passargs[0] = strtol( args[1].c_str(), NULL, 10 );
            passargs[1] = strtol( args[2].c_str(), NULL, 10 );
            passargs[2] = strtol( args[3].c_str(), NULL, 10 );
            passargs[3] = strtol( args[4].c_str(), NULL, 10 );
            if ( passargs[0] == 0 || passargs[1] == 0 || passargs[2] == 0 || passargs[3] == 0 ) {
                sayString( "Non-int CLI for crop.", "Error" );
            }

            if( parent != NULL ) {
                PostMessage( parent, 59093, passargs[0], passargs[1] );
                PostMessage( parent, 59094, passargs[2], passargs[3] );
            } else {
                Bitmap mb = GetWindow( GetDesktopWindow() );
                mb.Crop( passargs[0], passargs[1], passargs[2], passargs[3] );
                void* data = mb.ReadPNG();
                if( data != 0 ) {
                    Upload( "png", data, mb.PNGLen() );
                    SetClipboard( whff.GetLastUpload() );
                }
            }
            return 1;
        } else {
            printf( "Invalid number of arguments for crop.\n" );
        }
        return 1;
    }
    return 1;
}

int WINAPI WinMain( HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow ) {

    //printf("%s\n", cmdline );

    ReadConf( "conf.cfg" );
    if( HandleArgs( cmdline ) ) {
        WaitForUploads();
        return 0;
    }

    dragCursor = LoadCursor( NULL, IDC_CROSS );
    dfltCursor = GetCursor();
    MyCursor = dfltCursor;

    HWND        fgwindow = GetForegroundWindow(); /* Current foreground window */
    MSG         msg;
    WNDCLASSEX  windowclass;
    HINSTANCE   modulehandle;

    hFont = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);
    color = RGB(48,98,48);

    windowclass.hInstance = thisinstance;
    windowclass.lpszClassName = CLASSNAME;
    windowclass.lpfnWndProc = windowprocedure;
    windowclass.style = (CS_DBLCLKS&~WS_VISIBLE) | 0x00020000;
    windowclass.cbSize = sizeof( WNDCLASSEX );
    windowclass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hCursor  = NULL;
    windowclass.lpszMenuName = NULL;
    windowclass.cbClsExtra = 0;
    windowclass.cbWndExtra = 0;
    //if (olddrag){
        windowclass.hbrBackground =  CreateSolidBrush( RGB( color_r, color_g, color_b ) );
    //}
    //else{
    //    windowclass.hbrBackground =  CreateSolidBrush( RGB(255,255,255) );
    //}
    windowclass.hCursor  = dragCursor;
    if ( !( RegisterClassEx( &windowclass ) ) ) {
        return 1;
    }

    windowclass.lpszClassName = CLASSNAMETEXT;
    windowclass.lpfnWndProc = textwindowprocedure;
    //windowclass.hbrBackground =  CreateSolidBrush( RGB(0,255,0) );
    if ( !( RegisterClassEx( &windowclass ) ) ) {
        return 2;
    }

    windowclass.lpszClassName = CLASSNAMEMOUS;
    windowclass.style = CS_DBLCLKS&~WS_VISIBLE;
    windowclass.lpfnWndProc = mouswindowprocedure;
    //windowclass.hbrBackground =  CreateSolidBrush( RGB(255,0,0) );
    if ( !( RegisterClassEx( &windowclass ) ) ) {
        return 3;
    }

    hwnd = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW, CLASSNAME, WINDOWTITLE, WS_POPUP,
                           CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, HWND_DESKTOP, NULL,
                           thisinstance, NULL );

    if ( !( hwnd ) ) {
        return 4;
    }

    hwndmous = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW, CLASSNAMEMOUS, WINDOWTITLE, WS_POPUP,
                           CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, HWND_DESKTOP, NULL,
                           thisinstance, NULL );

    if ( !( hwndmous ) ) {
        return 5;
    }

    hwndtext = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW, CLASSNAMETEXT, WINDOWTITLE, WS_POPUP,
                           CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, HWND_DESKTOP, NULL,
                           thisinstance, NULL );

    if ( !( hwndtext ) ) {
        return 6;
    }

    ShowWindow( hwnd, SW_HIDE );
    ShowWindow( hwndmous, SW_HIDE );
    ShowWindow( hwndtext, SW_HIDE );

    UpdateWindow( hwnd );
    UpdateWindow( hwndmous );
    UpdateWindow( hwndtext );

    SetForegroundWindow( fgwindow );

//    SetWindowPos( hwnd, HWND_TOP, 0, 0, 0, 0, 0 );
//    SetWindowPos( hwndtext, HWND_TOP, 0, 0, 0, 0, 0 );
//    SetWindowPos( hwndmous, HWND_TOPMOST, 0, 0, 0, 0, 0 );

    SetWindowPos( hwnd, HWND_TOP, GetSystemMetrics( SM_CXVIRTUALSCREEN ), 0, 20, 20, 0 );
    SetWindowPos( hwndtext, HWND_TOP, GetSystemMetrics( SM_CXVIRTUALSCREEN ), 0, 20, 20, 0 );
    SetWindowPos( hwndmous, HWND_TOPMOST, GetSystemMetrics( SM_CXVIRTUALSCREEN ), 0, 20, 20, 0 );

    modulehandle = GetModuleHandle( NULL );

    //#define BEGIN_IN_DRAGMODE
    #ifdef BEGIN_IN_DRAGMODE
    startCrop();
    #endif

    IconA = LoadIcon( thisinstance, MAKEINTRESOURCE( 1 ) );
    IconB = LoadIcon( thisinstance, MAKEINTRESOURCE( 2 ) );


    nid.cbSize = sizeof( NOTIFYICONDATA );
    nid.uID = 100;
    nid.hWnd = hwnd;
    //nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = ICON_MESSAGE;
    nid.hIcon = IconB; //= LoadIcon(NULL, IDI_APPLICATION);
    snprintf( nid.szTip, 64, "Frog-lies" );
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon( NIM_ADD, &nid );
    kbdhook = SetWindowsHookEx( WH_KEYBOARD_LL, ( HOOKPROC )handlekeys, modulehandle, 0 );
    mouhook = NULL;

    running = true;

    sayString( "Frog-lies has started...", "Startup" );

    _beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

    while ( running ) {
        if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
            break;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    Shell_NotifyIcon( NIM_DELETE, &nid );

    Log( "Quitting...." );

    return 0;
}

#ifdef DEBUGMALLOCS
#undef malloc(a)
#undef free(a)
#undef realloc(a,b)

void* DMALLOC( size_t a, int line, const char* name ) {
    void *r = malloc( a );
    printf( "MALLOC %X (%d) on line %d of %s\n", r, a, line, name );
    return r;
}
void DFREE( void* a, int line, const char* name ) {
    free( a );
    printf( "FREE %X on line %d of %s\n", a, line, name );
}
void* DREALLOC( void*a, size_t b, int line, const char*name ) {
    void* ret = realloc( a, b );
    printf( "REALLOC from %X to %X (%d) on line %d of %s\n", a, ret, b, line, name );
    return ret;
}
#endif

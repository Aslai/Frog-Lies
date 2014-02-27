#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <string>
#include <map>

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
#include <vector>
#include <ctime>
#include <cstring>
#include <deque>

#include "files.h"
#include "whff.h"
#include "bitmap.h"
#include <map>
#include "luawrap.h"
#include "froglies.h"
#include "mutex.h"


#define CLASSNAME   "winss"
#define WINDOWTITLE "FrogLies"
#define ICON_MESSAGE (WM_USER + 1)

#define NOTNOW 0
#define WAITING 1
#define DRAGGING 2

namespace FrogLies {
    enum {
        UPLOAD_NONE = -1,
        UPLOAD_WINDOW,
        UPLOAD_SCREEN,
        UPLOAD_CROP,
        UPLOAD_CLIP
    };

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

    int doquit = 0;

    bool bubble;

    WHFF whff( "" );

    struct _anupload {
        int t;
        struct {int x, y, w, h;} d;
    };
    std::deque<_anupload> todouploads;

    Mutex lock;


    void CheckKeys();

    int uploadthreadrunning = 0;

    void Upload( std::string type, const void* data, size_t datalen );
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
                        Upload( "png", data, mb.PNGLen() );
                    } break;

                case UPLOAD_SCREEN: {
                        Bitmap mb = GetWindow( GetDesktopWindow() );
                        void* data = mb.ReadPNG();
                        Upload( "png", data, mb.PNGLen() );
                    } break;

                case UPLOAD_CROP: {
                        Bitmap mb = GetWindow( GetDesktopWindow() );
                        mb.Crop( a.d.x, a.d.y, a.d.w, a.d.h );
                        void* data = mb.ReadPNG();
                        if( data != 0 ) {
                            Upload( "png", data, mb.PNGLen() );
                        }
                    } break;

                case UPLOAD_CLIP: {
                        if ( !OpenClipboard( NULL ) ) {
                            continue;
                        }
                        HANDLE hClipboardData = GetClipboardData( CF_TEXT );
                        char *pchData = ( char* )GlobalLock( hClipboardData );
                        void* data = ( void* )pchData;
                        Upload( "txt", data, strlen( pchData ) );
                        GlobalUnlock( hClipboardData );
                        CloseClipboard();
                    } break;

                default:
                    break;

            }
        }
        lock.Lock();
        uploadthreadrunning = 0;
        lock.Unlock();
    }

    void DoUpload( int type, int x = -1, int y = -1, int w = -1, int h = -1 ) {
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
        lock.Lock();
        todouploads.push_back( a );
        _beginthread( __uploadthread, 1000, 0 );
        lock.Unlock();
    }

    std::map<std::string, int> keyspressed;
    std::string Timestamp();
    void sayString( std::string message, std::string title );
    int ReadKey( std::string key ) {
        if( keyspressed[key] == 2 ) {
            keyspressed[key] = 1;
            return 2;
        }
        if( keyspressed[key] == 3 ) {
            keyspressed[key] = 0;
            return 2;
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

    std::string Timestamp() {
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char str[64];
        snprintf( str, 64, "ss at %s", asctime( timeinfo ) );
        int i = 0;
        while ( str[i] ) {
            i++;
            if ( str[i] == ' ' ) {
                str[i] = '_';
            }
        }
        std::string ToReturn( str );
        return ToReturn;
    }

    LRESULT CALLBACK handlemouse( int code, WPARAM wp, LPARAM lp ) {

        if ( clickDrag ) {
            if ( ( wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP || wp == WM_MOUSEMOVE ) ) {
                MSLLHOOKSTRUCT st_hook = *( ( MSLLHOOKSTRUCT* )lp );

                dragEnd = st_hook.pt;

                if ( clickDrag == WAITING ) {
                    coords = dragEnd;
                } else {
                    coords.x = dragEnd.x - dragStart.x;
                    coords.y = dragEnd.y - dragStart.y;
                }

                int x, y, w, h;
                x = dragStart.x < dragEnd.x ? dragStart.x : dragEnd.x;
                y = dragStart.y < dragEnd.y ? dragStart.y : dragEnd.y;
                w = dragStart.x - dragEnd.x;
                h = dragStart.y - dragEnd.y;
                if( w < 0 ) { w = -w; }
                if( h < 0 ) { h = -h; }

                if( clickDrag != DRAGGING ) {
                    SetLayeredWindowAttributes( hwnd, RGB( 255, 255, 255 ), 1, LWA_ALPHA );
                    SetWindowPos( hwnd, HWND_TOPMOST, dragEnd.x - 100, dragEnd.y - 100, 200, 200, 0 );
                } else {
                    SetWindowPos( hwnd, HWND_TOPMOST, x, y, w, h, 0 );
                    if( clickDrag != NOTNOW ) {
                        SetLayeredWindowAttributes( hwnd, RGB( 255, 255, 255 ), 100, LWA_ALPHA );
                    }
                }

                //printf("State: %i \t MPos: [%i, %i] \t Coord: [%i, %i]\n", clickDrag, dragEnd.x - GetSystemMetrics( SM_XVIRTUALSCREEN ), dragEnd.y - GetSystemMetrics( SM_YVIRTUALSCREEN ), coords.x, coords.y);

                //printf("HANDMOUS- wp: 0x%X \t md: 0x%X \t fl: 0x%X\n", wp, st_hook.mouseData, st_hook.flags);

                if ( wp == WM_LBUTTONDOWN ) {
                    dragStart = dragEnd;
                    clickDrag = DRAGGING;
                    printf( "DOWN!\n" );
                }
                if ( wp == WM_LBUTTONUP ) {
                    SetLayeredWindowAttributes( hwnd, RGB( 255, 255, 255 ), 0, LWA_ALPHA );
                    printf( "\n\n\n\n\n%d\n\n\n\n\n\n", GetSystemMetrics( SM_XVIRTUALSCREEN ) );
                    DoUpload( UPLOAD_CROP, dragStart.x - GetSystemMetrics( SM_XVIRTUALSCREEN ), dragStart.y - GetSystemMetrics( SM_YVIRTUALSCREEN ), coords.x, coords.y );
                    /*
                    Bitmap mb = GetWindow( GetDesktopWindow() );
                    mb.Crop( dragStart.x, dragStart.y, coords.x, coords.y );
                    void* data = mb.ReadPNG();
                    if( data != 0 ) {
                            Upload( "png", data, mb.PNGLen() );
                        }
                        */

                    clickDrag = NOTNOW;
                    printf( "UP!\n" );
                    UnhookWindowsHookEx( mouhook );
                    mouhook = NULL;
                    MyCursor = dfltCursor;
                    SetCursor( dfltCursor );
                    ShowWindow( hwnd, SW_HIDE );
                }
            }
        } else {
            UnhookWindowsHookEx( mouhook );
            mouhook = NULL;
            MyCursor = dfltCursor;
            SetCursor( dfltCursor );
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
            msg += ( st_hook.scanCode << 16 );
            msg += ( st_hook.flags << 24 );
            GetKeyNameText( msg, tmp, 0xFF );
            str = std::string( tmp );
            if( keyspressed[str] == 2 ) {
                keyspressed[str] = 3;
            } else {
                keyspressed[str] = 0;
            }
            //fprintf(out, "%s\n", str.c_str());

            printf( "%s up\n", str.c_str() );
        } else if ( code == HC_ACTION && ( wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN ) ) {
            char tmp[0xFF] = {0};
            std::string str;
            DWORD msg = 1;
            KBDLLHOOKSTRUCT st_hook = *( ( KBDLLHOOKSTRUCT* )lp );
            msg += ( st_hook.scanCode << 16 );
            msg += ( st_hook.flags << 24 );
            GetKeyNameText( msg, tmp, 0xFF );
            str = std::string( tmp );
            if( keyspressed[str] == 0 ) {
                keyspressed[str] = 2;
            } else {
                keyspressed[str] = 1;
            }

            printf( "%s down\n", str.c_str() );
        }
        CheckKeys();

        return CallNextHookEx( kbdhook, code, wp, lp );
    }

    LRESULT CALLBACK windowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
        //printf("FISH!");

        //printf( "WINDPROC- msg: 0x%X \t wp: 0x%X \t lp: 0x%X\n", msg, wparam, lparam );

        switch ( msg ) {
            case WM_CREATE:
                //printf("FISH!");
                break;
            case ICON_MESSAGE:
                switch( lparam ) {
                    case WM_LBUTTONDBLCLK:
                        MessageBox( NULL, "Tray icon double clicked!", "clicked", MB_OK );
                        break;
                    case WM_LBUTTONUP:
                        MessageBox( NULL, "Tray icon clicked!", "clicked", MB_OK );
                        break;
                    case 0x404:
                        if( doquit != 0 ) {
                            //PostMessage( hwnd, WM_CLOSE, 0, 0 );
                        }
                        break;
                    case 0x405:
                        if( whff.GetStatus() == 200 ) {
                            system( ( "start " + whff.GetLastUpload() ).c_str() );
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
        return 0;
    }

    void sayString( std::string str, std::string title ) {
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
    }

    void Upload( std::string type, const void* data, size_t datalen ) {
        std::string str = Timestamp() + "." + type;
        whff.Upload( str, data, datalen, GetMimeFromExt( type ) );
        SetClipboard( whff.GetLastUpload() );
        sayString( Timestamp() + type + " has been uploaded...", "Screenshot Taken" );
        PlaySound( MAKEINTRESOURCE( 3 ), GetModuleHandle( NULL ), SND_ASYNC | SND_RESOURCE );
    }

    void CheckKeys() {
        if( ShortcutDesk.IsHit() ) {
            DoUpload( UPLOAD_SCREEN );
            /*Bitmap mb = GetWindow( GetDesktopWindow() );
            void* data = mb.ReadPNG();
            Upload( "png", data, mb.PNGLen() );*/
        }

        if ( ShortcutWin.IsHit() ) {
            DoUpload( UPLOAD_WINDOW );
            /*WHFF whff( "" );
            Bitmap mb = GetWindow( GetForegroundWindow() );
            void* data = mb.ReadPNG();
            Upload( "png", data, mb.PNGLen() );*/
        }

        if ( ShortcutCrop.IsHit() ) {
            printf( "hi\n" );
            clickDrag = WAITING;
            mouhook = SetWindowsHookEx( WH_MOUSE_LL, ( HOOKPROC )handlemouse, GetModuleHandle( NULL ), 0 );
            MyCursor = dragCursor;
            SetCursor( dragCursor );

            SetLayeredWindowAttributes( hwnd, RGB( 255, 255, 255 ), 0, LWA_ALPHA );
            ShowWindow( hwnd, SW_SHOW );
        }
        if ( ShortcutClip.IsHit() ) {
            DoUpload( UPLOAD_CLIP );

            /*WHFF whff( "" );
            if ( !OpenClipboard( NULL ) ) {
                    return;
                }
            HANDLE hClipboardData = GetClipboardData( CF_TEXT );
            char *pchData = ( char* )GlobalLock( hClipboardData );
            void* data = ( void* )pchData;
            printf( "%s\n", pchData );
            Upload( "txt", data, strlen( pchData ) );
            GlobalUnlock( hClipboardData );
            CloseClipboard();*/
        }
        if ( ShortcutQuit.IsHit() ) {
            sayString( "Frog-lies is quitting", "Quitting" );
            doquit = 6000;
        }
    }

    void GUIThread( void* ) {
        while( running ) {
            if( doquit > 0 ) {
                doquit -= 2000;
                if( doquit <= 0 ) {
                    PostMessage( hwnd, WM_CLOSE, 0, 0 );
                }
            }
            Sleep( 1000 );
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconA;
            Shell_NotifyIcon( NIM_MODIFY, &nid );
            Sleep( 1000 );
            nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            nid.hIcon = IconB;
            Shell_NotifyIcon( NIM_MODIFY, &nid );
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
        L.funcreg<int, Shortcut*, const char*, SetShortcut >( "shortcut" );
        L.set( "TRUE", 1 );
        L.set( "SHORTCUT_WIN",  &ShortcutWin );
        L.set( "SHORTCUT_DESK", &ShortcutDesk );
        L.set( "SHORTCUT_CROP", &ShortcutCrop );
        L.set( "SHORTCUT_CLIP", &ShortcutClip );
        L.set( "SHORTCUT_QUIT", &ShortcutQuit );
        L.set( "SHORTCUT_QUIT", &ShortcutQuit );
        L.set( "true", 1 );
        L.set( "false", 1 );


        L.run();

        bubble = L.get<int>( "bubble" );
    }
}

using namespace FrogLies;




int WINAPI WinMain( HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow ) {


    ReadConf( "conf.cfg" );


    HWND        fgwindow = GetForegroundWindow(); /* Current foreground window */
    MSG         msg;
    WNDCLASSEX  windowclass;
    HINSTANCE   modulehandle;

    windowclass.hInstance = thisinstance;
    windowclass.lpszClassName = CLASSNAME;
    windowclass.lpfnWndProc = windowprocedure;
    windowclass.style = CS_DBLCLKS&~WS_VISIBLE;
    windowclass.cbSize = sizeof( WNDCLASSEX );
    windowclass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hCursor  = NULL;
    windowclass.lpszMenuName = NULL;
    windowclass.cbClsExtra = 0;
    windowclass.cbWndExtra = 0;
    windowclass.hbrBackground =  CreateSolidBrush( RGB( 0, 0, 255 ) );
    //windowclass.style

    if ( !( RegisterClassEx( &windowclass ) ) ) {
        return 1;
    }

    hwnd = CreateWindowEx( WS_EX_LAYERED | WS_EX_TOOLWINDOW, CLASSNAME, WINDOWTITLE, WS_POPUP,
                           CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, HWND_DESKTOP, NULL,
                           thisinstance, NULL );

    if ( !( hwnd ) ) {
        return 1;
    }

    ShowWindow( hwnd, SW_HIDE );
    UpdateWindow( hwnd );
    SetForegroundWindow( fgwindow ); /* Give focus to the previous fg window */


    dragCursor = LoadCursor( NULL, IDC_CROSS );
    dfltCursor = GetCursor();
    MyCursor = dfltCursor;

    modulehandle = GetModuleHandle( NULL );

    //#define BEGIN_IN_DRAGMODE
#ifdef BEGIN_IN_DRAGMODE
    mouhook = SetWindowsHookEx( WH_MOUSE_LL, ( HOOKPROC )handlemouse, modulehandle, 0 );
    MyCursor = dragCursor;
    SetCursor( dragCursor );
    clickDrag = WAITING;
#endif

    IconA = LoadIcon( thisinstance, MAKEINTRESOURCE( 1 ) );
    IconB = LoadIcon( thisinstance, MAKEINTRESOURCE( 2 ) );


    nid.cbSize = sizeof( NOTIFYICONDATA );
    nid.uID = 100;
    nid.hWnd = hwnd;
    //nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = ICON_MESSAGE;
    nid.hIcon = IconB; //= LoadIcon(NULL, IDI_APPLICATION);
    snprintf( nid.szTip, 64, "Icon" );
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

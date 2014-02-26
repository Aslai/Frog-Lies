#include <string>
#include <windows.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <stdio.h>
#include "png.h"

//#include "shitty_system_tray.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <Shlwapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <process.h>

#include <string.h>

#include "mimetypes.h"

void CheckKeys();

class WHFF {
    static int HasInit;
    std::string Owner;
    std::string LastUpload;
    static size_t callback( char *ptr, size_t size, size_t nmemb, void *userdata);
public:
    WHFF( std::string owner );
    void SetOwner( std::string owner );
    int Upload( std::string name, const void* data, size_t datalen, std::string mimetype = "application/octet-stream", std::string password = "" );
    int Upload( std::string fname, std::string password = "" );
    std::string GetLastUpload();
};
int WHFF::HasInit = 0;

WHFF::WHFF( std::string owner ) {
    if( !HasInit ) {
        HasInit = 1;
        curl_global_init(CURL_GLOBAL_WIN32);
    }
    SetOwner( owner );
}

void WHFF::SetOwner( std::string owner ) {
    Owner = owner;
}

size_t WHFF::callback( char *ptr, size_t size, size_t nmemb, void *userdata ) {
    printf("HAH %d|%s|", size * nmemb, ptr);
    char* tmp = (char*) malloc( size*nmemb + 1 );
    memcpy( tmp, ptr, size*nmemb );
    tmp[size*nmemb] = 0;
    std::string value = tmp;
    if( value.substr(0, 5) != "Error" )
        value = "http://fiel.tk/?i=" + value;
    WHFF* self = (WHFF*) userdata;
    self->LastUpload = value;
    return size * nmemb;
}

std::string WHFF::GetLastUpload() {
    return LastUpload;
}

void* FillTemplate( unsigned int& bpos, const char** Template, ... ) {
    va_list args;
    va_start( args, Template );
    unsigned int bsize = 1000;
    char* buffer = (char*) malloc( bsize );
    bpos = 0;

    for( int i = 0; Template[i] != 0; i += 2 ) {
        switch( Template[i][0] ) {
        case 'R':
        case 'r': {
            unsigned int rawlen = strlen( Template[i+1] );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            bpos += sprintf( buffer + bpos, "%s", Template[i+1] );
        }
        break;
        case 'S':
        case 's': {
            char* arg = va_arg( args, char* );
            unsigned int rawlen = strlen( arg );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            bpos += sprintf( buffer + bpos, "%s", arg );
        }
        break;
        case 'D':
        case 'd': {
            void* arg = va_arg( args, void* );
            unsigned int rawlen = va_arg( args, unsigned int );
            while( bpos + rawlen >= bsize ) {
                bsize *= 2;
                buffer = (char*) realloc( buffer, bsize );
            }
            memcpy( buffer + bpos, arg, rawlen );
            bpos += rawlen;
        }
        break;
        }
    }
    return buffer;
}

int WHFF::Upload( std::string name, const void* data, size_t datalen, std::string mimetype, std::string password ) {
    for( unsigned int i = 0; i < name.length(); ++i ) {
        if( name[i] == '\"' || name[i] < ' ' )
            name[i] = ' ';
    }

    const char * posttemplate[] = {
        "raw",  "-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"file\"; filename=\"",
        "str",  "",
        "raw",  "\"\r\n",
        "raw",  "Content-Type: ",
        "str",  "",
        "raw",  "\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"password\"\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151\r\n",
        "raw",  "Content-Disposition: form-data; name=\"owner\"\r\n\r\n",
        "dat",  "",
        "raw",  "\r\n-----------------------------28251299466151--\r\n",
        0, 0
    };
    unsigned int length;
    void *postobject = FillTemplate(   length, posttemplate,
                                       name.c_str(),
                                       mimetype.c_str(),
                                       data, datalen,
                                       password.c_str(), password.length(),
                                       Owner.c_str(), Owner.length() );

    printf("%s\n", (char*) postobject );

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    const char *theUrl = "http://frogbox.es/whff/upload.php?raw";
    curl_easy_setopt(curl, CURLOPT_URL, theUrl );
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postobject);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, length);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WHFF::callback );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this );



    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data; boundary=---------------------------28251299466151" );
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free( postobject );
    return 1;
}

void* read_file_to_buffer( std::string fname, size_t& length ) {
    FILE* f = fopen( fname.c_str(), "rb" );
    if( f == 0 ) return 0;
    length = 1000;
    unsigned char* ret = (unsigned char*) malloc( length );
    size_t i = 0;

    while( true ) {
        size_t hasread = fread( ret + i, 1, length - i, f);
        i += hasread;
        if( feof( f ) ) {
            length = i;
            break;
        }
        length *= 2;
        ret = (unsigned char*) realloc( ret, length );
    }
    return ret;
}

std::string basename( std::string in ) {
    int pos = 0;
    for( int i = 0; in[i]!= 0; ++i ) {
        if( in[i] == '/' || in[i] == '\\' )
            pos = i+1;
    }
    std::string ret = in.c_str()+pos;
    return ret;
}

std::string extension( std::string in ) {
    int pos = 0;
    for( int i = 0; in[i]!= 0; ++i ) {
        if( in[i] == '.' )
            pos = i+1;
    }
    std::string ret = in.c_str()+pos;
    return ret;
}

int WHFF::Upload( std::string fname, std::string password ) {
    void* buffer;
    size_t len;
    buffer = read_file_to_buffer( fname, len );
    Upload( basename( fname ), buffer, len, GetMimeFromExt( extension( fname ) ), password );
    return 1;
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

class Bitmap {
    int Width, Height;
    unsigned char* Data;
    void* pngdata;
    size_t pnglen;
    size_t pngpos;
    int* references;
public:
    Bitmap() {
        Data = NULL;
        Width = Height = 0;
        references = new int;
        *references = 1;
        pngdata = 0;
        pnglen = pngpos = 0;
    }
    Bitmap( const Bitmap& other ) {
        Data = other.Data;
        Width = other.Width;
        Height = other.Height;
        references = other.references;
        (*references)++;
        pngdata = 0;
        pnglen = pngpos = 0;
    }

    ~Bitmap() {
        (*references)--;
        if( *references <= 0 ) {
            if( Data != NULL )
                free( Data );
            delete references;
        }
        if( pngdata != NULL )
            free( pngdata );
    }
    void Write( int width, int height, void* data ) {
        Data = (unsigned char*) malloc( width * height * 4 );
        memcpy( Data, data, width * height * 4 );
        Width = width;
        Height = height;
    }
    void* Read() {
        return Data;
    }
    int W() {
        return Width;
    }
    int H() {
        return Height;
    }

    void Crop( int x, int y, int w, int h ) {
        if( x < 0 ) {
            w += x;
            x = 0;
        }
        if( y < 0 ) {
            h += y;
            y = 0;
        }
        if( x >= Width ) {
            x = Width-1;
        }
        if( y >= Height ) {
            y = Height-1;
        }
        if( x + w > Width ) {
            w = Width - x;
        }
        if( y + h > Height ) {
            h = Height - y;
        }

        if( Data == 0 ) return;
        unsigned char* newdata = (unsigned char*) malloc( w * h * 4 );
        for( int i = 0; i < h; ++i ) {
            memcpy( newdata + i * w * 4, Data + (y+i)*Width*4 + x*4, w*4);
        }
        free( Data );
        Data = newdata;
        Width = w;
        Height = h;
    }
    static Bitmap *self;
    static void pngwrite(png_structp png_ptr,png_bytep data, png_uint_32 length) {
        //Bitmap* self = (Bitmap*) png_get_io_ptr(png_ptr);
        int changed = 0;
        while( self->pngpos + length > self->pnglen ) {
            self->pnglen *= 2;
            changed = 1;
        }
        if( changed ) {
            self->pngdata = realloc( self->pngdata, self->pnglen );
        }
        memcpy(self->pngdata+self->pngpos, data, length);
        self->pngpos += length;
    }


    static void pngflush(png_structp png_ptr) {

    }

    size_t PNGLen() {
        return pngpos;
    }

    void* ReadPNG() {
        self = this;
        if( pngdata )
            free( pngdata );
        pnglen = 1000;
        pngpos = 0;
        pngdata = malloc( pnglen );



        FILE *fp = fopen("t.png", "wb");
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        size_t x, y;
        png_uint_32 bytes_per_row;
        png_byte **row_pointers = NULL;
        int pixel_size = 4;

        if (fp == NULL) return 0;

        /* Initialize the write struct. */
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
            fclose(fp);
            return 0;
        }

        /* Initialize the info struct. */
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
            png_destroy_write_struct(&png_ptr, NULL);
            fclose(fp);
            return 0;
        }


        /* Set image attributes. */
        png_set_IHDR(png_ptr,
                     info_ptr,
                     Width,
                     Height,
                     8,
                     PNG_COLOR_TYPE_RGB_ALPHA,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
        /* Initialize rows of PNG. */
        bytes_per_row = Width * pixel_size;
        row_pointers = (png_byte**)png_malloc(png_ptr, Height * sizeof(png_byte *));
        for (y = 0; y < Height; ++y) {
            uint8_t *row = (uint8_t*)png_malloc(png_ptr, Width * sizeof(uint8_t) * pixel_size);
            row_pointers[y] = (png_byte *)row;
            for (x = 0; x < Width; ++x) {
                *row++ = Data[x*pixel_size+y*Width*pixel_size+2];
                *row++ = Data[x*pixel_size+y*Width*pixel_size+1];
                *row++ = Data[x*pixel_size+y*Width*pixel_size+0];

                //*row++ = Data[x*pixel_size+y*Width*pixel_size+3];
                *row++ = 0xFF;
            }
        }

        /* Actually write the image data. */
        png_set_write_fn (png_ptr, this, (png_rw_ptr)pngwrite, (png_flush_ptr)pngflush );
        //png_init_io(png_ptr, fp);
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);


        /* Cleanup. */
        for (y = 0; y < Height; y++) {
            png_free(png_ptr, row_pointers[y]);
        }
        png_free(png_ptr, row_pointers);

        /* Finish writing. */
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return pngdata;
    }
};
Bitmap *Bitmap::self;

Bitmap GetWindow(HWND h) {
    RECT rect;
    RECT rect2;

    Bitmap ret;
    if( !GetClientRect(h, &rect) )
        return ret;
    if( !GetWindowRect(h, &rect2) )
        return ret;



    int width = rect2.right - rect2.left;
    int height = rect2.bottom - rect2.top;
    if( width <= 0 || height <= 0 ) return ret;
    int x = (int)(rect2.left);// + ((int)(rect.right-rect.left) - (int)(rect2.right-rect2.left))/2;
    int y = (int)(rect2.top);// + ((int)(rect.bottom-rect.top) - (int)(rect2.bottom-rect2.top)) - x;
    printf("%d\t%d\n", x, y );
    printf("%d\t%d\n", rect.top, rect2.top);
    if( h == GetDesktopWindow() ){
        width = GetSystemMetrics (SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics (SM_CYVIRTUALSCREEN);
        x = GetSystemMetrics (SM_XVIRTUALSCREEN);
        y = GetSystemMetrics (SM_YVIRTUALSCREEN);
    }
    h = GetDesktopWindow();

    HDC hDC = GetDC(h);
    HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap =CreateCompatibleBitmap(hDC,
                            width, height);
    HGDIOBJ hOld = SelectObject(hCaptureDC,hCaptureBitmap);
    BitBlt(hCaptureDC,0,0,width,height,
           hDC,x,y,SRCCOPY|CAPTUREBLT);


    SelectObject(hCaptureDC, hOld);

    BITMAPINFOHEADER bmi = {0};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;


    BYTE* ScreenData = (BYTE*)malloc(4 * width * height );

    GetDIBits(hCaptureDC, hCaptureBitmap, 0, height, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    ret.Write( width, height, ScreenData );


    // SaveCapturedBitmap(hCaptureBitmap); //Place holder - Put your code
    //here to save the captured image to disk
    ReleaseDC(h,hDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);

    return ret;
}

#include <map>
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

HHOOK	kbdhook;

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


#define CLASSNAME	"winss"
#define WINDOWTITLE	"svchost"
#define ICON_MESSAGE (WM_USER + 1)

bool running;

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
HWND		hwnd;
void CheckKeys(){
	if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("2") >= 4) {
            printf("hi\n");
            WHFF whff("");
            Bitmap mb = GetWindow(GetDesktopWindow());
            void* data = mb.ReadPNG();
            whff.Upload( "Screencap.png", data, mb.PNGLen(), GetMimeFromExt("png"));
            SetClipboard( whff.GetLastUpload() );
        }

        if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("3") >= 4) {
                printf("hi\n");
            WHFF whff("");
            Bitmap mb = GetWindow(GetForegroundWindow());
            void* data = mb.ReadPNG();
            whff.Upload( "Screencap.png", data, mb.PNGLen(), GetMimeFromExt("png"));
            SetClipboard( whff.GetLastUpload() );
        }

        if (ReadKey("Ctrl") + ReadKey("Alt") + ReadKey("Q") >= 4) {
                printf("hi\n");
            PostMessage( hwnd, WM_CLOSE, 0, 0 );
        }
}

NOTIFYICONDATA nid;
HICON IconA;
HICON IconB;

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

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow){

	HWND		fgwindow = GetForegroundWindow(); /* Current foreground window */
	MSG		    msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

    //HINSTANCE thisinstance = (HINSTANCE)GetModuleHandle(NULL);

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

	hwnd = CreateWindowEx(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
                          thisinstance, NULL);

	if (!(hwnd)){ return 1; }

    ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */

    IconA = (HICON) LoadImage( thisinstance, "icona.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );
    IconB = (HICON) LoadImage( thisinstance, "iconb.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE );


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
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);



    running = true;

    _beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

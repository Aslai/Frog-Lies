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

#include "files.h"
#include "whff.h"
#include "bitmap.h"

namespace FrogLies{
    Bitmap::Bitmap() {
        Data = NULL;
        Width = Height = 0;
        references = new int;
        *references = 1;
        pngdata = 0;
        pnglen = pngpos = 0;
    }
    Bitmap::Bitmap( const Bitmap& other ) {
        Data = other.Data;
        Width = other.Width;
        Height = other.Height;
        references = other.references;
        (*references)++;
        pngdata = 0;
        pnglen = pngpos = 0;
    }

    Bitmap::~Bitmap() {
        (*references)--;
        if( *references <= 0 ) {
            if( Data != NULL )
                free( Data );
            delete references;
        }
        if( pngdata != NULL )
            free( pngdata );
    }
    void Bitmap::Write( int width, int height, void* data ) {
        Data = (unsigned char*) malloc( width * height * 4 );
        memcpy( Data, data, width * height * 4 );
        Width = width;
        Height = height;
    }
    void* Bitmap::Read() {
        return Data;
    }
    int Bitmap::W() {
        return Width;
    }
    int Bitmap::H() {
        return Height;
    }

    void Bitmap::Crop( int x, int y, int w, int h ) {
        if( x < 0 ) {
            w += x;
            x = 0;
        }
        if( y < 0 ) {
            h += y;
            y = 0;
        }
        if( x >= (int)Width ) {
            x = Width-1;
        }
        if( y >= (int)Height ) {
            y = Height-1;
        }
        if( x + w > (int)Width ) {
            w = Width - x;
        }
        if( y + h > (int)Height ) {
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

    void Bitmap::pngwrite(png_structp png_ptr,png_bytep data, png_uint_32 length) {
        //Bitmap* self = (Bitmap*) png_get_io_ptr(png_ptr);
        int changed = 0;
        while( self->pngpos + length > self->pnglen ) {
            self->pnglen *= 2;
            changed = 1;
        }
        if( changed ) {
            self->pngdata = realloc( self->pngdata, self->pnglen );
        }
        memcpy((unsigned char*)(self->pngdata)+self->pngpos, data, length);
        self->pngpos += length;
    }


    void Bitmap::pngflush(png_structp png_ptr) {

    }

    size_t Bitmap::PNGLen() {
        return pngpos;
    }

    void* Bitmap::ReadPNG() {
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
}

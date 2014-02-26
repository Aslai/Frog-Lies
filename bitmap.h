class Bitmap {
    int Width, Height;
    unsigned char* Data;
    void* pngdata;
    size_t pnglen;
    size_t pngpos;
    int* references;
public:
    Bitmap();
    Bitmap( const Bitmap& other );
    ~Bitmap();

    void Write( int width, int height, void* data );
    void* Read();

    int W();
    int H();

    void Crop( int x, int y, int w, int h );
    static Bitmap *self;
    static void pngwrite(png_structp png_ptr,png_bytep data, png_uint_32 length);
    static void pngflush(png_structp png_ptr);
    size_t PNGLen();
    void* ReadPNG();
};
Bitmap GetWindow(HWND h);

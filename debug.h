#ifdef DEBUGMALLOCS
#define malloc(a) DMALLOC(a,__LINE__,__FILE__)
void* DMALLOC(size_t,int, const char*);
#define free(a) DFREE(a,__LINE__,__FILE__)
void DFREE(void*,int,const char*);
#define realloc(a,b) DREALLOC(a,b,__LINE__,__FILE__)
void* DREALLOC(void*,size_t,int,const char*);
#endif

#ifndef WTYPES_H
#define WTYPES_H

// Dummy file added by me.
typedef void* HANDLE;


typedef struct HINSTANCE__ { long x; } MyStruct;
#define HINSTANCE MyStruct *

#define INVALID_HANDLE_VALUE NULL

#define INFINITE -1


#endif // WTYPES_H

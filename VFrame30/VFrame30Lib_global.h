#pragma once
#include <QtGlobal>

#ifdef VFRAME30LIB_LIBRARY
    #define VFRAME30LIBSHARED_EXPORT Q_DECL_EXPORT
#else
    #define VFRAME30LIBSHARED_EXPORT Q_DECL_IMPORT
#endif



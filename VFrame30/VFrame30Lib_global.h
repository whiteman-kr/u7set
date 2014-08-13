#ifndef VFRAME30LIB_GLOBAL_H
#define VFRAME30LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef VFRAME30LIB_LIBRARY
    #define VFRAME30LIBSHARED_EXPORT Q_DECL_EXPORT
#else
    #define VFRAME30LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VFRAME30LIB_GLOBAL_H

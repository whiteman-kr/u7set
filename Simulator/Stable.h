#ifndef STABLE_H
#define STABLE_H

#include <QtCore>


// For detecting memory leaks
//
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif


#endif // STABLE_H

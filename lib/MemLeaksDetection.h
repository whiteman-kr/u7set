#pragma once

//
//	Current memory leaks detection is working only in Debug builds under OS Windows
//
//	For turn on memory leaks detection:
//
//	1) call initMemoryLeaksDetection() at the beginning of main()
//	2) call dumpMemoryLeaks() before exit from main()
//	3) place next code snipped in your precompiled header (ex. Stable.h)
//
//	#if defined (Q_OS_WIN) && defined (Q_DEBUG)
//	#define _CRTDBG_MAP_ALLOC
//	#include <crtdbg.h>
//		#ifndef DBG_NEW
//			#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//			#define new DBG_NEW
//		#endif
//	#endif

void initMemoryLeaksDetection();	// this function should be called at the beginning of main()

void dumpMemoryLeaks();				// this function should be called before exit from main()


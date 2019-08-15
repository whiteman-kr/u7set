#pragma once

//
//	Current memory leaks detection is working only in Debug builds under OS Windows
//
//	For turn on memory leaks detection:
//
//	1) insert code snipped shown below in Stable.h
//	2) call initMemoryLeaksDetection() at the beginning of main()
//	3) call dumpMemoryLeaks() before exit from main()

//	This code snippet should be placed in Stable.h:
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


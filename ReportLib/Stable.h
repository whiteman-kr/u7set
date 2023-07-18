#pragma once

#include <cassert>
#include <cstdint>

//#include <algorithm>
//#include <atomic>
//#include <functional>
//#include <iostream>
//#include <limits>
//#include <list>
//#include <map>
//#include <memory>
//#include <vector>
//#include <set>
//#include <type_traits>
//#include <utility>

// Qt includes
//
#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 6011)
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4127)
	#pragma warning(disable : 6326)
	#pragma warning(disable : 28182)
	#pragma warning(disable : 6386)		// Static analyzer warning: qvariant.h:444: warning: C6386: Buffer overrun while writing to 'data':  the writable size is 'size' bytes, but '8' bytes might be written.: Lines: 431, 432, 433, 435, 436, 443, 444
	#pragma warning(disable : 6385)		// Static analyzer warning: qhash.h:367: warning: C6385: Reading invalid data from 'this->offsets':  the readable size is '128' bytes, but 'i' bytes may be read.: Lines: 366, 367
#endif

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QPageLayout>
#include <QMutex>
#include <QPdfWriter>
#include <QPainter>
#include <QTextCursor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJSValue>
#include <QBuffer>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#define CLIENT_LIB_DOMAIN


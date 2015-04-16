/****************************************************************************
*
*  Copyright (C) 2002-2013 Helta Kft. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  Created: 18 Feb 2013 (nszabo)
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORT_GLOBAL_H
#define NCREPORT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NCREPORT_DIRECT_INTEGRATION)
#  define NCREPORTSHARED_EXPORT
#else
#if defined(NCREPORT_LIBRARY)
#  define NCREPORTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define NCREPORTSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif // NCREPORT_GLOBAL_H

/****************************************************************************
*
*  Copyright (C) 2002-2007 Norbert Szabo /Helta Kft. All rights reserved.
*  nszabo@helta.hu
*  www.helta.hu/ncreport
*
*  This file is part of the NCReport reporting software
*
*  This file may be used under the terms of the GNU General Public
*  License version 2.0 as published by the Free Software Foundation
*  and appearing in the file LICENSE.GPL included in the packaging of
*  this file.  Please review the following information to ensure GNU
*  General Public Licensing requirements will be met:
*  http://www.opensource.org/licenses/gpl-license.php
*
*  If you are unsure which license is appropriate for your use, please
*  review the following information:
*  http://www.helta.hu/ncreport/price.html or contact me at
*  nszabo@helta.hu
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef INFO_H
#define INFO_H

#define VER_FILEVERSION                     2,14,0,1
#define VER_FILEVERSION_STR                 "2.14.0\0"

#define VER_PRODUCTVERSION                  2,14,0,1
#define VER_PRODUCTVERSION_STR              "2.14.0.1\0"

#define VER_COMPANYNAME_STR                 "Helta Ltd. NociSoft Software Solutions"
#define VER_FILEDESCRIPTION_STR             "NCReport Designer"
#define VER_INTERNALNAME_STR                "NCReport Designer"
#define VER_LEGALCOPYRIGHT_STR              "Copyright (c) 2006-2015 Helta Ltd. NociSoft"
#define VER_LEGALTRADEMARKS1_STR            "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR            VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR            "NCReportDesigner.exe"
#define VER_PRODUCTNAME_STR                 "NCReportDesigner"
#define VER_COMPANYDOMAIN_STR               "nocisoft.com"

#define NCREPORTAPP_VERSION                 VER_FILEVERSION_STR
#define NCREPORTAPP_ORG                     "NociSoft"
#define NCREPORTAPP_NAME                    "NCReport"
#define NCREPORTAPP_LONGNAME                "NCReport Report Generator"
#ifdef NCREPORT_EVAL
    #define NCREPORTAPP_RELEASENAME         "Eval version"
#else
    #define NCREPORTAPP_RELEASENAME         ""
#endif
#define NCREPORTAPP_RELEASEDATE             "2015-01-15"
#define NCREPORTAPP_COPYRIGHT               VER_LEGALCOPYRIGHT_STR
#define NCREPORTAPP_WEB                     "http://www.nocisoft.com"

#define NCREPORTAPP_DEFAULT_IMAGEPATH       "images/"
#define NCREPORTAPP_DIALOG_EXTENSIONS       "NCReport (*.xml *.ncr)"




#endif // INFO_H

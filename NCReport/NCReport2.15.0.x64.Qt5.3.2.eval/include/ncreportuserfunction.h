/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Ltd. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*  Created: 2014.03.01. (nocisoft)
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  office@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORTUSERFUNCTION_H
#define NCREPORTUSERFUNCTION_H

#include "ncreport_global.h"

class NCReportOutput;
class NCReportItem;
class NCReportEvaluator;

class NCREPORTSHARED_EXPORT NCReportUserFunction
{
public:
    NCReportUserFunction();
    virtual ~NCReportUserFunction();

    virtual void updateValue( NCReportItem* item, NCReportOutput* output, NCReportEvaluator* evaluator) = 0;

};

#endif // NCREPORTUSERFUNCTION_H

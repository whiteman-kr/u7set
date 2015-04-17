/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Ltd. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*  Created: 2014.05.05. (nocisoft)
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

#ifndef NCREPORTVARIABLEMODIFIER_H
#define NCREPORTVARIABLEMODIFIER_H

#include <QString>

class NCReportEvaluator;

class NCReportVariableModifier
{
public:
    NCReportVariableModifier();

    enum ModifierFunction { PlusEqual=0x0, MinusEqual=0x1, DivideEqual=0x2, MultiplyEqual=0x3, Equal=0x4, Invalid=0x5};

    bool modifyValue(const QString &expression, NCReportEvaluator *evaluator);

private:
    bool evaluate(const QString &line, NCReportEvaluator *evaluator);

};

#endif // NCREPORTVARIABLEMODIFIER_H

/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport Report Generator System
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  norbert@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORTTEXTOUTPUT_H
#define NCREPORTTEXTOUTPUT_H

#include "ncreportoutput.h"

#include <QHash>

QT_BEGIN_NAMESPACE
class QFile;
class QTextStream;
QT_END_NAMESPACE

/*!
  Text file output class. Makes possible to run report to TXT/CSV/HTML/XML or any text file.
  */
class NCREPORTSHARED_EXPORT NCReportTextOutput : public NCReportOutput
{
	Q_OBJECT
public:
	NCReportTextOutput(QObject *parent = 0);
	~NCReportTextOutput();

	bool setup();
	void cleanup();
	void begin();
	void end();
	void newPage();
	
	virtual bool renderSection(NCReportSection* section, NCReportEvaluator* evaluator);

	void setTemplate(const QString& tpl);
	void setTemplateFile(const QString& fileName);

private:
	QTextStream *m_ts;
	QFile* m_file;
	bool m_insideSection;
	QString m_template;
	QString m_templateFile;

	QHash<QString,QString> m_textSections;
	QString m_currentTextSection;
	QString m_currentID;

	bool processTemplateLine(const QString &line);
};

#endif // NCREPORTTEXTOUTPUT_H

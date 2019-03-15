#ifndef LexerXML_H
#define LexerXML_H

#include "../../QScintilla/Qt4Qt5/Qsci/qscilexerxml.h"

class LexerXML : public QsciLexerXML
{
public:
	QFont defaultFont(int style) const;

};

#endif

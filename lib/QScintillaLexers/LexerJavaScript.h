#ifndef LexerJavaScript_H
#define LexerJavaScript_H

#include "../../QScintilla/Qt4Qt5/Qsci/qscilexerjavascript.h"

class LexerJavaScript : public QsciLexerJavaScript
{
public:
	QFont defaultFont(int style) const;
	QColor defaultColor(int style) const;

};

#endif

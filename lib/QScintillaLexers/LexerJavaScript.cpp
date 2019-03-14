#include "LexerJavaScript.h"

QFont LexerJavaScript::defaultFont(int style) const
{
	return QsciLexer::defaultFont(style);
}

QColor LexerJavaScript::defaultColor(int style) const
{
	switch (style)
	{
	case Default:
		return QColor(0x80, 0x80, 0x80);

	case Comment:
	case CommentLine:
		return QColor(0x00, 0x7f, 0x00);

	case CommentDoc:
	case CommentLineDoc:
	case PreProcessorCommentLineDoc:
		return QColor(0x3f, 0x70, 0x3f);

	case Number:
		return QColor(0x00, 0x00, 0x7f);

	case Keyword:
		return QColor(0x7f, 0x7f, 0x00);

	case DoubleQuotedString:
	case SingleQuotedString:
	case RawString:
		return QColor(0x00, 0x7f, 0x00);

	case PreProcessor:
		return QColor(0x00, 0x00, 0x7f);

	case Operator:
		return QColor(0x00, 0x00, 0x00);

	case UnclosedString:
		return QColor(0x00, 0x00, 0x00);

	case VerbatimString:
	case TripleQuotedVerbatimString:
	case HashQuotedString:
		return QColor(0x00, 0x7f, 0x00);

	case Regex:
		return QColor(0x3f, 0x7f, 0x3f);

	case CommentDocKeyword:
		return QColor(0x30, 0x60, 0xa0);

	case CommentDocKeywordError:
		return QColor(0x80, 0x40, 0x20);

	case PreProcessorComment:
		return QColor(0x65, 0x99, 0x00);

	case InactiveDefault:
	case InactiveUUID:
	case InactiveCommentLineDoc:
	case InactiveKeywordSet2:
	case InactiveCommentDocKeyword:
	case InactiveCommentDocKeywordError:
	case InactivePreProcessorCommentLineDoc:
		return QColor(0xc0, 0xc0, 0xc0);

	case InactiveComment:
	case InactiveCommentLine:
	case InactiveNumber:
	case InactiveVerbatimString:
	case InactiveTripleQuotedVerbatimString:
	case InactiveHashQuotedString:
		return QColor(0x90, 0xb0, 0x90);

	case InactiveCommentDoc:
		return QColor(0xd0, 0xd0, 0xd0);

	case InactiveKeyword:
		return QColor(0x7f, 0x7f, 0x00);

	case InactiveDoubleQuotedString:
	case InactiveSingleQuotedString:
	case InactiveRawString:
		return QColor(0xb0, 0x90, 0xb0);

	case InactivePreProcessor:
		return QColor(0xb0, 0xb0, 0x90);

	case InactiveOperator:
		return QColor(0x80, 0x80, 0x80);

	case InactiveIdentifier:
	case InactiveGlobalClass:
		return QColor(0xb0, 0xb0, 0xb0);

	case InactiveUnclosedString:
		return QColor(0x00, 0x00, 0x00);

	case InactiveRegex:
		return QColor(0x7f, 0xaf, 0x7f);

	case InactivePreProcessorComment:
		return QColor(0xa0, 0xc0, 0x90);

	case UserLiteral:
		return QColor(0xc0, 0x60, 0x00);

	case InactiveUserLiteral:
		return QColor(0xd7, 0xa0, 0x90);

	case TaskMarker:
		return QColor(0xbe, 0x07, 0xff);

	case InactiveTaskMarker:
		return QColor(0xc3, 0xa1, 0xcf);
	}

	return QsciLexer::defaultColor(style);
}

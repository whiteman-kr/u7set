#include "CodeEditor.h"

#include <QApplication>
#include <QPainter>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QTimer>

//
// ------------------------------------------------------------------------------------------
//
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    //m_tabSymbol.fill(QChar::Space, 4);
    m_tabSymbol = "\t";

    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::onCursorPositionChanged);

	updateLineNumberAreaWidth();

	onCursorPositionChanged();

	setWordWrapMode(QTextOption::NoWrap);

    // Selection color
    //
    QPalette p = palette();
    p.setColor(QPalette::Highlight, QColor(0x0078d7));
    p.setColor(QPalette::HighlightedText, QColor(0xffffff));
    setPalette(p);

    setCaretWidth(2);
    setTabWidth(4);

    QFontMetrics fm(font());
    setLineNumberOffset(static_cast<int>(fm.horizontalAdvance(QChar::Space) * 0.75));

    installEventFilter(this);
}

QString CodeEditor::text() const
{
    return toPlainText();
}

void CodeEditor::setText(const QString& text)
{
    setPlainText(QString());

	if (m_highlighter != nullptr)
	{
		document()->blockSignals(true);
	}

	setPlainText(text);

	if (m_highlighter != nullptr)
	{
		document()->blockSignals(false);
	}

	document()->setModified(false);

    updateLineNumberAreaWidth();
}

bool CodeEditor::autoIdent() const
{
    return m_autoIndent;
}

void CodeEditor::setAutoIndent(bool autoIdent)
{
    m_autoIndent = autoIdent;
}

void CodeEditor::setCustomMenuActions(QList<QAction*> actions)
{
    m_customMenuActions = actions;
}

void CodeEditor::setFont(const QFont& f)
{
    QPlainTextEdit::setFont(f);

    QFontMetrics fm(f);
    setTabStopDistance(fm.horizontalAdvance(' ') * m_tabWidth);

    setLineNumberOffset(static_cast<int>(fm.horizontalAdvance(QChar::Space) * 0.75));

	updateLineNumberAreaWidth();
}

void CodeEditor::setHighlighter(Highlighter* highlighter)
{
	m_highlighter = highlighter;
}

bool CodeEditor::isModified() const
{
	return document()->isModified();
}

void CodeEditor::setModified(bool value)
{
	document()->setModified(value);
}

void CodeEditor::setCaretLineVisible(bool visible)
{
    m_caretLineVisible = visible;
}

void CodeEditor::setCaretLineBackgroundColor(QColor color)
{
    m_caretLineColor = color;
}

void CodeEditor::setCaretWidth(int w)
{
    setCursorWidth(w);
}

void CodeEditor::setTabWidth(int w)
{
    m_tabWidth = w;

    QFontMetrics fm(font());
    setTabStopDistance(fm.horizontalAdvance(' ') * m_tabWidth);
}

void CodeEditor::getCursorPosition(int* line, int* index) const
{
    if (line == nullptr || index == nullptr)
    {
        Q_ASSERT(line);
        Q_ASSERT(index);
        return;
    }

    *line = textCursor().blockNumber() + 1;
    *index= textCursor().columnNumber() + 1;
    return;
}

void CodeEditor::setCurrentLine(int line)
{
    if (line > lines())
    {
        return;
    }

    QTextCursor cursor(document()->findBlockByLineNumber(line - 1));
    setTextCursor(cursor);

    QTimer::singleShot(10, this, [this](){
        setFocus();
    });
}

int CodeEditor::lines() const
{
    return document()->lineCount();
}

bool CodeEditor::lineNumberAreaVisible() const
{
    return m_lineNumberAreaVisible;
}

void CodeEditor::setLineNumberAreaVisible(bool visible)
{
    m_lineNumberAreaVisible = visible;
}

int CodeEditor::lineNumberOffset() const
{
    return m_lineNumberOffset;
}

void CodeEditor::setLineNumberOffset(int offset)
{
	m_lineNumberOffset = offset;
}

int CodeEditor::getLineNumberAreaWidth()
{
    if (m_customLineNumberAreaWidth != -1)
    {
        return m_customLineNumberAreaWidth;
    }

    if (m_lineNumberAreaVisible == false)
    {
        return 0;
    }

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

	int space = static_cast<int>(fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits) + m_lineNumberOffset * 2;

	return space;
}

int CodeEditor::customLineNumberAreaWidth()
{
    return m_customLineNumberAreaWidth;
}

void CodeEditor::setCustomLineNumberAreaWidth(int width)
{
    m_customLineNumberAreaWidth = width;
}

QColor CodeEditor::lineNumberAreaBackgroundColor() const
{
    return m_lineNumberAreaBackgroundColor;
}

void CodeEditor::setLineNumberAreaBackgroundColor(const QColor& color)
{
    m_lineNumberAreaBackgroundColor = color;
}

QColor CodeEditor::lineNumberAreaForegroundColor() const
{
    return m_lineNumberAreaForegroundColor;
}

void CodeEditor::setLineNumberAreaForegroundColor(const QColor& color)
{
    m_lineNumberAreaForegroundColor = color;
}

bool CodeEditor::findFirst(const QString& text, bool caseSensitive, bool whole)
{
    moveCursor(QTextCursor::Start);

    m_findContext.text = text;
    m_findContext.caseSensitive = caseSensitive;
    m_findContext.wholeWord = whole;

    return findNext();
}

bool CodeEditor::findNext()
{
    QTextDocument::FindFlags f = {};
    if (m_findContext.caseSensitive == true)
    {
        f |= QTextDocument::FindCaseSensitively;
    }
    if (m_findContext.wholeWord == true)
    {
        f |= QTextDocument::FindWholeWords;
    }

    return find(m_findContext.text, f);
}

bool CodeEditor::hasSelectedText() const
{
    return textCursor().hasSelection();
}

QString CodeEditor::selectedText() const
{
    return textCursor().selectedText();
}


void CodeEditor::replace(const QString& text)
{
    QTextCursor cursor = textCursor();

    if (cursor.hasSelection() == false)
    {
        return;
    }

    cursor.removeSelectedText();

    // Save anchor and position before inserting text
    //
    int oldAnchor = cursor.anchor();
    int oldPosition = cursor.position();

    cursor.insertText(text);

    // Select text between new cursor position and old selection start
    //
    int newPosition = 0, newAnchor = 0;
    if(oldAnchor < oldPosition)
    {
        Q_ASSERT(false);
        newAnchor = oldAnchor;
        newPosition = cursor.position();
    }
    else
    {
        newAnchor = cursor.position();
        newPosition = oldPosition;
    }

    cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
    cursor.setPosition(newPosition, QTextCursor::KeepAnchor);

    setTextCursor(cursor);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), lineNumberAreaBackgroundColor());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(m_lineNumberAreaForegroundColor);
            painter.drawText(m_lineNumberOffset, top, m_lineNumberArea->width() - m_lineNumberOffset * 2, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool CodeEditor::processAutoIdent(QKeyEvent* e)
{
    QString documentContents = document()->toPlainText();

    if (documentContents.isEmpty() == true)
    {
        return false;
    }

    int indexToLeftOfCursor = textCursor().position() - 1;

    if (indexToLeftOfCursor < 0 || indexToLeftOfCursor >= documentContents.length())
    {
        return false;
    }

	bool hitEnterAfterOpeningBrace = documentContents.at(indexToLeftOfCursor) == '{';
	bool hitClosingBrace = e->text() == "}";

	// Find the beginning of the current line
	//
	qsizetype currentLineStartIndex = documentContents.lastIndexOf('\n', indexToLeftOfCursor);
	if (currentLineStartIndex == -1)
	{
		currentLineStartIndex = 0;    // the beginning of the text
	}
	else
	{
		currentLineStartIndex++;
	}

	if (hitClosingBrace == true)
	{
		qsizetype openBracePos = documentContents.indexOf('{', currentLineStartIndex);
		bool lineHasOpeningBrace = openBracePos != -1 && openBracePos < indexToLeftOfCursor;   // Needed to skip '{}' situation

		// Remove one tab symbol in current line
		//
		qsizetype lastTabSymbolStartIndex = documentContents.lastIndexOf(m_tabSymbol, indexToLeftOfCursor);
		if (lineHasOpeningBrace == false &&
			lastTabSymbolStartIndex != -1 &&
			currentLineStartIndex <= lastTabSymbolStartIndex)
		{
			// Set cursor before last tabSymbol

			qsizetype moveCount = indexToLeftOfCursor - lastTabSymbolStartIndex + 1 /*}*/;
			for (int i = 0; i < moveCount; i++)
			{
				moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
			}

			// Select tab symbol

			for (int i = 0; i < m_tabSymbol.length(); i++)
			{
				moveCursor(QTextCursor::Right, QTextCursor::KeepAnchor);
			}

			// Remove tab symbol

			textCursor().removeSelectedText();

			// Move cursor after '}'

			for (int i = 0; i < (moveCount - m_tabSymbol.length()); i++)
			{
				moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
			}
		}

		QPlainTextEdit::keyPressEvent(e);   // insert }
	}
	else
	{
		QPlainTextEdit::keyPressEvent(e);   // insert \n or {

		// Find a whitespace part of the string from the beginning of the current line
		//

		static QRegularExpression regexp("[\\S+\\n]");

		qsizetype currentTextStartIndex = documentContents.indexOf(regexp, currentLineStartIndex);

		if (currentTextStartIndex > currentLineStartIndex)
		{
			// Insert whitespace to the next line
			//
			QString currentLineStartWhiteSpace = documentContents.mid(currentLineStartIndex, currentTextStartIndex - currentLineStartIndex);

			currentLineStartWhiteSpace.remove('\n');
			currentLineStartWhiteSpace.remove('\r');

			insertPlainText(currentLineStartWhiteSpace);
		}

		if (hitEnterAfterOpeningBrace)
		{
			insertPlainText(m_tabSymbol);
		}
	}

    return true;
}

// Add, remove or toggle line prefix. Operation is specified by operationCode : -1 - remove, 1 - add, 0 - toggle
//
bool CodeEditor::processPrefix(const QString& prefix, int operationCode)
{
    QTextCursor c = textCursor();
    int position = c.position();

    bool clearSelection = false;
    if (c.hasSelection() == false)
    {
        // If no text is selected - select current line
        //
		clearSelection = true;
		c.select(QTextCursor::LineUnderCursor);
    }

    int selectionStart = c.selectionStart();
    int selectionEnd = c.selectionEnd();

    // Check if selection starts from the beginning of the line. If not, extend selection to the beginning
    //
    QTextBlock startBlock = document()->findBlock(selectionStart);
    int startBlockPosition = startBlock.position();
    if (startBlockPosition != selectionStart)
    {
        selectionStart = startBlockPosition;
        c.clearSelection();
		c.setPosition(selectionStart);
		c.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }

    // Split selection into lines and process its prefix
    //
    QString str = c.selection().toPlainText();
        
	QStringList l = str.split("\n", Qt::KeepEmptyParts);
    if (l.isEmpty() == true)
    {
        return false;
    }

    bool prefixState = false;
    
    switch (operationCode)
    {
    case 0:
        prefixState = l[0].startsWith(prefix);  // Toggle operation is based on first line state
        break;
    case -1:
        prefixState = true;
        break;
    case 1:
        prefixState = false;
        break;
    }

    int changesCount = 0;

    for (int i = 0; i < l.length(); i++)
    {
        QString& s = l[i];

        if (s.trimmed().isEmpty() == false)
        {
            if (prefixState == true)
            {
                // remove prefix
                //
                if (s.startsWith(prefix) == true)
                {
                    int commentIndex = s.indexOf(prefix);
                    if (commentIndex != -1)
                    {
                        s = s.remove(commentIndex, prefix.length());
                        changesCount--;
                    }
                }
            }
            else
            {
                // add prefix
                //
                s = prefix + s;
                changesCount++;
            }
        }

        if (i < l.length() - 1)
        {
            s.append("\n");
        }
		c.insertText(s);
	}
    
    // Restore selection state
    //
    if (clearSelection == true)
	{
		c.clearSelection();
        c.setPosition(position + changesCount * prefix.length());
	}
	else
	{
		c.setPosition(selectionStart);
		c.setPosition(selectionEnd + changesCount * prefix.length(), QTextCursor::KeepAnchor);
    }
	setTextCursor(c);

    return true;
}

bool CodeEditor::eventFilter(QObject *object, QEvent *event)
{
    if (object == this && event->type() == QEvent::KeyPress) 
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Tab && hasSelectedText() == true)
		{
			if ((qApp->keyboardModifiers() & Qt::ControlModifier) == 0)
			{
				// Tab - add ident
				//
				processPrefix(m_tabSymbol, 1);
			}
			else
			{
				// Ctrl+Tab - remove ident
				//
				processPrefix(m_tabSymbol, -1);
			}
			return true;
		}
    }
    return false;
}

void CodeEditor::keyPressEvent(QKeyEvent* e)
{
    bool keyEventProcessed = false;

    if (m_autoIndent == true
        && (qApp->keyboardModifiers() == Qt::NoModifier && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return))  // Enter key 
        || e->text() == "}")
    {
        keyEventProcessed |= processAutoIdent(e);
    }

    if ((qApp->keyboardModifiers() & Qt::ControlModifier)  != 0 && e->key() == Qt::Key_Slash)
    {
        keyEventProcessed |= processPrefix("//", 0);
    }

    if ((qApp->keyboardModifiers() & Qt::AltModifier) != 0 && e->key() == Qt::Key_Left)
    {
        goBack();
        keyEventProcessed = true;
    }

    if ((qApp->keyboardModifiers() & Qt::AltModifier) != 0 && e->key() == Qt::Key_Right)
    {
        goForward();
        keyEventProcessed = true;
    }

    if (keyEventProcessed == false)
    {
        QPlainTextEdit::keyPressEvent(e);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), getLineNumberAreaWidth(), cr.height()));
}

void CodeEditor::contextMenuEvent (QContextMenuEvent *e)
{
    QMenu* menu = createStandardContextMenu();

    if (m_customMenuActions.empty() == false)
    {
        emit customContextMenuAboutToBeShown();

        menu->addSeparator();
        menu->addActions(m_customMenuActions);
    }
    menu->exec(e->globalPos());
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
	if (m_highlighter != nullptr)
	{
		bool b = signalsBlocked();

		if (b == false)
		{
			blockSignals(true);
		}

		updateHighlighter();

		if (b == false)
		{
			blockSignals(false);
		}
	}

	QPlainTextEdit::paintEvent(event);
}

void CodeEditor::updateHighlighter()
{
	if (m_highlighter == nullptr)
	{
		return;
	}

	int startPos = cursorForPosition(QPoint(0, 0)).position();
	QPoint bottom_right(viewport()->width() - 1, viewport()->height() - 1);
	int endPos = cursorForPosition(bottom_right).position();

	if (startPos == m_startPosPrev && endPos == m_endPosPrev)
	{
		return;
	}
	m_startPosPrev = startPos;
	m_endPosPrev = endPos;

	QTextCursor cursor = textCursor();
	cursor.setPosition(startPos);
	QTextBlock startBlock = cursor.block();
	cursor.setPosition(endPos);
	cursor.movePosition(QTextCursor::NextBlock);
	QTextBlock endBlock = cursor.block();

	//Iterate visible blocks
	//
	for(QTextBlock b = startBlock; b.isValid() && b != endBlock; b = b.next())
	{
		int st = b.userState();
		if (st == -1)
			st = 0;
		bool processedPeriodicHighlight = (st & 0x80) != 0;
		if (processedPeriodicHighlight == true)
		{
			continue;
		}
		b.setUserState(b.userState() | 0x80);

		m_highlighter->rehighlightBlock(b);
	}
}

void CodeEditor::highlightCurrentLine()
{
    // highlight Current Line
    //
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (m_caretLineVisible == true && isReadOnly() == false)
    {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(m_caretLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::saveCursorHistory()
{
    // Process cursor position history
    //
    int cursorPosition = textCursor().position();

	// Push the last cursor position onto the "back" history stack if it is empty
	//
	if (m_cursorBackwardHistory.empty() == true)
	{
		m_cursorBackwardHistory.push(m_lastCursorPosition);
	}

    // Write new cursor position if it is changed in more than 1 position or line
    //
    int lastLine = document()->findBlock(m_lastCursorPosition).blockNumber();
    int currentLine = document()->findBlock(cursorPosition).blockNumber();

    if (lastLine != currentLine || abs(m_lastCursorPosition - cursorPosition) > 1)
    {
        // Clear the "forward" history stack since any forward history should be invalidated when the cursor moves
        //
        while (m_cursorForwardHistory.empty() == false)
        {
            m_cursorForwardHistory.pop();
        }

        // Push the current cursor position onto the "back" history stack
        //
        m_cursorBackwardHistory.push(cursorPosition);
    }

    // Save current cursor position as last
    //
    m_lastCursorPosition = cursorPosition;
}

void CodeEditor::goBack()
{
    // Pop the top element from the "back" history stack (if it's not empty).
    //
    if (m_cursorBackwardHistory.empty() == true)
    {
        return;
    }

    int currentPosition = textCursor().position();

    // Push the current positon to forward history
    //
    m_cursorForwardHistory.push(currentPosition);

    int backPos = currentPosition;
    while (backPos == currentPosition && m_cursorBackwardHistory.empty() == false)
    {
        backPos = m_cursorBackwardHistory.top();
        m_cursorBackwardHistory.pop();
    }

    // Set the cursor position to the popped position.
    //
    QTextCursor c = textCursor();
    c.setPosition(backPos);
    m_lastCursorPosition = backPos;

    blockSignals(true);
    setTextCursor(c);
    blockSignals(false);

    //Push the popped position onto the "forward" history stack.
    //
    m_cursorForwardHistory.push(backPos);
}

void CodeEditor::goForward()
{
    // Pop the top element from the "forward" history stack (if it's not empty).
    //
    if (m_cursorForwardHistory.empty() == true)
    {
        return;
    }

    int currentPosition = textCursor().position();

    // Push the current positon to backward history
    //
    m_cursorBackwardHistory.push(currentPosition);

    int forwardPos = currentPosition;
    while (forwardPos == currentPosition && m_cursorForwardHistory.empty() == false)
    {
        forwardPos = m_cursorForwardHistory.top();
        m_cursorForwardHistory.pop();
    }

    // Set the cursor position to the popped position.
    //
	QTextCursor c = textCursor();
	c.setPosition(forwardPos);
    m_lastCursorPosition = forwardPos;

	blockSignals(true);
	setTextCursor(c);
	blockSignals(false);

    //Push the popped position onto the "back" history stack.
    //
    m_cursorBackwardHistory.push(forwardPos);
}

void CodeEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(getLineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        m_lineNumberArea->scroll(0, dy);
    }
    else
    {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth();
    }
}

void CodeEditor::onCursorPositionChanged()
{
    saveCursorHistory();

    highlightCurrentLine();
}

//
// ------------------------------------------------------------------------------------------
//
LineNumberArea::LineNumberArea(CodeEditor *editor) :
    QWidget(editor),
    m_codeEditor(editor)
{

}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEditor->getLineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_codeEditor->lineNumberAreaPaintEvent(event);
}

//
// ------------------------------------------------------------------------------------------
//
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(m_highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(static_cast<int>(match.capturedStart()), static_cast<int>(match.capturedLength()), rule.format);
        }
    }

    extraHighlightBlock(text);
}

//
// ------------------------------------------------------------------------------------------
//
void JsHighlighter::createJsHighlighter(CodeEditor* codeEditor)
{
	JsHighlighter* h = new JsHighlighter(codeEditor->document());
    h->initializeFormat();
	codeEditor->setHighlighter(h);
	return;
}

JsHighlighter::JsHighlighter(QTextDocument *parent):
    Highlighter(parent)
{

}

void JsHighlighter::initializeFormat()
{
    QTextCharFormat keywordFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;

    HighlightingRule rule;

   keywordFormat.setForeground(QColor(0x7f, 0x7f, 0x00));

    static const char* keywordArray =
        "abstract boolean break byte case catch char class const continue "
        "debugger default delete do double else enum export extends final "
        "finally float for function goto if implements import in instanceof "
        "int interface long native new package private protected public "
        "return short static super switch synchronized this throw throws "
        "transient try typeof var void volatile while with let";


    QStringList keywordPatterns = QString(keywordArray).split(' ');

    for (const QString &pattern : keywordPatterns) {
		rule.pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.format = keywordFormat;
        m_highlightingRules.append(rule);
    }

    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    m_highlightingRules.append(rule);

    numberFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[0x]*[a-f0-9]+\\b"));
    rule.format = numberFormat;
    m_highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*?\""));
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    m_highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_multiLineCommentFormat.setForeground(Qt::darkGreen);
}

void JsHighlighter::extraHighlightBlock(const QString &text)
{
    static QRegularExpression commentStartExpression(QStringLiteral("/\\*"));
    static QRegularExpression commentEndExpression(QStringLiteral("\\*/"));

    int st = currentBlockState();
    if (st == -1)
        st = 0;
    bool processedPeriodicHighlight = (st & 0x80) != 0;

    setCurrentBlockState(0 | (processedPeriodicHighlight ? 0x80 : 0));

    int pt = previousBlockState();
    if (pt == -1)
        pt = 0;
    int previousState = pt & ~0x80;

    qsizetype startIndex = 0;
    if (previousState != 1)
    {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        qsizetype endIndex = match.capturedStart();
        qsizetype commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1 | (processedPeriodicHighlight ? 0x80 : 0));
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(static_cast<int>(startIndex), static_cast<int>(commentLength), m_multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }

}

//
// ------------------------------------------------------------------------------------------
//
void XmlHighlighter::createXmlHighlighter(CodeEditor *codeEditor)
{
	XmlHighlighter* h = new XmlHighlighter(codeEditor->document());
    h->initializeFormat();
	codeEditor->setHighlighter(h);
	return;
}

XmlHighlighter::XmlHighlighter(QTextDocument *parent):
    Highlighter(parent)
{

}

void XmlHighlighter::initializeFormat()
{
    QTextCharFormat     xmlKeywordFormat;
    QTextCharFormat     xmlElementFormat;
    QTextCharFormat     xmlAttributeFormat;
    QTextCharFormat     xmlValueFormat;
    QTextCharFormat     xmlCommentFormat;

    HighlightingRule rule;

    xmlElementFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegularExpression(QStringLiteral("<[?\\s]*[/]?[\\s]*([^\\n][^>]*)(?=[\\s/>])"));
    rule.format = xmlElementFormat;
    m_highlightingRules.append(rule);

    xmlAttributeFormat.setForeground(QColor(0x00,0x80,0x80));
    rule.pattern = QRegularExpression(QStringLiteral("\\w+(?=\\=)"));
    rule.format = xmlAttributeFormat;
    m_highlightingRules.append(rule);

    xmlValueFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\\n\"]+\"(?=[?\\s/>])"));
    rule.format = xmlValueFormat;
    m_highlightingRules.append(rule);

    xmlCommentFormat.setForeground(QColor(0x80,0x80,0x00));
    rule.pattern = QRegularExpression(QStringLiteral("<!--[^\\n]*-->"));
    rule.format = xmlCommentFormat;
    m_highlightingRules.append(rule);

    QList<QRegularExpression> xmlKeywordRegexes;
    xmlKeywordRegexes = QList<QRegularExpression>()
            << QRegularExpression("<\\?")
            << QRegularExpression("/>")
            << QRegularExpression(">")
            << QRegularExpression("<")
            << QRegularExpression("</")
            << QRegularExpression("\\?>");
    xmlKeywordFormat.setForeground(Qt::blue);
    xmlKeywordFormat.setFontWeight(QFont::Bold);

    for (const auto& it: xmlKeywordRegexes)
    {
        rule.pattern = QRegularExpression(it.pattern());
        rule.format = xmlCommentFormat;
        m_highlightingRules.append(rule);
    }
}



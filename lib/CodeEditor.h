#pragma once

#include <QWidget>
#include <QMenu>
#include <QPainter>
#include <QTextBlock>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <stack>

class Highlighter;

struct FindContext
{
    QString text;
    bool caseSensitive = false;
    bool wholeWord = false;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    bool autoIdent() const;
    void setAutoIndent(bool autoIdent);

    void setCustomMenuActions(QList<QAction*> actions);

    void setFont(const QFont& f);

	void setHighlighter(Highlighter* highlighter);

    // State
    //
    bool isModified() const;
    void setModified(bool value);

    // Text format
    //
    void setCaretLineVisible(bool visible);
    void setCaretLineBackgroundColor(QColor color);
    void setCaretWidth(int w);
    void setTabWidth(int w);

    // Cursor pos
    //
    void getCursorPosition(int* line, int* index) const;
    void setCurrentLine(int line);
    int lines() const;

    // Line Number Area
    //
    bool lineNumberAreaVisible() const;
    void setLineNumberAreaVisible(bool visible);

    int lineNumberOffset() const;
    void setLineNumberOffset(int offset);

    int getLineNumberAreaWidth();

    int customLineNumberAreaWidth();
    void setCustomLineNumberAreaWidth(int width);

    QColor lineNumberAreaBackgroundColor() const;
    void setLineNumberAreaBackgroundColor(const QColor& color);

    QColor lineNumberAreaForegroundColor() const;
    void setLineNumberAreaForegroundColor(const QColor& color);

    // Find/replace
    //
    bool findFirst(const QString& text, bool caseSensitive, bool whole);
    bool findNext();

    bool hasSelectedText() const;
    QString selectedText() const;

    void replace(const QString& text);

    // Utility functions
    //
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    void keyPressEvent( QKeyEvent* e) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent (QContextMenuEvent *e) override;
	void paintEvent(QPaintEvent *event) override;

    bool processAutoIdent(QKeyEvent* e);
    bool processPrefix(const QString& prefix, int operationCode);

	void updateHighlighter();
    void highlightCurrentLine();

    // Cursor history functions
    //
    void saveCursorHistory();
    void goBack();
    void goForward();

signals:
    void customContextMenuAboutToBeShown();

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);

    void onCursorPositionChanged();

private:
    QWidget* m_lineNumberArea = nullptr;
	Highlighter* m_highlighter = nullptr;

    bool m_autoIndent = true;

    QList<QAction*> m_customMenuActions;
    QMenu m_replaceMenu;
    QAction* m_replaceSelectedAction = nullptr;
    QAction* m_replaceAllAction = nullptr;

    bool m_caretLineVisible = true;
    QColor m_caretLineColor = QColor(0xf0f0f0);

    int m_customLineNumberAreaWidth = -1;
    bool m_lineNumberAreaVisible = true;
    int m_lineNumberOffset = 0;
    QColor m_lineNumberAreaBackgroundColor = QColor(Qt::lightGray);
    QColor m_lineNumberAreaForegroundColor = QColor(Qt::black);

    FindContext m_findContext;

    QString m_tabSymbol;
    int m_tabWidth = 4;

	int m_startPosPrev = 0;
	int m_endPosPrev = 0;

    // Cursor position history
    //
    int m_lastCursorPosition = 0;
    std::stack<int> m_cursorBackwardHistory;
    std::stack<int> m_cursorForwardHistory;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor);

private:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    CodeEditor *m_codeEditor = nullptr;
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

protected:
    Highlighter(QTextDocument *parent = nullptr);

protected:
    virtual void initializeFormat() = 0;

    void highlightBlock(const QString &text) override;

    virtual void extraHighlightBlock(const QString &text) {Q_UNUSED(text);}

protected:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> m_highlightingRules;

};

class JsHighlighter : public Highlighter
{

public:
	static void createJsHighlighter(CodeEditor *codeEditor);

private:
    JsHighlighter(QTextDocument *parent);

private:
    virtual void initializeFormat() override;
    virtual void extraHighlightBlock(const QString &text) override;

private:
    QTextCharFormat m_multiLineCommentFormat;
};

class XmlHighlighter : public Highlighter
{

public:
	static void createXmlHighlighter(CodeEditor* codeEditor);

private:
    XmlHighlighter(QTextDocument *parent);

private:
    virtual void initializeFormat() override;
};


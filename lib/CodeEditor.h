#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <memory>
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QSpinBox>
#include <QCheckBox>
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QTextEdit>
#include <QDialog>
#include <QSet>
#include <QComboBox>
#include <QStringList>
#include <QPlainTextEdit>
#include <QTextBlock>


namespace ExtWidgets
{
	class CodeEditor : public QPlainTextEdit
	{
		Q_OBJECT

	public:
		CodeEditor(QWidget *parent = 0);

		void lineNumberAreaPaintEvent(QPaintEvent *event);
		int lineNumberAreaWidth();

	protected:
		void resizeEvent(QResizeEvent *event) override;

	private slots:
		void updateLineNumberAreaWidth(int newBlockCount);
		void highlightCurrentLine();
		void updateLineNumberArea(const QRect &, int);

	private:
		QWidget *lineNumberArea;
	};

	class LineNumberArea : public QWidget
	{
	public:
		LineNumberArea(CodeEditor *editor) : QWidget(editor) {
			codeEditor = editor;
		}

		QSize sizeHint() const override {
			return QSize(codeEditor->lineNumberAreaWidth(), 0);
		}

	protected:
		void paintEvent(QPaintEvent *event) override {
			codeEditor->lineNumberAreaPaintEvent(event);
		}

	private:
		CodeEditor *codeEditor;
	};
}

#endif

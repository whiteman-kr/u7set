#ifndef SVGEDITOR_H
#define SVGEDITOR_H

#include "../lib/PropertyEditor.h"
#include "../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"
#include "../lib/QScintillaLexers/LexerXML.h"

class SvgWidget : public QWidget
{
public:
	SvgWidget(QWidget* parent);

	const QString& svgData() const;
	void setSvgData(const QString& data);

	bool isValid() const;

	bool stretch() const;
	void setStretch(bool value);

protected:
	void paintEvent(QPaintEvent* pe) override;

private:
	QString m_svgData;
	QSvgRenderer m_svgRenderer;		// Drawing resources

	bool m_stretch = true;
};

class SvgEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT
public:
	explicit SvgEditor(QWidget* parent);
	virtual ~SvgEditor();

	void setText(const QString& text) override;
	QString text() override;

	void setReadOnly(bool value) override;

private slots:
	void onTextChanged();

	void onOkClicked();
	void onCancelClicked();

	void onStretchCheckClicked(bool checked);


private:
	QsciScintilla* m_textEdit = nullptr;

	LexerXML m_lexerXml;

	QSplitter* m_topSplitter = nullptr;
	SvgWidget m_svgWidget;

	QWidget* m_parent = nullptr;
};

#endif // SVGEDITOR_H

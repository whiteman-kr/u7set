#ifndef SVGEDITOR_H
#define SVGEDITOR_H

#include <UiLib/PropertyEditor.h>

namespace UiLib
{
	class CodeEditor;
}

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

	QString text() const override;
	void setText(const QString& text) override;

	bool readOnly() const override;
	void setReadOnly(bool value) override;

	bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private slots:
	void onTextChanged();

	void onOkClicked();
	void onCancelClicked();

	void onStretchCheckClicked(bool checked);

private:
	UiLib::CodeEditor* m_textEdit = nullptr;

	QSplitter* m_topSplitter = nullptr;
	SvgWidget m_svgWidget;

	QPushButton* m_okButton = nullptr;
	QPushButton* m_cancelButton = nullptr;

	QWidget* m_parent = nullptr;
};

#endif // SVGEDITOR_H

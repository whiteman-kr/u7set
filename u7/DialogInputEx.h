#ifndef DIALOGINPUTEX_H
#define DIALOGINPUTEX_H

#include "../lib/PropertyEditor.h"

class DialogInputEx : public QDialog
{
	Q_OBJECT

public:
	DialogInputEx(QWidget *parent, bool multiLine, int dialogWidth = -1, int dialogHeight = -1);
	virtual ~DialogInputEx();

public:
	// Static functions
	//
	static QString getSingleLineText(QWidget *parent,
									 const QString &title,
									 const QString &label,
									 const QString &text = QString(),
									 bool *ok = Q_NULLPTR,
									 const QString& validator = QString(),
									 int width = -1,
									 int height = -1);

	static QString getMultiLineText(QWidget *parent,
									const QString &title,
									const QString &label,
									const QString &text = QString(),
									bool *ok = Q_NULLPTR,
									const QString& validator = QString(),
									int width = -1,
									int height = -1);

public:
	// Operations
	//
	void setLabelText(const QString& text);
	void setValidator(const QString& validator);

	QString textValue() const;
	void setTextValue(const QString& value);

private:
	QString getText(const QString &title,
					const QString &label,
					const QString &text,
					bool *ok,
					const QString& validator);


private:
	bool m_multiLine = false;

	QLabel* m_labelText = nullptr;

	QLineEdit* m_lineEdit = nullptr;

	ExtWidgets::PropertyPlainTextEditor* m_plainTextEditor = nullptr;
};

#endif // DIALOGINPUTEX_H

#include "DialogInputEx.h"

DialogInputEx::DialogInputEx(QWidget *parent, bool multiLine, int dialogWidth, int dialogHeight):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_multiLine(multiLine)
{

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	m_labelText = new QLabel();
	mainLayout->addWidget(m_labelText);

	if (m_multiLine == true)
	{
		m_plainTextEditor = new ExtWidgets::PropertyPlainTextEditor(this);
		mainLayout->addWidget(m_plainTextEditor);
	}
	else
	{
		m_lineEdit = new QLineEdit();
		mainLayout->addWidget(m_lineEdit);
	}

	QHBoxLayout* boxLayout = new QHBoxLayout();
	boxLayout->addStretch();

	QPushButton* b = new QPushButton(tr("OK"));
	connect(b, &QPushButton::clicked, this, &QDialog::accept);
	boxLayout->addWidget(b);

	b = new QPushButton(tr("Cancel"));
	connect(b, &QPushButton::clicked, this, &QDialog::reject);
	boxLayout->addWidget(b);

	mainLayout->addLayout(boxLayout);

	// Resize the dialog

	if (dialogWidth != -1 || dialogHeight != -1)
	{
		if (dialogWidth == -1)
		{
			dialogWidth = width();
		}

		if (dialogHeight == -1)
		{
			dialogHeight = height();
		}

		resize(dialogWidth, dialogHeight);
	}

}

DialogInputEx::~DialogInputEx()
{
}

QString DialogInputEx::getSingleLineText(QWidget *parent,
										 const QString &title,
										 const QString &label,
										 const QString &text,
										 bool *ok,
										 const QString& validator,
										 int width,
										 int height)
{

	DialogInputEx inputDialog(parent, false, width, height);
	return inputDialog.getText(title, label, text, ok, validator);
}

QString DialogInputEx::getMultiLineText(QWidget *parent,
										const QString &title,
										const QString &label,
										const QString &text,
										bool *ok,
										const QString& validator,
										int width,
										int height)
{

	DialogInputEx inputDialog(parent, true, width, height);
	return inputDialog.getText(title, label, text, ok, validator);
}

void DialogInputEx::setLabelText(const QString& text)
{
	if (m_labelText == nullptr)
	{
		assert(m_labelText);
		return;
	}

	m_labelText->setText(text);

}

void DialogInputEx::setValidator(const QString& validator)
{
	if (m_multiLine == false)
	{
		m_lineEdit->setValidator(new QRegExpValidator(QRegExp(validator), this));
	}
	else
	{
		m_plainTextEditor->setValidator(validator);
	}
}

QString DialogInputEx::textValue() const
{
	if (m_multiLine == true)
	{
		if (m_plainTextEditor == nullptr)
		{
			assert(m_plainTextEditor);
			return QString();
		}

		return m_plainTextEditor->text();
	}
	else
	{
		if (m_lineEdit == nullptr)
		{
			assert(m_lineEdit);
			return QString();
		}

		return m_lineEdit->text();
	}
}

void DialogInputEx::setTextValue(const QString& value)
{
	if (m_multiLine == true)
	{
		if (m_plainTextEditor == nullptr)
		{
			assert(m_plainTextEditor);
			return;
		}

		m_plainTextEditor->setText(value);
	}
	else
	{
		if (m_lineEdit == nullptr)
		{
			assert(m_lineEdit);
			return;
		}

		m_lineEdit->setText(value);
	}
}

QString DialogInputEx::getText(
		const QString &title,
		const QString &label,
		const QString &text,
		bool *ok,
		const QString& validator
		)
{
	setWindowTitle(title);
	setLabelText(label);
	setTextValue(text);
	setValidator(validator);

	//

	int inputDialogResult = exec();

	QString newValue = textValue();

	if (inputDialogResult == QDialog::Accepted && newValue.isNull() == false)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}
		return newValue;
	}

	if (ok != nullptr)
	{
		*ok = false;
	}

	return QString();
}

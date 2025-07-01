#include "CsvEditorDialog.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTextEdit>

CsvEditorDialog::CsvEditorDialog(const QString& csvFile, QWidget* parent) :
	QDialog(parent),
	m_csvFile(csvFile)
{
	setWindowTitle(tr("Edit CSV File"));
	m_textEdit = new QTextEdit(this);
	m_newFileBtn = new QPushButton(tr("New Signals"), this);
	m_saveBtn = new QPushButton(tr("Save"), this);

	QHBoxLayout* m_buttonsLayout = new QHBoxLayout;
	m_buttonsLayout->addStretch();
	m_buttonsLayout->addWidget(m_newFileBtn);
	m_buttonsLayout->addWidget(m_saveBtn);

	QFormLayout* layout = new QFormLayout(this);

	layout->addWidget(m_textEdit);
	layout->addRow(m_buttonsLayout);
	setLayout(layout);

	// Load CSV file
	QFile file(m_csvFile);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream in(&file);
		m_textEdit->setPlainText(in.readAll());
		file.close();
	}
	connect(m_newFileBtn, &QPushButton::clicked, this, &CsvEditorDialog::onNewFile);
	connect(m_saveBtn, &QPushButton::clicked, this, &CsvEditorDialog::onSave);

	resize(620, 300);
	setMinimumSize(450, 250);
}

void CsvEditorDialog::onNewFile()
{
	m_textEdit->clear();
	const QString placeholder = "<SygnalID%1_or_%1%2>";
	QString templateText = "TYPE;ID;[Start];[END]\n"
						   "D;#SygnalID;1;32\n"
						   "I;#SygnalID|#SygnalID;0;0\n"
						   "F;#SygnalID;1;32";

	templateText.replace("SygnalID", placeholder);
	m_textEdit->setPlainText(templateText);
	m_textEdit->setPlaceholderText(placeholder);
}

void CsvEditorDialog::onSave()
{
	QFile file(m_csvFile);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream out(&file);
		out << m_textEdit->toPlainText();
		file.close();
		emit csvSaved(m_csvFile);
		accept();
	}
	else
	{
		QMessageBox::warning(this, tr("Error"), tr("Could not save file!"));
	}
}
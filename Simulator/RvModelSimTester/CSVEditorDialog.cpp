#include "CSVEditorDialog.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

CsvEditorDialog::CsvEditorDialog(const QString& csvFile, QWidget* parent) :
	QDialog(parent),
	m_csvFile(csvFile)
{
	setWindowTitle("Edit signals.csv");
	m_textEdit = new QTextEdit(this);
	m_newFileBtn = new QPushButton("New Signals", this);
	m_saveBtn = new QPushButton("Save", this);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(m_newFileBtn);
	buttonsLayout->addWidget(m_saveBtn);

	QFormLayout* layout = new QFormLayout(this);
	
	layout->addWidget(m_textEdit);
	//layout->addStretch();
	layout->addRow(buttonsLayout);
	setLayout(layout);

	// Load CSV
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

void CsvEditorDialog::onNewFile() {
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
		QMessageBox::warning(this, "Error", "Could not save file!");
	}
}
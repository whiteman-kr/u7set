#include "DialogImport.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QFileDialog>

DialogImport::DialogImport(const QString& comparatorsFile, const QString& appSignalsFile, QDialog* parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Import"));

	m_fileComparatorsEdit = new QLineEdit;
	m_fileComparatorsEdit->setText(comparatorsFile);

	m_fileAppSignalsEdit = new QLineEdit;
	m_fileAppSignalsEdit->setText(appSignalsFile);

	QFormLayout* layout = new QFormLayout;

	QPushButton* comparatorsBrowse = new QPushButton(tr("Browse..."));
	QHBoxLayout* comparatorsLayout = new QHBoxLayout;
	comparatorsLayout->setContentsMargins(0, 0, 0, 0);
	comparatorsLayout->addWidget(m_fileComparatorsEdit);
	comparatorsLayout->addWidget(comparatorsBrowse);

	QString defaultComparatorsPath = QCoreApplication::applicationDirPath() + "/Comparators.set";
	defaultComparatorsPath = QSettings().value("fileComparators", defaultComparatorsPath).toString();

	connect(comparatorsBrowse,
			&QPushButton::clicked,
			this,
			[this, defaultComparatorsPath]()
			{
				QString fileName =
					QFileDialog::getOpenFileName(this, tr("Select Comparators File"), 
						m_fileComparatorsEdit->text().isEmpty() ? defaultComparatorsPath : m_fileComparatorsEdit->text(), 
						tr("All Files (*.set)"));

				if (fileName.isEmpty() == false)
				{
					m_fileComparatorsEdit->setText(fileName);
				}
			});

	layout->addRow(tr("Comparators File:"), comparatorsLayout);

	QPushButton* appSignalsBrowse = new QPushButton(tr("Browse..."));

	QHBoxLayout* appSignalsLayout = new QHBoxLayout;
	appSignalsLayout->setContentsMargins(0, 0, 0, 0);
	appSignalsLayout->addWidget(m_fileAppSignalsEdit);
	appSignalsLayout->addWidget(appSignalsBrowse);

	QString defaultAppSignalsPath = QCoreApplication::applicationDirPath() + "/AppSignals.asgs";
	defaultAppSignalsPath = QSettings().value("fileAppSignals", defaultAppSignalsPath).toString();

	connect(appSignalsBrowse,
			&QPushButton::clicked,
			this,
			[this, defaultAppSignalsPath]()
			{
				QString fileName =
					QFileDialog::getOpenFileName(this,
												 tr("Select AppSignals File"),
												 defaultAppSignalsPath,
												 tr("All Files (*.asgs)"));

				if (fileName.isEmpty() == false)
				{
					m_fileAppSignalsEdit->setText(fileName);
				}
			});
	layout->addRow(tr("AppSignals File:"), appSignalsLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	QPushButton* okButton = new QPushButton(tr("Load"));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	connect(okButton, &QPushButton::clicked, this, &DialogImport::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	layout->addRow(buttonsLayout);

	setLayout(layout);

	setFixedSize(620, 140);
}

QString DialogImport::comparatorsFile() const
{ 
	return m_comparatorsFile; 
}

QString DialogImport::appSignalsFile() const 
{ 
	return m_appSignalsFile; 
}

void DialogImport::accept()
{
	m_comparatorsFile = m_fileComparatorsEdit->text();
	m_appSignalsFile = m_fileAppSignalsEdit->text();

	QDialog::accept();
}
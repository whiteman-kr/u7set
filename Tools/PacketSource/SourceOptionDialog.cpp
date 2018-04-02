#include "SourceOptionDialog.h"

#include <QFile>
#include <QFileDialog>

// -------------------------------------------------------------------------------------------------------------------

SourceOptionDialog::SourceOptionDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

SourceOptionDialog::~SourceOptionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SourceOptionDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));
	setFixedSize(400, 110);

	// Path
	//
	QGroupBox* groupPath = new QGroupBox(tr("Path"));
	QHBoxLayout *pathLayout = new QHBoxLayout;

	m_pathEdit = new QLineEdit(theOptions.source().path(), this);
	m_selectPathBtn = new QPushButton(tr("Select ..."), this);

	pathLayout->addWidget(m_pathEdit);
	//serverPathLayout->addStretch();
	pathLayout->addWidget(m_selectPathBtn);

	connect(m_selectPathBtn, &QPushButton::clicked, this, &SourceOptionDialog::onSelectPath);

	groupPath->setLayout(pathLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SourceOptionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SourceOptionDialog::reject);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(groupPath);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOptionDialog::onSelectPath()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::currentPath());
	if (path.isEmpty() == true)
	{
		return;
	}

	m_pathEdit->setText(path);
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOptionDialog::onOk()
{
	QString path = m_pathEdit->text();
	if (path.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input path to sources file!"));
		return;
	}

	if (QFile::exists(path) == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Path to sources is not found!"));
		return;
	}

	theOptions.source().setPath(path);

	theOptions.source().save();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

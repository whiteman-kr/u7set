#include "CreateSchemeDialog.h"
#include "ui_CreateSchemeDialog.h"
#include "../VFrame30/Settings.h"

CreateSchemeDialog::CreateSchemeDialog(std::shared_ptr<VFrame30::Scheme> scheme, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::CreateSchemeDialog),
	m_scheme(scheme)
{
	assert(m_scheme.get() != nullptr);
	ui->setupUi(this);

	ui->strdIdEdit->setText(m_scheme->strID());
	ui->captionEdit->setText(m_scheme->caption());

	double w = 0;
	double h = 0;
	double precision = 0;

	if (m_scheme->unit() == VFrame30::SchemeUnit::Display)
	{
		w = m_scheme->docWidth();
		h = m_scheme->docHeight();
		precision = 0;
	}
	else
	{
		assert(m_scheme->unit() == VFrame30::SchemeUnit::Inch);

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemeUnit::Inch)
		{
			w = m_scheme->docWidth();
			h = m_scheme->docHeight();
			precision = 4;
		}
		else
		{
			assert(VFrame30::Settings::regionalUnit() == VFrame30::SchemeUnit::Millimeter);

			w = m_scheme->docWidth() * 25.4;
			h = m_scheme->docHeight() * 25.4;
			precision = 2;
		}
	}

	ui->widthEdit->setText(QString::number(w, 'f', precision));
	ui->heightEdit->setText(QString::number(h, 'f', precision));

	return;
}

CreateSchemeDialog::~CreateSchemeDialog()
{
	delete ui;
}

void CreateSchemeDialog::accept()
{
	// StrdID
	//
	QString strID = ui->strdIdEdit->text();
	if (strID.isEmpty() == true)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid StrID."));
		msgBox.exec();

		ui->strdIdEdit->setFocus();
		return;
	}

	// Caption
	//
	QString caption = ui->captionEdit->text();
	if (caption.isEmpty() == true)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid caption."));
		msgBox.exec();

		ui->captionEdit->setFocus();
		return;
	}

	// Width
	//
	bool widthResult = false;
	double width = ui->widthEdit->text().toDouble(&widthResult);

	if (widthResult == false ||
		width <= 0.0)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid width."));
		msgBox.exec();

		ui->widthEdit->setFocus();
		return;
	}

	// Height
	//
	bool heightResult = false;
	double height = ui->heightEdit->text().toDouble(&heightResult);

	if (heightResult == false ||
		height <= 0.0)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Enter valid height."));
		msgBox.exec();

		ui->heightEdit->setFocus();
		return;
	}

	// Assign values to the scheme
	//
	m_scheme->setStrID(strID);
	m_scheme->setCaption(caption);


	if (m_scheme->unit() == VFrame30::SchemeUnit::Display)
	{
		m_scheme->setDocWidth(width);
		m_scheme->setDocHeight(height);
	}
	else
	{
		assert(m_scheme->unit() == VFrame30::SchemeUnit::Inch);

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemeUnit::Inch)
		{
			m_scheme->setDocWidth(width);
			m_scheme->setDocHeight(height);
		}
		else
		{
			assert(VFrame30::Settings::regionalUnit() == VFrame30::SchemeUnit::Millimeter);

			m_scheme->setDocWidth(width / 25.4);
			m_scheme->setDocHeight(height / 25.4);
		}
	}

	QDialog::accept();
}

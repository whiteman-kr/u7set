#include "CreateSchemaDialog.h"
#include "ui_CreateSchemaDialog.h"
#include "../VFrame30/Settings.h"

CreateSchemaDialog::CreateSchemaDialog(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::CreateSchemaDialog),
	m_schema(schema)
{
	assert(m_schema.get() != nullptr);
	ui->setupUi(this);

	ui->strdIdEdit->setText(m_schema->strID());
	ui->captionEdit->setText(m_schema->caption());

	double w = 0;
	double h = 0;
	double precision = 0;

	if (m_schema->unit() == VFrame30::SchemaUnit::Display)
	{
		w = m_schema->docWidth();
		h = m_schema->docHeight();
		precision = 0;
	}
	else
	{
		assert(m_schema->unit() == VFrame30::SchemaUnit::Inch);

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Inch)
		{
			w = m_schema->docWidth();
			h = m_schema->docHeight();
			precision = 4;
		}
		else
		{
			assert(VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Millimeter);

			w = m_schema->docWidth() * 25.4;
			h = m_schema->docHeight() * 25.4;
			precision = 2;
		}
	}

	ui->widthEdit->setText(QString::number(w, 'f', precision));
	ui->heightEdit->setText(QString::number(h, 'f', precision));

	return;
}

CreateSchemaDialog::~CreateSchemaDialog()
{
	delete ui;
}

void CreateSchemaDialog::accept()
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

	// Assign values to the schema
	//
	m_schema->setStrID(strID);
	m_schema->setCaption(caption);


	if (m_schema->unit() == VFrame30::SchemaUnit::Display)
	{
		m_schema->setDocWidth(width);
		m_schema->setDocHeight(height);
	}
	else
	{
		assert(m_schema->unit() == VFrame30::SchemaUnit::Inch);

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Inch)
		{
			m_schema->setDocWidth(width);
			m_schema->setDocHeight(height);
		}
		else
		{
			assert(VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Millimeter);

			m_schema->setDocWidth(width / 25.4);
			m_schema->setDocHeight(height / 25.4);
		}
	}

	QDialog::accept();
}

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

	// Set StrID label
	//
	QString idLable = "ID";

	if (isLogicSchema() == true)
	{
		idLable = "AppSchemaID";
	}

	if (isMonitorSchema() == true)
	{
		idLable = "MonitorSchemaID";
	}

	if (isDiagSchema() == true)
	{
		idLable = "DiagSchemaID";
	}

	assert(idLable != "ID");			// Should be corresponded to schema type

	ui->strIdLabel->setText(idLable);

	// Set height and width lables, append px, in or mm
	//
	QString units;

	if (schema->unit() == VFrame30::SchemaUnit::Display)
	{
		units = tr(", px");
	}
	else
	{
		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Inch)
		{
			units = tr(", in");
		}

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Millimeter)
		{
			units = tr(", mm");
		}
	}

	ui->widthLabel->setText(ui->widthLabel->text() + units);
	ui->heigtLabel->setText(ui->heigtLabel->text() + units);

	// Set height and width
	//
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

	// LogicSchame Equipment ID
	//
	if (isLogicSchema() == true)
	{
		ui->equipmentIdLabel->setVisible(true);
		ui->equipmentIdEdit->setVisible(true);

		ui->equipmentIdEdit->setText(logicSchema()->hardwareStrIds());
	}
	else
	{
		ui->equipmentIdLabel->setVisible(false);
		ui->equipmentIdEdit->setVisible(false);
	}

	setWindowTitle(tr("Schema Properties"));

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

	// EquipmentID for LogicSchema
	//
	QString equipmnetId;

	if (isLogicSchema() == true)
	{
		equipmnetId = ui->equipmentIdEdit->text();
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

	if (isLogicSchema() == true)
	{
		logicSchema()->setHardwareStrIds(equipmnetId);
	}

	QDialog::accept();
}

bool CreateSchemaDialog::isLogicSchema() const
{
	return (dynamic_cast<VFrame30::LogicSchema*>(m_schema.get()) != nullptr);
}

bool CreateSchemaDialog::isMonitorSchema() const
{
	return (dynamic_cast<VFrame30::MonitorSchema*>(m_schema.get()) != nullptr);
}

bool CreateSchemaDialog::isDiagSchema() const
{
	return (dynamic_cast<VFrame30::DiagSchema*>(m_schema.get()) != nullptr);
}

std::shared_ptr<VFrame30::LogicSchema> CreateSchemaDialog::logicSchema()
{
	assert(isLogicSchema());

	return std::dynamic_pointer_cast<VFrame30::LogicSchema>(m_schema);
}

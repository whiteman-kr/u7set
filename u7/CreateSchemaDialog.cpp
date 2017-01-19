#include "CreateSchemaDialog.h"
#include "ui_CreateSchemaDialog.h"
#include "../lib/DbController.h"
#include "../VFrame30/Settings.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/FblItemRect.h"

CreateSchemaDialog::CreateSchemaDialog(std::shared_ptr<VFrame30::Schema> schema, DbController* db, int tempateParentFileId, QString templateFileExtension, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::CreateSchemaDialog),
	m_db(db),
	m_schema(schema)
{
	assert(m_schema.get() != nullptr);
	assert(db);

	ui->setupUi(this);

	// Set StrID label
	//
	QString idLable = "ID";

	if (dynamic_cast<VFrame30::LogicSchema*>(m_schema.get()) != nullptr)
	{
		idLable = "AppSchemaID";
	}

	if (dynamic_cast<VFrame30::UfbSchema*>(m_schema.get()) != nullptr)
	{
		idLable = "UserFunctionalBlock ID";
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


	// Set height and width
	//
	ui->strdIdEdit->setText(schema->schemaId());
	ui->captionEdit->setText(schema->caption());

	// Set width/height
	//
	setWidthHeight(m_schema.get());

	// LogicSchame Equipment ID
	//
	if (isLogicSchema() == true)
	{
		ui->equipmentIdLabel->setVisible(true);
		ui->equipmentIdEdit->setVisible(true);

		ui->equipmentIdEdit->setText(logicSchema()->equipmentIds());
	}
	else
	{
		ui->equipmentIdLabel->setVisible(false);
		ui->equipmentIdEdit->setVisible(false);
	}

	setWindowTitle(tr("Schema Properties"));

	// Fill Template combo box
	//
	ui->templateComboBox->addItem(tr("Blank"), QVariant(-1));		// -1 means Blnk, any othe number is DbFileID

	std::vector<DbFileInfo> templates;

	bool ok = db->getFileList(&templates, tempateParentFileId, templateFileExtension, true, parent);

	if (ok == true)
	{
		// Sort files
		//
		std::sort(templates.begin(), templates.end(),
			[](const DbFileInfo& f1, const DbFileInfo& f2)
			{
				return f1.fileName() < f2.fileName();
			});

		// read files
		//
		m_templates.reserve(templates.size());

		db->getLatestVersion(templates, &m_templates, this);

		int defultIndex = -1;
		for (size_t i = 0; i < m_templates.size(); i++)
		{
			std::shared_ptr<DbFile> f = m_templates[i];

			qDebug() << f->fileName();

			std::shared_ptr<VFrame30::Schema> schemaTemplate =  VFrame30::Schema::Create(f->data());

			// Check if type the sane
			//
			if (schemaTemplate->inherits(schema->metaObject()->className()) == false)
			{
				assert(schemaTemplate->inherits(schema->metaObject()->className()) == true);
				continue;
			}

			ui->templateComboBox->addItem(schemaTemplate->caption(), QVariant(f->fileId()));

			if (defultIndex == -1 &&
				f->fileName().startsWith(QLatin1String("Default."), Qt::CaseInsensitive) == true)
			{
				defultIndex = static_cast<int>(i);
			}
		}

		if (defultIndex != -1)
		{
			// Set current selection to item with "Default." file name
			// ComboBox also has "Blank" item, so + 1 is required as defultIndex is an index in file vector
			//
			ui->templateComboBox->setCurrentIndex(defultIndex + 1);

			templateChanged(defultIndex + 1);
		}
	}

	// --
	//
	connect(ui->templateComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &CreateSchemaDialog::templateChanged);

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
	if (m_templateSchema == nullptr)
	{
		// Template Blank is selected
		//
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
	}
	else
	{
		Proto::Envelope data;

		m_templateSchema->Save(&data);
		m_schema->Load(data);

		// Set new uuids and labels to the schema
		//
		m_schema->setGuid(QUuid::createUuid());

		for (auto layer : m_schema->Layers)
		{
			layer->setGuid(QUuid::createUuid());

			for (auto item : layer->Items)
			{
				item->setGuid(QUuid::createUuid());

				if (item->isFblItemRect() == true)
				{
					int counterValue = m_db->nextCounterValue();
					item->toFblItemRect()->setLabel(strID + "_" + QString::number(counterValue));
				}
			}
		}

	}

	m_schema->setSchemaId(strID);
	m_schema->setCaption(caption);

	if (isLogicSchema() == true)
	{
		logicSchema()->setEquipmentIds(equipmnetId);
	}

	QDialog::accept();
}

void CreateSchemaDialog::templateChanged(int index)
{
	int dbFileId = ui->templateComboBox->itemData(index).toInt();

	if (dbFileId == -1)
	{
		m_templateSchema.reset();

		ui->widthEdit->setEnabled(true);
		ui->heightEdit->setEnabled(true);

		setWidthHeight(m_schema.get());

		return;
	}

	// read template from DB and in it width/height
	//
	ui->widthEdit->setEnabled(false);
	ui->heightEdit->setEnabled(false);

	for (std::shared_ptr<DbFile> tf : m_templates)
	{
		if (tf->fileId() == dbFileId)
		{
			m_templateSchema = VFrame30::Schema::Create(tf->data());

			if (m_templateSchema == nullptr)
			{
				assert(m_templateSchema);
				break;
			}

			setWidthHeight(m_templateSchema.get());
			break;
		}
	}

	return;
}

void CreateSchemaDialog::setWidthHeight(VFrame30::Schema* schema)
{
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

	if (ui->widthLabel->text().endsWith(units) == false)
	{
		ui->widthLabel->setText(ui->widthLabel->text() + units);
	}

	if (ui->heigtLabel->text().endsWith(units) == false)
	{
		ui->heigtLabel->setText(ui->heigtLabel->text() + units);
	}

	double w = 0;
	double h = 0;
	double precision = 0;

	if (schema->unit() == VFrame30::SchemaUnit::Display)
	{
		w = schema->docWidth();
		h = schema->docHeight();
		precision = 0;
	}
	else
	{
		assert(m_schema->unit() == VFrame30::SchemaUnit::Inch);

		if (VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Inch)
		{
			w = schema->docWidth();
			h = schema->docHeight();
			precision = 4;
		}
		else
		{
			assert(VFrame30::Settings::regionalUnit() == VFrame30::SchemaUnit::Millimeter);

			w = schema->docWidth() * 25.4;
			h = schema->docHeight() * 25.4;
			precision = 2;
		}
	}

	ui->widthEdit->setText(QString::number(w, 'f', precision));
	ui->heightEdit->setText(QString::number(h, 'f', precision));

	return;
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

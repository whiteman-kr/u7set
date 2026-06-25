#include "CreateActuatorDialog.h"

#include <VFrame30/ActuatorHeader.h>
#include <DbLib/DbController.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>

#include "../Builder/SubsystemStorage.h"
#include "DialogSubsystemListEditor.h"

#include <QSpan>
#include <QSizePolicy>

CreateActuatorDialog::CreateActuatorDialog(std::shared_ptr<VFrame30::ActuatorHeader> actuatorHeader, DbController* db, QWidget* parent) :
	QDialog{parent},
	m_db{db},
	m_actuatorHeader{actuatorHeader}
{
	if (m_actuatorHeader == nullptr || m_db == nullptr)
	{
		assert(m_db);
		assert(m_actuatorHeader);
		return;
	}

	setWindowTitle(tr("Create Actuator"));

	getAcmPresets();

	QStringList acmPresetNames;
	acmPresetNames.reserve(m_presets.size());
	for (const auto& preset : m_presets)
	{
		acmPresetNames.append(preset->presetName());
	}

	m_layout = new QGridLayout{this};

	// Next row
	//
	int row = 0;
	m_actuatorTypeIdLabel = new QLabel{tr("ActuatorTypeID"), this};
	m_layout->addWidget(m_actuatorTypeIdLabel, row, 0);

	m_actuatorTypeIdLineEdit = new QLineEdit{};
	m_actuatorTypeIdLineEdit->setText(actuatorHeader->actuatorTypeId());
	m_layout->addWidget(m_actuatorTypeIdLineEdit, row, 1, 1, 2);

	QRegularExpression regex("[A-Za-z0-9_]+"); // Allow only alphanumeric characters and underscores in ActuatorTypeId
	QValidator* validator = new QRegularExpressionValidator(regex, this);
	m_actuatorTypeIdLineEdit->setValidator(validator);

	// Next row - Caption
	//
	row++;
	m_captionLabel = new QLabel{tr("Caption"), this};
	m_layout->addWidget(m_captionLabel, row, 0);

	m_captionLineEdit = new QLineEdit{this};
	m_captionLineEdit->setText(actuatorHeader->caption());
	m_layout->addWidget(m_captionLineEdit, row, 1, 1, 2);

	// Next row - horizontal separator
	//
	row++; 
	m_separator1 = new QFrame{this};
	m_separator1->setFrameShape(QFrame::HLine);
	m_separator1->setFrameShadow(QFrame::Sunken);
	m_layout->addWidget(m_separator1, row, 0, 1, 3);

	// Next row
	//
	row++;

	m_acmPresetNameLabel = new QLabel{tr("ACM Preset"), this};
	m_acmPresetNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	m_layout->addWidget(m_acmPresetNameLabel, row, 0);

	m_acmPresetNameComboBox = new QComboBox{this};
	m_acmPresetNameComboBox->setEditable(true);
	m_acmPresetNameComboBox->addItems(acmPresetNames);
	m_acmPresetNameComboBox->setCurrentText(actuatorHeader->acmPresetName());

	m_acmPresetNameComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_layout->addWidget(m_acmPresetNameComboBox, row, 1, 1, 2);

	m_layout->setColumnStretch(0, 0);
	m_layout->setColumnStretch(1, 1);

	// Next row
	//
	row++;

	m_moduleDescriptionFileLabel = new QLabel{tr("DescriptionFile"), this};
	m_moduleDescriptionFileLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	m_layout->addWidget(m_moduleDescriptionFileLabel, row, 0);

	m_moduleDescriptionFileLineEdit = new QLineEdit{this};
	QString acmDescriptionFile = getModuleDescriptionFile(actuatorHeader->acmPresetName());
	m_moduleDescriptionFileLineEdit->setText(acmDescriptionFile);
	m_layout->addWidget(m_moduleDescriptionFileLineEdit, row, 1, 1, 2);

	// Next row - horizontal separator
	//
	row++;
	m_separator2 = new QFrame{this};
	m_separator2->setFrameShape(QFrame::HLine);
	m_separator2->setFrameShadow(QFrame::Sunken);
	m_layout->addWidget(m_separator2, row, 0, 1, 3);

	// Next row
	//
	row++;
	m_subsystemIdLabel = new QLabel{tr("SubsystemID"), this};
	m_layout->addWidget(m_subsystemIdLabel, row, 0);

	m_subsystemIdComboBox = new QComboBox{this};
	m_subsystemIdComboBox->addItems(m_subsystemIds);
	m_subsystemIdComboBox->setCurrentText(actuatorHeader->subsystemId());
	m_layout->addWidget(m_subsystemIdComboBox, row, 1);

	m_subsystemsButton = new QPushButton{tr("Subsystems..."), this};
	m_layout->addWidget(m_subsystemsButton, row, 2);

	// Next row
	//
	row++;
	m_lmNumberLabel = new QLabel{tr("LmNumber"), this};
	m_layout->addWidget(m_lmNumberLabel, row, 0); 

	m_lmNumberLineEdit = new QLineEdit{this};
	m_lmNumberLineEdit->setValidator(new QRegularExpressionValidator{QRegularExpression{"\\d+"}, this});
	m_lmNumberLineEdit->setText(QString::number(actuatorHeader->lmNumber()));
	m_layout->addWidget(m_lmNumberLineEdit, row, 1);

	m_lmNumberHelpLabel = new QLabel{tr("0 - %1").arg(VFrame30::ActuatorHeader::maxLmNumber()), this};
	m_layout->addWidget(m_lmNumberHelpLabel, row, 2);

	// Next row
	//
	row++;

	m_buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
	m_layout->addWidget(m_buttonBox, row, 0, 1, 3, Qt::AlignBottom);

	setLayout(m_layout);

	// --
	//
	connect(m_actuatorTypeIdLineEdit, &QLineEdit::textChanged, this, &CreateActuatorDialog::validate);
	connect(m_acmPresetNameComboBox, &QComboBox::currentTextChanged, this, &CreateActuatorDialog::validate);
	connect(m_acmPresetNameComboBox, &QComboBox::currentTextChanged, this, &CreateActuatorDialog::newPresetNameIsSet);
	connect(m_moduleDescriptionFileLineEdit, &QLineEdit::textChanged, this, &CreateActuatorDialog::validate);
	connect(m_lmNumberLineEdit, &QLineEdit::textChanged, this, &CreateActuatorDialog::validate);
	connect(m_subsystemIdComboBox, &QComboBox::currentTextChanged, this, &CreateActuatorDialog::validate);

	connect(m_subsystemsButton, &QPushButton::clicked, this, &CreateActuatorDialog::subsystemsClicked);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	validate();
	return;
}

CreateActuatorDialog::~CreateActuatorDialog()
{
}

void CreateActuatorDialog::getAcmPresets()
{
	m_presets.clear();
	m_subsystemIds.clear();

	// Get All presets
	//
	std::vector<DbFileInfo> presetFileInfos;
	bool ok = m_db->getFileList(&presetFileInfos, DbDir::HardwarePresetsDir, true, this);
	if (ok == false) 
	{
		return;
	}
	
	std::vector<std::shared_ptr<DbFile>> presetFiles;
	ok = m_db->getLatestVersion(presetFileInfos, &presetFiles, this);
	if (ok == false)
	{
		return;
	}

	// Parse presets
	//
	m_presets.reserve(presetFiles.size());

	for (const auto file : presetFiles)
	{
		auto preset = Hardware::DeviceObject::Create(file->data());
		if (preset != nullptr &&                                                             //
			preset->isPreset() == true &&                                                    //
			preset->presetRoot() == true &&                                                  //
			preset->isModule() == true &&                                                    //
			preset->toModule()->moduleFamily() == Hardware::DeviceModule::FamilyType::ACM && //
			preset->presetName().isEmpty() == false)
		{
			m_presets.push_back(preset->toModule());
		}
	}

	// Also get subsystem IDs
	//
	{
		Builder::SubsystemStorage subsystemStorage;
		QString errorMessage;

		ok = subsystemStorage.load(m_db, errorMessage);
		if (ok == true)
		{
			std::transform(subsystemStorage.begin(),
						   subsystemStorage.end(),
						   std::back_inserter(m_subsystemIds),
						   [](const auto& subsystem)
						   {
							   return subsystem->subsystemId();
						   });
		}
	}

	return;
}

QString CreateActuatorDialog::getModuleDescriptionFile(const QString& acmPresetName) const 
{
	QString result;

	auto it = std::find_if(m_presets.begin(),
						   m_presets.end(),
						   [acmPresetName](const auto& preset)
						   {
							   return preset->presetName() == acmPresetName;
						   });
	if (it != m_presets.end() && *it != nullptr)
	{
		assert(*it);
		auto module = *it;

		auto v = module->propertyValue(Hardware::PropertyNames::lmDescriptionFile);
		if (v.isValid() == true && v.canConvert<QString>() == true)
		{
			result = v.toString();
		}
	}
	
	return result;
}

void CreateActuatorDialog::validate() 
{
	// Validate ActuatorTypeId
	//
	if (m_actuatorTypeIdLineEdit->text().isEmpty() == true)
	{
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	// Validate Caption
	//
	if (m_captionLineEdit->text().isEmpty() == true)
	{
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	// Validate ACM Preset
	//
	if (m_acmPresetNameComboBox->currentText().isEmpty() == true)
	{
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	// Validate DescriptionFile
	//
	if (m_moduleDescriptionFileLineEdit->text().isEmpty() == true)
	{
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	// Validate SubsystemId (non-empty)
	//
	if (m_subsystemIdComboBox->currentText().isEmpty() == true)
	{
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	// Validate LmNumber (0-ActuatorHeader::maxLmNumber())
	//
	{
		if (m_lmNumberLineEdit->text().isEmpty() == false)
		{
			bool ok = false;
			int lmNumber = m_lmNumberLineEdit->text().toInt(&ok);
			if (ok == false || lmNumber < 0 || lmNumber > VFrame30::ActuatorHeader::maxLmNumber())
			{
				m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
				return;
			}
		}
		else
		{
			m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
			return;
		}
	}

	// All validations passed
	//
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	return;
}

void CreateActuatorDialog::newPresetNameIsSet(const QString& newPresetName) 
{
	auto descriptionFile = getModuleDescriptionFile(newPresetName);
	if (descriptionFile.isEmpty() == false)
	{
		m_moduleDescriptionFileLineEdit->setText(descriptionFile);
	}

	return;
}

void CreateActuatorDialog::subsystemsClicked()
{
	if (m_db->isProjectOpened() == false)
	{
		return;
	}

	DialogSubsystemListEditor dialog{m_db, this};
	auto result = dialog.exec();
	if (result == QDialog::Accepted)
	{
		// Update subsystem list in case it was changed.
		//
		getAcmPresets();
		
		QString currentSubsystemId = m_subsystemIdComboBox->currentText();
		m_subsystemIdComboBox->clear();
		m_subsystemIdComboBox->addItems(m_subsystemIds);
		m_subsystemIdComboBox->setCurrentText(currentSubsystemId);
	}
	 
	return;
}

void CreateActuatorDialog::accept()
{
	m_actuatorHeader->setActuatorTypeId(m_actuatorTypeIdLineEdit->text());
	m_actuatorHeader->setCaption(m_captionLineEdit->text());
	m_actuatorHeader->setAcmPresetName(m_acmPresetNameComboBox->currentText());
	m_actuatorHeader->setDescriptionFile(m_moduleDescriptionFileLineEdit->text());
	m_actuatorHeader->setSubsystemId(m_subsystemIdComboBox->currentText());
	
	{
		bool ok = false;
		int lmNumber = m_lmNumberLineEdit->text().toInt(&ok);
		if (ok == true)
		{
			m_actuatorHeader->setLmNumber(lmNumber);
		}
	}

	QDialog::accept();
	return;
}

void CreateActuatorDialog::reject()
{
	QDialog::reject();
	return;
}
#include "CreateSignalDialog.h"
#include "../DbLib/DbController.h"
#include "BusStorage.h"
#include "SignalsTabPage.h"
#include "SignalPropertiesDialog.h"
#include "../Builder/AppSignalSetProvider.h"

void CreatingSignalDialogOptions::init(QString schemaId, QString schemaCaption, QStringList equipmentIds, QStringList proposedAppSignalIds)
{
	m_schemaId = schemaId;
	m_schemaCaption = schemaCaption;
	m_equipmentIds = equipmentIds;
	m_proposedAppSignalIds = proposedAppSignalIds;

	return;
}

CreateSignalDialog::CreateSignalDialog(DbController* dbc, CreatingSignalDialogOptions* options, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_dbc(dbc),
	m_options(*options)
{
	assert(dbc);

	setWindowTitle(tr("Add AppSignal(s)"));

	// Creatig items
	//

	// Equipment CheckBoxes, AppSignalIDs/CustomSignalIDs edit controls
	//
	for (QString eid : m_options.m_equipmentIds)
	{
		QCheckBox* checkBox = new QCheckBox(eid, this);
		m_equipmentCheckBoxes.push_back(checkBox);

		QLineEdit* appSiganalIdEdit = new QLineEdit;
		appSiganalIdEdit->setPlaceholderText(tr("#APPSIGNALID"));
		appSiganalIdEdit->setValidator(new QRegExpValidator(QRegExp(AppSignal::IDENTIFICATORS_VALIDATOR), this));
		m_appSiganalIds.push_back(appSiganalIdEdit);

		QLineEdit* customSiganalIdEdit = new QLineEdit;
		customSiganalIdEdit->setPlaceholderText(tr("SIGNALID"));
		customSiganalIdEdit->setValidator(new QRegExpValidator(QRegExp(AppSignal::IDENTIFICATORS_VALIDATOR), this));
		m_customSiganalIds.push_back(customSiganalIdEdit);
	}

	if (m_options.m_lastEquipmentIds.isEmpty() == true)
	{
		// Select all EquipmentIDs
		//
		for (QCheckBox* cb : m_equipmentCheckBoxes)
		{
			cb->setChecked(true);
		}
	}
	else
	{
		for (QString eid: m_options.m_lastEquipmentIds)
		{
			for (QCheckBox* cb : m_equipmentCheckBoxes)
			{
				if (cb->text() == eid)
				{
					cb->setChecked(true);
					break;
				}
			}
		}
	}

	for (QCheckBox* ecb : m_equipmentCheckBoxes)
	{
		assert(ecb);
		connect(ecb, &QCheckBox::toggled, this, &CreateSignalDialog::equipmentIdToggled);
	}

	// Signal Type controls
	//
	m_signalTypeDiscrete = new QRadioButton(tr("Discrete"));
	m_signalTypeFloatingPoint = new QRadioButton(tr("Analog Float32"));
	m_signalTypeSignedInteger = new QRadioButton(tr("Analog SignedInt32"));
	m_signalTypeBus = new QRadioButton(tr("Bus"));
	connect(m_signalTypeBus, &QRadioButton::toggled, this, &CreateSignalDialog::busTypeToggled);

	m_signalTypeRadios.push_back({CreatingSignalDialogOptions::SignalTypeAndFormat::Discrete, m_signalTypeDiscrete});
	m_signalTypeRadios.push_back({CreatingSignalDialogOptions::SignalTypeAndFormat::AnalogFloat32, m_signalTypeFloatingPoint});
	m_signalTypeRadios.push_back({CreatingSignalDialogOptions::SignalTypeAndFormat::AnalogSignedInt32, m_signalTypeSignedInteger});
	m_signalTypeRadios.push_back({CreatingSignalDialogOptions::SignalTypeAndFormat::Bus, m_signalTypeBus});

	m_busTypeCombo = new QComboBox;
	m_busTypeCombo->setEnabled(false);
	m_busTypeCombo->setEditable(true);

	for (auto& p : m_signalTypeRadios)
	{
		if (p.first == m_options.m_lastSignalType)
		{
			p.second->setChecked(true);
			break;
		}
	}

	initBusTypes();
	initSignalIds();

	// Ok/Cancel buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	m_generateNewIdsButton = new QPushButton("New ID(s)");
	m_buttonBox->addButton(m_generateNewIdsButton, QDialogButtonBox::ResetRole);

	connect(m_generateNewIdsButton, &QAbstractButton::clicked, this, &CreateSignalDialog::generateNewSignalIds);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	// Set layout
	//
	QGridLayout* layout = new QGridLayout(this);

	layout->addWidget(new QLabel("EquipmentID(s)"), 0, 0, 1, 1);
	layout->addWidget(new QLabel("AppSignalID(s)"), 0, 1, 1, 1);
	layout->addWidget(new QLabel("CustomSignalID(s)"), 0, 2, 1, 1);

	int row = 1;
	for (QCheckBox* cb : m_equipmentCheckBoxes)
	{
		layout->addWidget(cb, row, 0);
		row++;
	}

	row = 1;
	for (QLineEdit* e : m_appSiganalIds)
	{
		layout->addWidget(e, row, 1);
		row++;
	}

	row = 1;
	for (QLineEdit* e : m_customSiganalIds)
	{
		layout->addWidget(e, row, 2);
		row++;
	}

	QGroupBox* signalTypeGroup = new QGroupBox(tr("Signal Type"));
	signalTypeGroup->setFlat(true);
	layout->addWidget(signalTypeGroup, row++, 0, 1, 3);

	layout->addWidget(m_signalTypeDiscrete, row++, 0, 1, 3);
	layout->addWidget(m_signalTypeFloatingPoint, row++, 0, 1, 3);
	layout->addWidget(m_signalTypeSignedInteger, row++, 0, 1, 3);
	layout->addWidget(m_signalTypeBus, row++, 0, 1, 3);

	layout->addWidget(m_busTypeCombo, row++, 0, 1, 1);

	QWidget* stretch = new QWidget;
	stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Ok/Cancel
	//
	layout->addWidget(stretch, row++, 0, 1, 3);

	QGroupBox* okCancelGroupBox = new QGroupBox;
	okCancelGroupBox->setFlat(true);
	layout->addWidget(okCancelGroupBox, row++, 0, 1, 3);

	layout->addWidget(m_buttonBox, row++, 0, 1, 3);

	setLayout(layout);

	// --
	//
	if (m_options.m_lastGeometry.isEmpty() == false)
	{
		restoreGeometry(m_options.m_lastGeometry);
	}
	else
	{
		resize(minimumSizeHint());
	}

	return;
}

void CreateSignalDialog::initSignalIds()
{
	if (m_options.m_proposedAppSignalIds.isEmpty() == true)
	{
		generateNewSignalIds();
		return;
	}

	size_t signalFieldIndex = 0;

	for (QString id  : m_options.m_proposedAppSignalIds)
	{
		// Set proposed signal ids only to checked modules
		//
		while (signalFieldIndex < m_equipmentCheckBoxes.size())
		{
			if (m_equipmentCheckBoxes[signalFieldIndex]->isChecked() == true)
			{
				break;
			}

			signalFieldIndex ++;
		}

		if (signalFieldIndex < m_appSiganalIds.size())
		{
			QString appSignalId = id;
			if (appSignalId.startsWith('#') == false)
			{
				appSignalId.prepend('#');
			}

			m_appSiganalIds[signalFieldIndex]->setText(appSignalId);
		}

		if (signalFieldIndex < m_customSiganalIds.size())
		{
			QString customSignalId = id;
			if (customSignalId.startsWith('#') == true)
			{
				customSignalId.remove(0, 1);
			}

			m_customSiganalIds[signalFieldIndex]->setText(customSignalId);
		}

		signalFieldIndex ++;
	}

	return;
}

void CreateSignalDialog::initBusTypes()
{
	assert(m_busTypeCombo);

	BusStorage busStorage(m_dbc);
	QString errorMessage;

	bool ok = busStorage.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this->parentWidget(), qAppName(), tr("Bus loading error: %1").arg(errorMessage));
		return;
	}

	for (int i = 0; i < busStorage.count(); i++)
	{
		const std::shared_ptr<VFrame30::Bus> bus = busStorage.get(i);
		m_busTypeCombo->addItem(bus->busTypeId());
	}

	if (m_options.m_lastBusTypeId.isEmpty() == true)
	{
		if (busStorage.count() != 0)
		{
			m_busTypeCombo->setEditText(busStorage.get(0)->busTypeId());
		}
		else
		{
			m_busTypeCombo->setEditText("BUSTYPEID");
		}
	}
	else
	{
		m_busTypeCombo->setEditText(m_options.m_lastBusTypeId);
	}

	m_busTypeCombo->model()->sort(0);

	return;
}

QStringList CreateSignalDialog::showDialog(DbController* dbc, CreatingSignalDialogOptions* options, QWidget* parent)
{
	if (dbc == nullptr ||
		options == nullptr)
	{
		Q_ASSERT(dbc);
		Q_ASSERT(options);
		return QStringList();
	}

	CreateSignalDialog d(dbc, options, parent);

	int dialogResult = d.exec();

	*options = d.options();

	if (dialogResult == QDialog::Rejected)
	{
		return QStringList();
	}

	// Generatuing Signal and show it's properties
	//
	QVector<AppSignal> newSignals;

	auto resultData = d.resultData();

	if (resultData.equipmentIds.size() != resultData.appSignalIds.size() ||
		resultData.equipmentIds.size() != resultData.customSignalIds.size())
	{
		Q_ASSERT(resultData.equipmentIds.size() == resultData.appSignalIds.size());
		Q_ASSERT(resultData.equipmentIds.size() == resultData.customSignalIds.size());
		QMessageBox::critical(parent,
							  qAppName(),
							  QString("CreateSignalDialog::showDialog(...)\nInternal Error: \n Q_ASSERT(resultData.equipmentIds.size() %1 == resultData.appSignalIds.size()) %2 ; or Q_ASSERT(resultData.equipmentIds.size() %1 == resultData.customSignalIds.size()) %3; \n ").arg(resultData.equipmentIds.size()).arg(resultData.appSignalIds.size()).arg(resultData.customSignalIds.size()));
		return {};
	}

	for (int index = 0; index < resultData.equipmentIds.size(); index ++)
	{
		QString equipmentId = resultData.equipmentIds[index];
		QString appSignalId = resultData.appSignalIds[index];
		QString customSignalId = resultData.customSignalIds[index];
		QString caption = QString("App signal %1 in schema %2").arg(appSignalId).arg(options->m_schemaId);

		AppSignal signal;

		switch (resultData.signalType)
		{
		case CreatingSignalDialogOptions::SignalTypeAndFormat::Discrete:
			signal.setSignalType(E::Discrete);
			signal.setDataSize(DISCRETE_SIZE);
			break;

		case CreatingSignalDialogOptions::SignalTypeAndFormat::AnalogFloat32:
			signal.setSignalType(E::Analog);
			signal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
			signal.setDataSize(FLOAT32_SIZE);
			break;

		case CreatingSignalDialogOptions::SignalTypeAndFormat::AnalogSignedInt32:
			signal.setSignalType(E::Analog);
			signal.setAnalogSignalFormat(E::AnalogAppSignalFormat::SignedInt32);
			signal.setDataSize(SIGNED_INT32_SIZE);
			break;

		case CreatingSignalDialogOptions::SignalTypeAndFormat::Bus:
			signal.setSignalType(E::Bus);
			signal.setBusTypeID(resultData.busTypeId);
			break;

		default:
			Q_ASSERT(false);
			QMessageBox::critical(parent,
								  qAppName(),
								  QString("CreateSignalDialog::showDialog(...)\nInternal Error: \nUnknown signal type: %1").arg(static_cast<int>(resultData.signalType)));
			return {};
		}

		initNewSignal(signal);

		if (resultData.signalType == CreatingSignalDialogOptions::SignalTypeAndFormat::Bus)
		{
			// Previous call initNewSignal spoils busTypeId
			// so I have to restore it here
			//
			signal.setBusTypeID(resultData.busTypeId);
		}

		signal.setAppSignalID(appSignalId);
		signal.setCustomAppSignalID(customSignalId);
		signal.setEquipmentID(equipmentId);
		signal.setCaption(caption);

		newSignals.push_back(signal);
	}

	if (newSignals.empty() == true)
	{
		return {};
	}

	// Show properties dialog
	//
	QVector<AppSignal*> signalPtrVector;

	for (AppSignal& signal : newSignals)
	{
		signalPtrVector.push_back(&signal);
	}

	SignalPropertiesDialog signalPropDialog(dbc, signalPtrVector, false, false, parent);

	dialogResult = signalPropDialog.exec();
	if (dialogResult != QDialog::Accepted)
	{
		return {};
	}

	for (AppSignal& signal : newSignals)
	{
		AppSignalSetProvider::trimSignalTextFields(signal);
	}

	bool ok = dbc->addSignal(newSignals.front().signalType(), &newSignals, parent);
	if (ok == false)
	{
		return {};
	}

	AppSignalSetProvider* model = AppSignalSetProvider::getInstance();
	model->loadSignals();

	QVector<int> selectIdList(newSignals.size());
	int currentIdIndex = 0;
	QStringList resultAppSignalIds;

	for (AppSignal& signal : newSignals)
	{
		resultAppSignalIds << signal.appSignalID();
		selectIdList[currentIdIndex++] = signal.ID();
	}

	return resultAppSignalIds;
}

CreateSignalDialogResult CreateSignalDialog::resultData() const
{
	assert(m_result.equipmentIds.size() == m_result.appSignalIds.size());
	assert(m_result.equipmentIds.size() == m_result.customSignalIds.size());

	return m_result;
}

void CreateSignalDialog::generateNewSignalIds()
{
	assert(m_appSiganalIds.size() == m_customSiganalIds.size());

	QString counter = QString("%1").
						arg(QString::number(m_dbc->nextCounterValue()), 6, '0');

	for (size_t i = 0; i < m_appSiganalIds.size(); i++)
	{
		QChar channelLetter = QChar('A' + static_cast<int>(i));

		QString appSignalId = m_appSiganalIds.size() == 1 ?
									QString("#%1_%2")
										.arg(m_options.m_schemaId)
										.arg(counter)
									:
									QString("#%1_%2_%3")
									  .arg(m_options.m_schemaId)
									  .arg(counter)
									  .arg(channelLetter);

		QString customSignalId = m_appSiganalIds.size() == 1 ?
									QString("%1_%2")
										.arg(m_options.m_schemaId)
										.arg(counter)
									:
									QString("%1_%2_%3")
									  .arg(m_options.m_schemaId)
									  .arg(counter)
									  .arg(channelLetter);
	\
		m_appSiganalIds[i]->setText(appSignalId);
		m_customSiganalIds[i]->setText(customSignalId);
	}

	return;
}

void CreateSignalDialog::equipmentIdToggled(bool /*checked*/)
{
	QCheckBox* checkBox =  qobject_cast<QCheckBox*>(sender());
	assert(checkBox);

	bool isAnyChecked = false;
	for (QCheckBox* ecb : m_equipmentCheckBoxes)
	{
		assert(ecb);
		isAnyChecked |= ecb->isChecked();
	}

	if (isAnyChecked == false)
	{
		checkBox->setChecked(true);
	}

	return;
}

void CreateSignalDialog::busTypeToggled(bool checked)
{
	assert(m_signalTypeBus);
	assert(m_busTypeCombo);

	m_busTypeCombo->setEnabled(checked);

	return;
}

void CreateSignalDialog::accept()
{
	// Saving geometry
	//
	m_options.m_lastGeometry = saveGeometry();

	// Saving selected EquipmentIDs
	//
	m_options.m_lastEquipmentIds.clear();
	for (QCheckBox* cb : m_equipmentCheckBoxes)
	{
		if (cb->isChecked() == true)
		{
			m_options.m_lastEquipmentIds.push_back(cb->text());
		}
	}

	// Saving selected SignalType
	//
	for (auto& p : m_signalTypeRadios)
	{
		assert(p.second);

		if (p.second->isChecked() == true)
		{
			m_options.m_lastSignalType = p.first;
			break;
		}
	}

	if (m_options.m_lastSignalType == CreatingSignalDialogOptions::SignalTypeAndFormat::Bus)
	{
		m_options.m_lastBusTypeId = m_busTypeCombo->currentText();
	}

	// Saving result: signal type
	//
	for (auto& p : m_signalTypeRadios)
	{
		if (p.second->isChecked() == true)
		{
			m_result.signalType = p.first;
			break;
		}
	}

	// Saving result: EquipmentIDs, AppSignalIDs, CustomSignalIDs
	//
	size_t index = 0;
	for (QCheckBox* ecb : m_equipmentCheckBoxes)
	{
		if (ecb->isChecked() == true)
		{
			m_result.equipmentIds.push_back(ecb->text());

			m_result.appSignalIds.push_back(m_appSiganalIds[index]->text().trimmed());
			m_result.customSignalIds.push_back(m_customSiganalIds[index]->text().trimmed());

			if (m_result.appSignalIds.back().isEmpty() == true)
			{
				QMessageBox::critical(this, qAppName(), tr("AppSignalID is empty."));
				m_appSiganalIds[index]->setFocus();
				return;
			}

			if (m_result.customSignalIds.back().isEmpty() == true)
			{
				QMessageBox::critical(this, qAppName(), tr("CustomSignalID is empty."));
				m_customSiganalIds[index]->setFocus();
				return;
			}
		}

		index ++;
	}

	// Saving BusTypeID
	//
	m_result.busTypeId = m_busTypeCombo->currentText().trimmed();

	// Checks
	//
	if (m_result.signalType == CreatingSignalDialogOptions::SignalTypeAndFormat::Bus &&
		m_result.busTypeId.isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("BusTypeID is empty. You can set it later in Signal Properties."));
	}

	QStringList tempString = m_result.appSignalIds;
	int duplicates = tempString.removeDuplicates();

	if (duplicates != 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Some AppSignalIDs have duplicate values."));
		return;
	}

	tempString = m_result.customSignalIds;
	duplicates = tempString.removeDuplicates();

	if (duplicates != 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Some CustomSignalIDs have duplicate values."));
		return;
	}

	// --
	//
	QDialog::accept();
	return;
}

const CreatingSignalDialogOptions& CreateSignalDialog::options() const
{
	return m_options;
}

#include <UiLib/PropertyEditor.h>
#include "../UtilsLib/Ui/WidgetUtils.h"
#include "../UtilsLib/WUtils.h"
#include "SignalPropertiesDialog.h"
#include "AppSettings.h"

SignalPropertiesDialog::SignalPropertiesDialog(const std::vector<AppSignal*>& signalsToEdit,
											   bool readOnly, bool isExistSignals, QWidget* parent) :
	QDialog(parent),
	m_signalsToEdit(signalsToEdit),
	m_readOnly(readOnly),
	m_isExistSignals(isExistSignals),
	m_signalSetProvider(AppSignalSetProvider::getInstance()),
	m_propManager(AppSignalPropertyManager::getInstance())
{
	TEST_PTR_RETURN(m_signalSetProvider);
	TEST_PTR_RETURN(m_propManager);

	if (m_signalsToEdit.size() == 0 ||
		CONTAINS_NULLPTR(m_signalsToEdit))
	{
		Q_ASSERT(false);
		return;
	}

	m_uppercaseAppSignalID = m_signalSetProvider->projectProperty_uppercaseAppSignalID();

	if (m_uppercaseAppSignalID == true && m_readOnly == false)
	{
		uppercaseAppSignalIDs();
	}

	createSignalsProps();

	// Dialog controls creation
	QVBoxLayout* vl = new QVBoxLayout;

	m_propertyEditor = new IdePropertyEditor(this, m_signalSetProvider->dbController());

	m_propertyEditor->setExpertMode(theAppSettings.isExpertMode());
	m_propertyEditor->setObjects(m_signalsProps);
	m_propertyEditor->autoAdjustSplitterPosition();

	vl->addWidget(m_propertyEditor);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::NoButton, this);

	if (m_readOnly == true)
	{
		setDialogReadOnly();
	}
	else
	{
		setDialogEditable();
	}

	vl->addWidget(m_buttonBox);
	setLayout(vl);

	//

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::onSignalsPropChanged);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	connect(this, &QDialog::accepted, this, &SignalPropertiesDialog::onOk);
	connect(this, &QDialog::rejected, this, &SignalPropertiesDialog::onCancel);

	setWindowPosition(this, "SignalPropertiesDialog");

	m_isValid = true;
}

bool SignalPropertiesDialog::isEditedSignal(int id) const
{
	return m_editedSignalsId.contains(id);
}

bool SignalPropertiesDialog::hasEditedSignals() const
{
	return m_editedSignalsId.empty() == false;
}

bool SignalPropertiesDialog::isValid() const
{
	return m_isValid;
}

void SignalPropertiesDialog::initNewSignal(AppSignal& signal)
{
	bool expertMode = theAppSettings.isExpertMode();

	signal.setInOutType(E::SignalInOutType::Internal);
	signal.setByteOrder(E::ByteOrder::BigEndian);
	signal.setDataSizeByType(signal.signalType(), signal.analogSignalFormat());
	signal.setAnalogSignalFormat(signal.analogSignalFormat());

	AppSignalPropertyManager* propManager = AppSignalPropertyManager::getInstance();

	auto setter = [&signal, &propManager, expertMode](const QString& name, QVariant value)
	{
		int index = propManager->propertyIndex(name);
		if (index == -1)
		{
			return;
		}

		if (propManager->getBehaviour(signal, index) == E::PropertyBehaviourType::Write)
		{
			propManager->setValue(&signal, index, value, expertMode);
		}
	};

	signal.initSpecificProperties();

	switch (signal.signalType())
	{
	case E::SignalType::Analog:
		setter(AppSignalPropNames::LOW_ENGINEERING_UNITS, 0.0);
		setter(AppSignalPropNames::HIGH_ENGINEERING_UNITS, 100.0);
		break;

	case E::SignalType::Discrete:
	case E::SignalType::Bus:
		break;

	default:
		Q_ASSERT(false);
	}

	QSettings settings;
	QString propKeyPrefix = AppSignalProperties::lastEditedSignalPropsPrefix(signal);

	for (int i = 0; i < propManager->count(); i++)
	{
		if (propManager->getBehaviour(signal, i) != E::PropertyBehaviourType::Write)
		{
			continue;
		}

		QString propName = propManager->name(i);

		QVariant value = settings.value(propKeyPrefix + propName, QVariant());

		if (value.isValid() == false)
		{
			continue;
		}

		QVariant propertyManagerValue = propManager->value(&signal, i, expertMode);
		QMetaType type = propertyManagerValue.metaType();

		if (type.id() == QMetaType::QString && propertyManagerValue.toString().isEmpty() == false)
		{
			continue;
		}

		if (value.canConvert(type) && value.convert(type))
		{
			propManager->setValue(&signal, i, value, expertMode);
		}
	}

	signal.initTuningValues();
}

// Returns vector of pairs,
//	first: previous AppSignalID
//  second: new AppSignalID
//
std::vector<std::pair<QString, QString>> SignalPropertiesDialog::editApplicationSignals(QStringList& signalId,
																						DbController* dbController,
																						QWidget* parent)
{
	for (QString& id : signalId)
	{
		id = id.trimmed();
	}

	std::vector<AppSignal> signalVector;

	if (!dbController->getLatestSignalsByAppSignalIDs(signalId, &signalVector, parent))
	{
		QMessageBox::critical(parent, "Error", "Could not load signal(s) from database");
		return {};
	}

	std::vector<AppSignal*> signalPtrVector;
	std::vector<QString> initialAppSignalID;

	for (AppSignal& signal : signalVector)
	{
		if (signalId.contains(signal.appSignalID()) == true)
		{
			signalPtrVector.push_back(&signal);
			initialAppSignalID.emplace_back(signal.appSignalID());
		}
	}

	if (signalPtrVector.empty() == true)
	{
		QMessageBox::critical(parent, "Error", "Could not find signal(s) in database");
		return {};
	}

	AppSignalSetProvider* signalSetProvider = AppSignalSetProvider::getInstance();

	int currentUserID = signalSetProvider->currentUserID();
	bool currentUserIsAdmin = signalSetProvider->currentUserIsAdmin();

	int readOnly = false;

	for (AppSignal* signal : signalPtrVector)
	{
		if (signal->checkedOut() &&
			signal->userID() != currentUserID &&
			currentUserIsAdmin == false)
		{
			readOnly = true;
		}
	}

	SignalPropertiesDialog dlg(signalPtrVector, readOnly, true, parent);

	if(dlg.isValid() == false ||
	   dlg.exec() != QDialog::Accepted)
	{
		return {};
	}

	std::vector<AppSignal*> editedSignals;
	std::vector<std::pair<QString, QString>> result;

	result.reserve(signalPtrVector.size());

	int i = 0;

	for (AppSignal* s : signalPtrVector)
	{
		if (dlg.isEditedSignal(s->ID()) == true)
		{
			editedSignals.push_back(s);
			result.emplace_back(initialAppSignalID[i], s->appSignalID());
		}

		i++;
	}

	bool saveOk = signalSetProvider->saveSignals(editedSignals, parent);
	if (saveOk == false) 
	{
		// TODO: Report and return which specific signals were changed and which were not.
		result.clear();
	}

	return result;
}


void SignalPropertiesDialog::onOk()
{
	checkAndSaveSignal();
	saveLastEditedSignalProperties();

	saveDialogSettings();
}

void SignalPropertiesDialog::onCancel()
{
	undoCheckouts();

	saveDialogSettings();
}

bool SignalPropertiesDialog::checkAndSaveSignal()
{
	// Check AppSignalID
	//
	bool res = true;

	for(auto object : m_signalsProps)
	{
		auto signalProps = dynamic_cast<AppSignalProperties*>(object.get());

		TEST_PTR_CONTINUE(signalProps);

		AppSignal& editedSignal = signalProps->signal();

		editedSignal.trimTextFields();

		if (editedSignal.appSignalID().isEmpty() ||
			editedSignal.appSignalID() == QStringLiteral("#"))
		{
			QMessageBox::critical(this, "Error", "AppSignalID is empty!");
			res = false;
			break;
		}
	}

	RETURN_IF_FALSE(res);

	Q_ASSERT(m_signalsToEdit.size() == m_signalsProps.size());

	// Save changes from propertyObjects array to m_signalsToEdit array
	//
	for (int i = 0; i < m_signalsProps.size(); i++)
	{
		AppSignalProperties* signalProps = dynamic_cast<AppSignalProperties*>(m_signalsProps[i].get());

		TEST_PTR_CONTINUE(signalProps);

		signalProps->updateSpecPropValues();

		AppSignal& editedSignal = signalProps->signal();

		bool edited = false;

		// here: editedSignal.appSignalID() is NOT empty and is NOT "#" only

		if (editedSignal.appSignalID()[0] != '#')
		{
			editedSignal.setAppSignalID("#" + editedSignal.appSignalID());
			edited = true;
		}

		if (m_uppercaseAppSignalID == true)
		{
			QString upper = editedSignal.appSignalID().toUpper();

			if (editedSignal.appSignalID() != upper)
			{
				editedSignal.setAppSignalID(upper);
				edited = true;
			}
		}

		if (editedSignal.customAppSignalID().isEmpty())
		{
			editedSignal.setCustomAppSignalID(editedSignal.appSignalID().mid(1));
			edited = true;
		}

		if (!editedSignal.customAppSignalID().isEmpty() &&
			editedSignal.customAppSignalID()[0] == '#')
		{
			editedSignal.setCustomAppSignalID(editedSignal.customAppSignalID().mid(1));
			edited = true;
		}

		if (editedSignal.caption().isEmpty())
		{
			editedSignal.setCaption("Signal " + editedSignal.customAppSignalID());
			edited = true;
		}

		if (edited == true)
		{
			m_editedSignalsId.emplace(editedSignal.ID());
		}

		if (isEditedSignal(editedSignal.ID()) && m_isExistSignals)
		{
			emit signalChanged(editedSignal.ID(), true);
		}

		*m_signalsToEdit[i] = editedSignal;
	}

	return res;
}

void SignalPropertiesDialog::undoCheckouts()
{
	if (m_checkedOutSignalsId.empty() || m_isExistSignals == false)
	{
		return;
	}

	std::vector<int> undoSignalIDs(m_checkedOutSignalsId.begin(), m_checkedOutSignalsId.end());

	m_checkedOutSignalsId.clear();

	m_signalSetProvider->undoSignalsChanges(undoSignalIDs);
}

void SignalPropertiesDialog::saveDialogSettings()
{
	saveWindowPosition(this, "SignalPropertiesDialog");
}

void SignalPropertiesDialog::onSignalsPropChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_readOnly == false && m_isExistSignals == true && m_firstPropChange == true)
	{
		checkoutSignals(objects);
		m_firstPropChange = false;
	}

	limitPropsPrecisionOnPropChanged(objects);
}

void SignalPropertiesDialog::checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects)
{
	bool setReadOnly = false;

	for (auto& object : objects)
	{
		AppSignalProperties* signalProps = dynamic_cast<AppSignalProperties*>(object.get());

		TEST_PTR_CONTINUE(signalProps);

		AppSignal& signal = signalProps->signal();

		if (signal.checkedOut() == false)
		{
			QString errMsg;

			if (checkoutSignal(signal, &errMsg) == false)
			{
				if (errMsg.isEmpty() == false)
				{
					showError(errMsg);
				}

				setReadOnly = true;
				break;
			}
		}
		else
		{
			if (m_signalSetProvider->isEditableSignal(&signal) == false)
			{
				setReadOnly = true;
				break;
			}
		}

		m_editedSignalsId.insert(signal.ID());
	}

	if (setReadOnly == true)
	{
		setDialogReadOnly();
	}
}

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	if (m_signalsToEdit.size() < 1)
	{
		return;
	}

	QSettings settings;
	bool expertMode = theAppSettings.isExpertMode();

	AppSignalPropertyManager& manager = *AppSignalPropertyManager::getInstance();

	const AppSignal& signal = *m_signalsToEdit[0];

	QString propKeyPrefix = AppSignalProperties::lastEditedSignalPropsPrefix(signal);

	for (int i = 0; i < manager.count(); i++)
	{
		if (manager.isHidden(manager.getBehaviour(signal, i), expertMode))
		{
			continue;
		}

		QString propName = manager.name(i);

		settings.setValue(propKeyPrefix + propName, manager.value(&signal, i, expertMode));
	}
}

void SignalPropertiesDialog::showError(const QString& errMsg)
{
	if (!errMsg.isEmpty())
	{
		QMessageBox::warning(this, "Error", errMsg);
	}
}

void SignalPropertiesDialog::uppercaseAppSignalIDs()
{
	Q_ASSERT(m_uppercaseAppSignalID == true);
	Q_ASSERT(m_readOnly == false);

	QString errMsg;

	std::vector<std::pair<AppSignal*, QString>> changedIDs;		// AppSignal* => previousAppSignalID

	changedIDs.reserve(m_signalsToEdit.size());

	for (AppSignal* s : m_signalsToEdit)
	{
		TEST_PTR_CONTINUE(s);

		QString upperAppSignalId = s->appSignalID().toUpper();

		if (s->appSignalID() == upperAppSignalId)
		{
			continue;
		}

		if (m_isExistSignals == true)
		{
			bool checkOutResult = checkoutSignal(*s, &errMsg);

			if (checkOutResult == false)
			{
				showError(errMsg);
				setDialogReadOnly();
				break;
			}
		}

		changedIDs.emplace_back(s, s->appSignalID());		// save previous AppSignalID

		s->setAppSignalID(upperAppSignalId);

		m_editedSignalsId.insert(s->ID());
	}

	if (m_readOnly == true)
	{
		// rollback changes
		//
		for(const auto& [s, prevAppSignalID] : changedIDs)
		{
			s->setAppSignalID(prevAppSignalID);
			m_editedSignalsId.erase(s->ID());
		}
	}
}

void SignalPropertiesDialog::createSignalsProps()
{
	bool expertMode = theAppSettings.isExpertMode();

	for (AppSignal* s : m_signalsToEdit)
	{
		TEST_PTR_CONTINUE(s);

		std::shared_ptr<AppSignalProperties> signalProps = std::make_shared<AppSignalProperties>(*s, true);

		if (m_readOnly == true)
		{
			signalProps->setReadOnly(true);
		}

		m_propManager->detectNewProperties(*s);

		for (auto& property : signalProps->properties())
		{
			if (property->isCategorized() == false)
			{
				continue;
			}

			int propertyIndex = m_propManager->propertyIndex(property->caption());

			if (propertyIndex == -1)
			{
				Q_ASSERT(false);
				continue;
			}

			if (m_propManager->dependsOnPrecision(propertyIndex))
			{
				m_propsWithPrecision.emplace(property->caption());

				property->setPrecision(s->decimalPlaces());
			}

			E::PropertyBehaviourType behaviour = m_propManager->getBehaviour(*s, propertyIndex);

			if (m_propManager->isHidden(behaviour, expertMode))
			{
				property->setVisible(false);
			}

			if (behaviour == E::PropertyBehaviourType::Read)
			{
				property->setReadOnly(true);
			}
		}

		m_signalsProps.push_back(signalProps);
	}
}

bool SignalPropertiesDialog::checkoutSignal(AppSignal& s, QString* message)
{
	if (m_checkedOutSignalsId.contains(s.ID()))
	{
		return true;
	}

	std::vector<int> checkedOutIds;

	bool checkoutResult = m_signalSetProvider->checkoutSignal(&s, message, &checkedOutIds);

	if (checkoutResult == true)
	{
		// update signal state in properties
		//
		s.setCheckedOut(true);

		const AppSignal* chs = m_signalSetProvider->getSignalByID(s.ID());

		if (chs != nullptr)
		{
			s.setUserID(chs->userID());
		}

		m_checkedOutSignalsId.insert(checkedOutIds.begin(), checkedOutIds.end());
	}

	return checkoutResult;
}

void SignalPropertiesDialog::limitPropsPrecisionOnPropChanged(const QList<std::shared_ptr<PropertyObject>>& objects)
{
	for (std::shared_ptr<PropertyObject> object : objects)
	{
		AppSignalProperties* signalProps = dynamic_cast<AppSignalProperties*>(object.get());

		TEST_PTR_CONTINUE(signalProps);

		m_editedSignalsId.emplace(signalProps->signal().ID());

		//signalProperties->updateSpecPropValues();

		int precision = signalProps->getPrecision();

		if (precision == -1)
		{
			continue;			// signal hasn't DecimalPlaces property
		}

		for (const QString& propWithPrecision : m_propsWithPrecision)
		{
			std::shared_ptr<Property> prop = signalProps->propertyByCaption(propWithPrecision);

			if (prop == nullptr)
			{
				continue;					// is not an error, signal hasn't propWithPrecision
			}

			prop->setPrecision(precision);

			if (m_propertyEditor->isPropertyExists(prop->caption()) == true)
			{
				m_propertyEditor->updatePropertyValue(prop->caption());
			}
		}
	}
}

void SignalPropertiesDialog::setDialogEditable()
{
	m_readOnly = false;

	setWindowTitle(QStringLiteral("Signal properties (editing)"));
	m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
}

void SignalPropertiesDialog::setDialogReadOnly()
{
	m_readOnly = true;

	setWindowTitle(QStringLiteral("Signal properties (read only)"));
	m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
}


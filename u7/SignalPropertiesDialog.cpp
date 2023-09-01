#include <QMessageBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include "SignalPropertiesDialog.h"
#include "SignalsTabPage.h"
#include "Settings.h"
#include "../lib/PropertyEditor.h"
#include "../DbLib/DbController.h"
#include "../AppSignalLib/AppSignal.h"
#include "../lib/WidgetUtils.h"
#include "AppSignalSetProvider.h"


SignalPropertiesDialog::SignalPropertiesDialog(const std::vector<AppSignal*>& signalVector,
											   bool readOnly, bool tryCheckout, QWidget* parent) :
	QDialog(parent),
	m_signalVector(signalVector),
	m_tryCheckout(tryCheckout),
	m_parent(parent)
{
	m_signalSetProvider = AppSignalSetProvider::getInstance();
	m_propManager = AppSignalPropertyManager::getInstance();

	m_uppercaseAppSignalID = m_signalSetProvider->projectProperty_uppercaseAppSignalID();

	//

	QVBoxLayout* vl = new QVBoxLayout;

	m_propertyEditor = new IdePropertyEditor(this, m_signalSetProvider->dbController());

	m_propertyEditor->setExpertMode(theSettings.isExpertMode());

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::onSignalPropertyChanged);

	for (AppSignal* s : signalVector)
	{
		TEST_PTR_CONTINUE(s);

		AppSignal& appSignal = *s;

		if (m_uppercaseAppSignalID)
		{
			QString upperAppSignalId = appSignal.appSignalID().toUpper();

			if (appSignal.appSignalID() != upperAppSignalId)
			{
				QString message;

				bool checkOutResult = m_tryCheckout ? checkoutSignal(appSignal, &message) : true;

				if (readOnly == false && checkOutResult == false)
				{
					if (message.isEmpty() == false)
					{
						showError(message);
					}

					setWindowTitle("Signal properties (read only)");

					readOnly = true;
				}

				if (readOnly == false)
				{
					appSignal.setAppSignalID(upperAppSignalId);
					m_editedSignalsId.insert(appSignal.ID());
				}
			}
		}

		std::shared_ptr<AppSignalProperties> signalProperties = std::make_shared<AppSignalProperties>(appSignal, true);

		if (readOnly == true)
		{
			for (auto property : signalProperties->properties())
			{
				property->setReadOnly(true);
			}
		}

		int precision = appSignal.decimalPlaces();

		m_propManager->detectNewProperties(&appSignal);

		for (auto& property : signalProperties->properties())
		{
			if (property->isCategorized() == false)
			{
				continue;
			}

			int propertyIndex = m_propManager->propertyIndex(property->caption());

			Q_ASSERT(propertyIndex != -1);

			if (m_propManager->dependsOnPrecision(propertyIndex))
			{
				property->setPrecision(precision);
			}

			E::PropertyBehaviourType behaviour = m_propManager->getBehaviour(appSignal, propertyIndex);

			if (m_propManager->isHidden(behaviour, theSettings.isExpertMode()))
			{
				property->setVisible(false);
			}

			if (behaviour == E::PropertyBehaviourType::Read)
			{
				property->setReadOnly(true);
			}
		}

		m_objList.push_back(signalProperties);
	}

	m_propertyEditor->setObjects(m_objList);
	m_propertyEditor->autoAdjustSplitterPosition();
	vl->addWidget(m_propertyEditor);

	if (!readOnly)
	{
		setWindowTitle("Signal properties editing");
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::checkAndSaveSignal);
	}
	else
	{
		setWindowTitle("Signal properties (read only)");
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
		connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::reject);
	}

	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertiesDialog::rejectCheckoutProperty);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertiesDialog::reject);
	connect(this, &SignalPropertiesDialog::finished, this, &SignalPropertiesDialog::saveDialogSettings);

	vl->addWidget(m_buttonBox);
	setLayout(vl);

	setWindowPosition(this, "SignalPropertiesDialog");

	m_isValid = true;
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
	std::vector<QString> foundSignalID;

	for (AppSignal& signal : signalVector)
	{
		if (!signalId.contains(signal.appSignalID()))
		{
			continue;
		}

		foundSignalID.emplace_back(signal.appSignalID());
		signalPtrVector.push_back(&signal);
	}

	if (signalPtrVector.empty() == true)
	{
		QMessageBox::critical(parent, "Error", "Could not find signal(s) in database");
		return {};
	}

	int currentUserID = dbController->currentUser().userId();
	bool currentUserIsAdmin = dbController->currentUser().isAdminstrator();

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

	QString message;

	for (AppSignal* s : signalPtrVector)
	{
		if (dlg.isEditedSignal(s->ID()) == false)
		{
			continue;
		}

		ObjectState state;

		s->trimTextFields();

		dbController->setSignalWorkcopy(s, &state, parent);

		if (state.errCode != ERR_SIGNAL_OK)
		{
			switch(state.errCode)
			{
				case ERR_SIGNAL_IS_NOT_CHECKED_OUT:
				{
					message += QString("Signal %1 could not be checked out\n").arg(state.id);
					break;
				}
				case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER:
				{
					message += QString("Signal %1 is checked out by other user\n").arg(state.id);
					break;
				}
				case ERR_SIGNAL_DELETED:
				{
					message += QString("Signal %1 was deleted already\n").arg(state.id);
					break;
				}
				case ERR_SIGNAL_NOT_FOUND:
				{
					message += QString("Signal %1 not found\n").arg(state.id);
					break;
				}
				default:
				{
					message += QString("Unknown error %1\n").arg(state.errCode);
				}
			}
		}
	}

	if (!message.isEmpty())
	{
		QMessageBox::critical(parent, "Error", message);
	}

	std::vector<std::pair<QString, QString>> result;

	result.resize(signalPtrVector.size());

	for (int i = 0; i < signalPtrVector.size(); i++)
	{
		result[i].first = foundSignalID[i];
		result[i].second = signalPtrVector[i]->appSignalID();
	}

	return result;
}

void SignalPropertiesDialog::initNewSignal(AppSignal& signal)
{
	AppSignalPropertyManager* propManager = AppSignalPropertyManager::getInstance();

	auto setter = [&signal, &propManager](const QString& name, QVariant value) {
		int index = propManager->propertyIndex(name);
		if (index == -1)
		{
			return;
		}

		if (propManager->getBehaviour(signal, index) == E::PropertyBehaviourType::Write)
		{
			propManager->setValue(&signal, index, value, theSettings.isExpertMode());
		}
	};

	signal.initSpecificProperties();

	switch (signal.signalType())
	{
	case E::SignalType::Analog:
	{
		signal.setDataSize(FLOAT32_SIZE);
		setter(AppSignalPropNames::LOW_ENGINEERING_UNITS, 0.0);
		setter(AppSignalPropNames::HIGH_ENGINEERING_UNITS, 100.0);
		break;
	}

	case E::SignalType::Discrete:
	{
		signal.setDataSize(DISCRETE_SIZE);
		break;
	}

	case E::SignalType::Bus:
	default:
		break;
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

		QVariant propertyManagerValue = propManager->value(&signal, i, theSettings.isExpertMode());
		QMetaType type = propertyManagerValue.metaType();

		if (type.id() == QMetaType::QString && propertyManagerValue.toString().isEmpty() == false)
		{
			continue;
		}

		if (value.canConvert(type) && value.convert(type))
		{
			propManager->setValue(&signal, i, value, theSettings.isExpertMode());
		}
	}

	signal.initTuningValues();

	signal.setInOutType(E::SignalInOutType::Internal);
	signal.setByteOrder(E::ByteOrder::BigEndian);
}

void SignalPropertiesDialog::checkAndSaveSignal()
{
	// Check AppSignalID
	//
	for(auto object : m_objList)
	{
		auto signalProperties = dynamic_cast<AppSignalProperties*>(object.get());

		TEST_PTR_CONTINUE(signalProperties);

		AppSignal& signal = signalProperties->signal();

		if (signal.appSignalID().trimmed().isEmpty() == true)
		{
			QMessageBox::critical(this, "Error: Application signal ID is empty", "Fill Application signal ID");
			return;
		}
	}

	connect(this, &SignalPropertiesDialog::signalChanged, AppSignalSetProvider::getInstance(), &AppSignalSetProvider::loadSignal, Qt::QueuedConnection);

	// Save
	//
	for (qsizetype i = m_signalVector.size() - 1; i >= 0; i--)
	{
		AppSignal& signal = *m_signalVector[i];

		AppSignalProperties* signalProperties = dynamic_cast<AppSignalProperties*>(m_objList[i].get());

		TEST_PTR_CONTINUE(signalProperties);

		signalProperties->updateSpecPropValues();

		AppSignal& editedSignalCopy = signalProperties->signal();

		signal.setTags(editedSignalCopy.tagsSet());
		signal = editedSignalCopy;

		signal.setAppSignalID(signal.appSignalID().trimmed());

		if (signal.appSignalID().isEmpty() || signal.appSignalID()[0] != '#')
		{
			signal.setAppSignalID("#" + signal.appSignalID());
		}

		if (m_uppercaseAppSignalID)
		{
			signal.setAppSignalID(signal.appSignalID().toUpper());
		}

		signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());

		if (signal.customAppSignalID().isEmpty())
		{
			signal.setCustomAppSignalID(signal.appSignalID().mid(1));
		}

		if (!signal.customAppSignalID().isEmpty() && signal.customAppSignalID()[0] == '#')
		{
			signal.setCustomAppSignalID(signal.customAppSignalID().mid(1));
		}

		signal.setEquipmentID(signal.equipmentID().trimmed());

		if (signal.caption().isEmpty())
		{
			signal.setCaption("Signal " + signal.customAppSignalID());
		}

		if (isEditedSignal(signal.ID()) && m_tryCheckout)
		{
			emit signalChanged(signal.ID(), true);
		}
	}

	saveLastEditedSignalProperties();

	accept();
}

void SignalPropertiesDialog::rejectCheckoutProperty()
{
	std::vector<int> undoSignalIDs;

	for (std::shared_ptr<PropertyObject>& object : m_objList)
	{
		AppSignalProperties* signalProperites = dynamic_cast<AppSignalProperties*>(object.get());

		TEST_PTR_CONTINUE(signalProperites);

		int id = signalProperites->signalID();

		if (signalProperites->signalCheckedOut() && m_editedSignalsId.contains(id))
		{
			undoSignalIDs.push_back(id);
		}
	}

	m_signalSetProvider->undoSignalsChanges(undoSignalIDs);
}

void SignalPropertiesDialog::saveDialogSettings()
{
	saveWindowPosition(this, "SignalPropertiesDialog");
}

void SignalPropertiesDialog::onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects)
{
	if (m_tryCheckout == true)
	{
		checkoutSignals(objects);
	}

	for (std::shared_ptr<PropertyObject> object : objects)
	{
		AppSignalProperties* signalProperties = dynamic_cast<AppSignalProperties*>(object.get());

		if (signalProperties == nullptr)
		{
			continue;
		}

		//signalProperties->updateSpecPropValues();

		int precision = signalProperties->getPrecision();

		for (std::shared_ptr<Property> property : signalProperties->properties())
		{
			if (isPropertyDependentOnPrecision(property->caption()) == true)
			{
				property->setPrecision(precision);

				if (m_propertyEditor->isPropertyExists(property->caption()) == true)
				{
					m_propertyEditor->updatePropertyValue(property->caption());
				}
			}
		}
	}
}

void SignalPropertiesDialog::checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects)
{
	for (std::shared_ptr<PropertyObject> object : objects)
	{
		AppSignalProperties* signalProperites = dynamic_cast<AppSignalProperties*>(object.get());
		AppSignal& signal = signalProperites->signal();
		int id = signal.ID();

		if (signal.checkedOut())
		{
			m_editedSignalsId.insert(id);
			continue;
		}

		QString message;

		if (checkoutSignal(signal, &message) == false)
		{
			if (message.isEmpty() == false)
			{
				showError(message);
			}
			setWindowTitle("Signal properties (read only)");
			m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
			return;
		}

		m_editedSignalsId.insert(id);
	}
}

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	if (m_signalVector.size() < 1)
	{
		return;
	}

	QSettings settings;

	AppSignalPropertyManager& manager = *AppSignalPropertyManager::getInstance();

	const AppSignal& signal = *m_signalVector[0];

	QString propKeyPrefix = AppSignalProperties::lastEditedSignalPropsPrefix(signal);

	for (int i = 0; i < manager.count(); i++)
	{
		if (manager.isHidden(manager.getBehaviour(signal, i), theSettings.isExpertMode()))
		{
			continue;
		}

		QString propName = manager.name(i);

		settings.setValue(propKeyPrefix + propName, manager.value(&signal, i, theSettings.isExpertMode()));
	}
}

void SignalPropertiesDialog::showError(const QString& errorString)
{
	if (!errorString.isEmpty())
	{
		QMessageBox::warning(this, "Error", errorString);
	}
}

void SignalPropertiesDialog::closeEvent(QCloseEvent* event)
{
	saveDialogSettings();

	QDialog::closeEvent(event);
}

bool SignalPropertiesDialog::checkoutSignal(const AppSignal& s, QString* message)
{
	return m_signalSetProvider->checkoutSignal(&s, message);
}

bool SignalPropertiesDialog::isPropertyDependentOnPrecision(const QString& propName) const
{
	return m_propertiesDependentOnPrecision.contains(propName);
}

void SignalPropertiesDialog::addPropertyDependentOnPrecision(const QString& propName)
{
	m_propertiesDependentOnPrecision.emplace(propName);
}


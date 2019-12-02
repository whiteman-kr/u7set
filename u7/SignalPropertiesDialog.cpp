#include <QMessageBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include "SignalPropertiesDialog.h"
#include "SignalsTabPage.h"
#include "Settings.h"
#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/DbController.h"
#include "../lib/WidgetUtils.h"


// Returns vector of pairs,
//	first: previous AppSignalID
//  second: new AppSignalID
//
std::vector<std::pair<QString, QString>> editApplicationSignals(QStringList& signalId, DbController* dbController, QWidget* parent)
{
	QVector<Signal> signalVector;

	for (QString& id : signalId)
	{
		id = id.trimmed();
	}

	if (!dbController->getLatestSignalsByAppSignalIDs(signalId, &signalVector, parent))
	{
		if (signalId.count() > 1)
		{
			QMessageBox::critical(parent, "Error", "Could not load signals from database");
		}
		else
		{
			QMessageBox::critical(parent, "Error", "Could not load signal from database");
		}
	}

	QVector<Signal*> signalPtrVector;

	QStringList foundSignalID;

	for (Signal& signal : signalVector)
	{
		if (!signalId.contains(signal.appSignalID()))
		{
			continue;
		}
		foundSignalID.push_back(signal.appSignalID());
		signalPtrVector.push_back(&signal);
	}

	int readOnly = false;
	std::vector<std::pair<QString, QString>> result;

	for (Signal* signal : signalPtrVector)
	{
		if (signal->checkedOut() && signal->userID() != dbController->currentUser().userId() && !dbController->currentUser().isAdminstrator())
		{
			readOnly = true;
		}
	}

	if (signalPtrVector.isEmpty() == true)
	{
		if (signalId.count() > 1)
		{
			QMessageBox::critical(parent, "Error", "Could not find signals in database");
		}
		else
		{
			QMessageBox::critical(parent, "Error", "Could not find signal in database");
		}
		return result;
	}

	result.resize(signalPtrVector.count());

	SignalPropertiesDialog dlg(dbController, signalPtrVector, readOnly, true, parent);

	if(dlg.isValid() == false)
	{
		return result;
	}

	if (dlg.exec() == QDialog::Accepted)
	{
		QString message;
		for (int i = 0; i < signalPtrVector.count(); i++)
		{
			if (!dlg.isEditedSignal(signalPtrVector[i]->ID()))
			{
				continue;
			}
			ObjectState state;
			SignalsModel::trimSignalTextFields(*signalPtrVector[i]);
			dbController->setSignalWorkcopy(signalPtrVector[i], &state, parent);
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
	}
	else
	{
		return {};	// Cancel is pressed
	}

	for (int i = 0; i < signalPtrVector.count(); i++)
	{
		result[i].first = foundSignalID[i];
		result[i].second = signalPtrVector[i]->appSignalID();
	}
	return result;
}


void initNewSignal(Signal& signal)
{
	QSettings settings;
	auto loader = [&settings](const QString& name, QVariant defaultValue = QVariant())
	{
		return settings.value(SignalProperties::lastEditedSignalFieldValuePlace + name, defaultValue);
	};

	switch (signal.signalType())
	{
	case E::SignalType::Analog:
	{
		signal.setDataSize(FLOAT32_SIZE);
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

	signal.initSpecificProperties();

	SignalPropertyManager& propertyManager = SignalsModel::instance()->signalPropertyManager();

	auto setter = [&signal, &propertyManager](const QString& name, QVariant value) {
		int index = propertyManager.index(name);
		if (index == -1)
		{
			return;
		}

		if (propertyManager.getBehaviour(signal, index) == E::PropertyBehaviourType::Write)
		{
			propertyManager.setValue(&signal, index, value);
		}
	};

	setter(SignalProperties::lowEngineeringUnitsCaption, 0.0);
	setter(SignalProperties::highEngineeringUnitsCaption, 100.0);

	for (int i = 0; i < propertyManager.count(); i++)
	{
		if (propertyManager.getBehaviour(signal, i) != E::PropertyBehaviourType::Write)
		{
			continue;
		}

		QString name = propertyManager.name(i);
		QVariant value = settings.value(SignalProperties::lastEditedSignalFieldValuePlace + name, QVariant());
		if (value.isValid() == false)
		{
			continue;
		}

		QVariant::Type type = propertyManager.type(i);
		if (type == QVariant::String && propertyManager.value(&signal, i).toString().isEmpty() == false)
		{
			continue;
		}

		if (value.canConvert(type) && value.convert(type))
		{
			propertyManager.setValue(&signal, i, value);
		}
	}

	signal.initTuningValues();

	signal.setInOutType(E::SignalInOutType::Internal);
	signal.setByteOrder(E::ByteOrder::BigEndian);
}


SignalPropertiesDialog::SignalPropertiesDialog(DbController* dbController, QVector<Signal*> signalVector, bool readOnly, bool tryCheckout, QWidget *parent) :
	QDialog(parent),
	m_dbController(dbController),
	m_signalVector(signalVector),
	m_tryCheckout(tryCheckout),
	m_parent(parent)
{
	QVBoxLayout* vl = new QVBoxLayout;

	m_propertyEditor = new IdePropertyEditor(this);

	m_propertyEditor->setExpertMode(theSettings.isExpertMode());

	DbFileInfo mcInfo = dbController->systemFileInfo(dbController->etcFileId());

	if (mcInfo.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(Db::File::EtcFileName));
		return;
	}

	DbFileInfo propertyBehaviorFile;
	dbController->getFileInfo(mcInfo.fileId(), QString(Db::File::SignalPropertyBehaviorFileName), &propertyBehaviorFile, parent);

	if (propertyBehaviorFile.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	std::shared_ptr<DbFile> file;
	bool result = dbController->getLatestVersion(propertyBehaviorFile, &file, parent);
	QVector<QStringList> fileFields;
	if (result == true)
	{
		QString fileText = file->data();
		QStringList rows = fileText.split("\n", QString::SkipEmptyParts);

		for (QString row : rows)
		{
			row = row.trimmed();

			if (row.isEmpty() == true)
			{
				continue;
			}

			QStringList&& fields = row.split(';', QString::KeepEmptyParts);

			for (QString& field : fields)
			{
				field = field.trimmed();
				assert(field.length() > 0);
			}

			fileFields.push_back(fields);
		}
	}
	else
	{
		assert(false);
	}

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::onSignalPropertyChanged);

	for (const QStringList& propertyDescription : fileFields)
	{
		if (propertyDescription[1].toLower() == "true")
		{
			addPropertyDependentOnPrecision(propertyDescription[0]);
		}
	}

	for (int i = 0; i < signalVector.count(); i++)
	{
		Signal& appSignal = *signalVector[i];

		bool uppercaseAppSignalID = true;
		if (m_dbController->getProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, &uppercaseAppSignalID, this) == false)
		{
			assert(false);
		}
		if (uppercaseAppSignalID)
		{
			QString upperAppSignalId = appSignal.appSignalID().toUpper();
			if (appSignal.appSignalID() != upperAppSignalId)
			{
				QString message;
				if (readOnly == false && checkoutSignal(appSignal, message) == false)
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
					if (m_editedSignalsId.contains(appSignal.ID()) == false)
					{
						m_editedSignalsId.push_back(appSignal.ID());
					}
				}
			}
		}
		std::shared_ptr<SignalProperties> signalProperties = std::make_shared<SignalProperties>(appSignal, true);

		if (readOnly == true)
		{
			for (auto property : signalProperties->properties())
			{
				property->setReadOnly(true);
			}
		}

		int precision = appSignal.decimalPlaces();

		SignalPropertyManager& manager = SignalsModel::instance()->signalPropertyManager();
		manager.detectNewProperties(appSignal);
		manager.loadNotSpecificProperties(*signalProperties);
		manager.reloadPropertyBehaviour(dbController, parent);

		for (auto property : signalProperties->properties())
		{
			int propertyIndex = manager.index(property->caption());

			if (propertyIndex == -1)
			{
				if (property->category().isEmpty() == false)
				{
					// PropertyManager have to know about all properties
					assert(false);
				}
				continue;
			}

			if (manager.dependsOnPrecision(propertyIndex))
			{
				property->setPrecision(precision);
			}

			E::PropertyBehaviourType behaviour = manager.getBehaviour(appSignal, propertyIndex);
			if (manager.isHidden(behaviour))
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
	m_propertyEditor->resizeColumnToContents(0);
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


void SignalPropertiesDialog::checkAndSaveSignal()
{
	// Check
	//
	for (auto object : m_objList)
	{
		auto signalProperties = dynamic_cast<SignalProperties*>(object.get());
		if (signalProperties == nullptr)
		{
			assert(false);
			continue;
		}
		Signal& signal = signalProperties->signal();
		if (signal.appSignalID().trimmed().isEmpty())
		{
			QMessageBox::critical(this, "Error: Application signal ID is empty", "Fill Application signal ID");
			return;
		}
	}

	connect(this, &SignalPropertiesDialog::signalChanged, SignalsModel::instance(), &SignalsModel::loadSignal, Qt::QueuedConnection);

	// Save
	//
	for (int i = m_signalVector.count() - 1; i >= 0; i--)
	{
		Signal& signal = *m_signalVector[i];

		SignalProperties* signalProperties = dynamic_cast<SignalProperties*>(m_objList[i].get());
		if (signalProperties == nullptr)
		{
			assert(false);
			continue;
		}

		signalProperties->updateSpecPropValues();

		signal = signalProperties->signal();

		signal.setAppSignalID(signal.appSignalID().trimmed());
		if (signal.appSignalID().isEmpty() || signal.appSignalID()[0] != '#')
		{
			signal.setAppSignalID("#" + signal.appSignalID());
		}

		bool uppercaseAppSignalId = true;
		if (m_dbController->getProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, &uppercaseAppSignalId, this) == false)
		{
			assert(false);
		}
		else
		{
			if (uppercaseAppSignalId)
			{
				signal.setAppSignalID(signal.appSignalID().toUpper());
			}
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
	for (std::shared_ptr<PropertyObject> object : m_objList)
	{
		SignalProperties* signalProperites = dynamic_cast<SignalProperties*>(object.get());
		Signal& signal = signalProperites->signal();
		int id = signal.ID();
		if (!signal.checkedOut() && m_editedSignalsId.contains(id))
		{
			ObjectState state;
			m_dbController->undoSignalChanges(id, &state, this);
		}
	}
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
		SignalProperties* signalProperties = dynamic_cast<SignalProperties*>(object.get());

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
				m_propertyEditor->updatePropertyValues(property->caption());
			}
		}
	}
}

void SignalPropertiesDialog::checkoutSignals(QList<std::shared_ptr<PropertyObject> > objects)
{
	for (std::shared_ptr<PropertyObject> object : objects)
	{
		SignalProperties* signalProperites = dynamic_cast<SignalProperties*>(object.get());
		Signal& signal = signalProperites->signal();
		int id = signal.ID();
		if (signal.checkedOut() || m_editedSignalsId.contains(id))
		{
			if (!m_editedSignalsId.contains(id))
			{
				m_editedSignalsId.append(id);
			}
			continue;
		}
		QString message;
		if (checkoutSignal(signal, message) == false)
		{
			if (message.isEmpty() == false)
			{
				showError(message);
			}
			setWindowTitle("Signal properties (read only)");
			m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
			return;
		}
		if (!m_editedSignalsId.contains(id))
		{
			m_editedSignalsId.append(id);
		}
	}
}

void SignalPropertiesDialog::showError(QString errorString)
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

bool SignalPropertiesDialog::checkoutSignal(Signal& s, QString& message)
{
	if (s.checkedOut())
	{
		if (s.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			message = tr("Signal %1 is checked out by other user").arg(s.appSignalID());
			return false;
		}
	}

	QVector<int> signalsIDs;
	signalsIDs << s.ID();

	QVector<ObjectState> objectStates;
	m_dbController->checkoutSignals(&signalsIDs, &objectStates, m_parent);
	if (objectStates.count() == 0)
	{
		return false;
	}

	// First time collect all error output
	//
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}

	// Then decide do we have error
	//
	foreach (const ObjectState& objectState, objectStates)
	{
		switch (objectState.errCode)
		{
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER:
			if (objectState.userId != m_dbController->currentUser().userId() &&
					m_dbController->currentUser().isAdminstrator() == false)
			{
				return false;
			}
			break;
		case ERR_SIGNAL_OK:
			break;
		default:
			return false;
		}
	}
	return true;
}

QString SignalPropertiesDialog::errorMessage(const ObjectState& state) const
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT:
			return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER:
		{
			std::vector<DbUser> users;
			m_dbController->getUserList(&users, m_parent);
			QString userName;
			for (DbUser& user : users)
			{
				if (user.userId() == state.userId)
				{
					userName = user.username();
				}
			}
			if (userName.isEmpty())
			{
				return tr("Signal %1 is checked out by other user").arg(state.id).arg(userName);
			}
			else
			{
				return tr("Signal %1 is checked out by other user\"%2\"").arg(state.id).arg(userName);
			}
		}
		case ERR_SIGNAL_DELETED:
			return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND:
			return tr("Signal %1 not found").arg(state.id);
		case ERR_SIGNAL_EXISTS:
			return "";				// error message is displayed by PGSql driver
		default:
			return tr("Unknown error %1").arg(state.errCode);
	}
}

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	if (m_signalVector.size() < 1)
	{
		return;
	}

	SignalsModel* model = SignalsModel::instance();

	if (model == nullptr)
	{
		return;
	}

	SignalPropertyManager& manager = model->signalPropertyManager();

	const Signal& signal = *m_signalVector[0];

	QSettings settings(QSettings::UserScope, qApp->organizationName());

	for (int i = 0; i < manager.count(); i++)
	{
		if (manager.isHidden(manager.getBehaviour(signal, i)))
		{
			continue;
		}

		QString name = manager.name(i);
		settings.setValue(SignalProperties::lastEditedSignalFieldValuePlace + name, manager.value(&signal, i));
	}
}

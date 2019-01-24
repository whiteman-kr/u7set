#include "SignalPropertiesDialog.h"
#include "SignalsTabPage.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "Settings.h"
#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/DbController.h"
#include "../lib/WidgetUtils.h"
#include "Stable.h"

const std::vector<std::pair<E::SignalType, E::SignalInOutType>> signalTypeSequence =
{
	{E::Analog, E::SignalInOutType::Input},
	{E::Analog, E::SignalInOutType::Output},
	{E::Analog, E::SignalInOutType::Internal},

	{E::Discrete, E::SignalInOutType::Input},
	{E::Discrete, E::SignalInOutType::Output},
	{E::Discrete, E::SignalInOutType::Internal},

	{E::Bus, E::SignalInOutType::Input},
	{E::Bus, E::SignalInOutType::Output},
	{E::Bus, E::SignalInOutType::Internal},
};

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

	for (int i = 0; i < signalPtrVector.count(); i++)
	{
		result[i].first = foundSignalID[i];
		result[i].second = signalPtrVector[i]->appSignalID();
	}
	return result;
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

	if (theSettings.m_propertyEditorFontScaleFactor != 1.0)
	{
		m_propertyEditor->setFontSizeF(m_propertyEditor->fontSizeF() * theSettings.m_propertyEditorFontScaleFactor);
	}

	DbFileInfo mcInfo = dbController->systemFileInfo(dbController->etcFileId());

	if (mcInfo.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(EtcFileName));
		return;
	}

	DbFileInfo propertyBehaviorFile;
	dbController->getFileInfo(mcInfo.fileId(), QString(SignalPropertyBehaviorFileName), &propertyBehaviorFile, parent);

	if (propertyBehaviorFile.isNull() == true)
	{
		QMessageBox::critical(parent, "Error", QString("File \"%1\" is not found!").arg(SignalPropertyBehaviorFileName));
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
			QStringList&& fields = row.split(';', QString::KeepEmptyParts);

			for (QString& field : fields)
			{
				field = field.trimmed();
				assert(field.length() > 0);
			}

			assert(static_cast<size_t>(fields.size()) >= signalTypeSequence.size() + 2);
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

		std::shared_ptr<SignalProperties> signalProperties = std::make_shared<SignalProperties>(appSignal);

		if (readOnly == true)
		{
			for (auto property : signalProperties->properties())
			{
				property->setReadOnly(true);
			}
		}

		int precision = appSignal.decimalPlaces();

		for (const QStringList& propertyDescription : fileFields)
		{
			for (auto property : signalProperties->properties())
			{
				if (property->caption() != propertyDescription[0])
				{
					continue;
				}

				if (isPropertyDependentOnPrecision(property->caption()) == true)
				{
					property->setPrecision(precision);
				}

				bool descriptionFound = false;

				for (int i = 0; i < signalTypeSequence.size(); i++)
				{
					if ((appSignal.signalType() == signalTypeSequence[i].first &&
						 appSignal.inOutType() == signalTypeSequence[i].second) == false)
					{
						continue;
					}

					descriptionFound = true;

					const QString& propertyState = propertyDescription[i + 2].toLower();

					if (propertyState == "hide")
					{
						property->setVisible(false);
						break;
					}

					if (propertyState == "read")
					{
						property->setReadOnly(true);
						break;
					}

					if (propertyState == "expert")
					{
						if (theSettings.isExpertMode() == false)
						{
							property->setVisible(false);
						}

						break;
					}

					assert(propertyState == "write");
				}

				assert(descriptionFound == true);
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
		if (checkoutSignal(signal, message) && !message.isEmpty())
		{
			showError(message);
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
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != m_dbController->currentUser().userId() && !m_dbController->currentUser().isAdminstrator())
		{
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

	const Signal& signal = *m_signalVector[0];

	QSettings settings(QSettings::UserScope, qApp->organizationName());

	auto saver = [&settings](const QString& name, auto value)
	{
		settings.setValue(SignalProperties::lastEditedSignalFieldValuePlace + name, value);
	};

	saver(SignalProperties::acquireCaption, signal.acquire());
	saver(SignalProperties::decimalPlacesCaption, signal.decimalPlaces());
	saver(SignalProperties::unitCaption, signal.unit());
	saver(SignalProperties::coarseApertureCaption, signal.coarseAperture());
	saver(SignalProperties::fineApertureCaption, signal.fineAperture());
	saver(SignalProperties::byteOrderCaption, signal.byteOrder());

	SignalSpecPropValues spv;

	spv.create(signal);

	for(const SignalSpecPropValue& sv : spv.values())
	{
		QVariant qv = sv.value();

		settings.setValue(SignalProperties::lastEditedSignalFieldValuePlace + sv.name(), qv);
	}
}

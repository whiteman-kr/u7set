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

	if (signalPtrVector.isEmpty())
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
					case ERR_SIGNAL_ALREADY_CHECKED_OUT:
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
	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	if (theSettings.m_propertyEditorFontScaleFactor != 1.0)
	{
		m_propertyEditor->setFontSizeF(m_propertyEditor->fontSizeF() * theSettings.m_propertyEditorFontScaleFactor);
	}

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::onSignalPropertyChanged);

	for (int i = 0; i < signalVector.count(); i++)
	{
		//std::shared_ptr<SharedIdSignalProperties> signalProperties = std::make_shared<SharedIdSignalProperties>(signalVector, i);
		std::shared_ptr<SignalProperties> signalProperties = std::make_shared<SignalProperties>(*signalVector[i]);

		int precision = signalVector[i]->isAnalog() ? signalVector[i]->decimalPlaces() : 0;

		for (auto property : signalProperties->propertiesDependentOnPrecision())
		{
			property->setPrecision(precision);
		}

		if (readOnly)
		{
			for (auto property : signalProperties->properties())
			{
				property->setReadOnly(true);
			}
		}

		signalProperties->propertyByCaption(typeCaption)->setReadOnly(true);
		signalProperties->propertyByCaption(inOutTypeCaption)->setReadOnly(true);
		signalProperties->propertyByCaption(dataSizeCaption)->setReadOnly(true);
		signalProperties->propertyByCaption(byteOrderCaption)->setReadOnly(true);

		auto& s = signalProperties->signal();
		if (s.signalType() == E::SignalType::Bus)
		{
			signalProperties->propertyByCaption(dataSizeCaption)->setVisible(false);
			signalProperties->propertyByCaption(byteOrderCaption)->setVisible(false);

			signalProperties->propertyByCaption(enableTuningCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningDefaultValueCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningLowBoundCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningHighBoundCaption)->setVisible(false);
		}
		else
		{
			signalProperties->propertyByCaption(busTypeIDCaption)->setVisible(false);
		}

		if (s.isInternal() == false)
		{
			signalProperties->propertyByCaption(enableTuningCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningDefaultValueCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningLowBoundCaption)->setVisible(false);
			signalProperties->propertyByCaption(tuningHighBoundCaption)->setVisible(false);
		}

		if (s.isAnalog())
		{
			if (s.isInput() == false)
			{
				signalProperties->propertyByCaption(lowValidRangeCaption)->setVisible(false);
				signalProperties->propertyByCaption(highValidRangeCaption)->setVisible(false);
				signalProperties->propertyByCaption(filteringTimeCaption)->setVisible(false);
				signalProperties->propertyByCaption(spreadToleranceCaption)->setVisible(false);

				signalProperties->propertyByCaption(electricLowLimitCaption)->setVisible(false);
				signalProperties->propertyByCaption(electricHighLimitCaption)->setVisible(false);
				signalProperties->propertyByCaption(electricUnitCaption)->setVisible(false);
				signalProperties->propertyByCaption(sensorTypeCaption)->setVisible(false);
			}

			if (s.isInternal())
			{
				signalProperties->propertyByCaption(lowADCCaption)->setVisible(false);
				signalProperties->propertyByCaption(highADCCaption)->setVisible(false);
			}

			if (s.isOutput())
			{
				signalProperties->propertyByCaption(lowADCCaption)->setVisible(false);
				signalProperties->propertyByCaption(highADCCaption)->setVisible(false);
			}
			else
			{
				signalProperties->propertyByCaption(lowDACCaption)->setVisible(false);
				signalProperties->propertyByCaption(highDACCaption)->setVisible(false);

				signalProperties->propertyByCaption(outputModeCaption)->setVisible(false);
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

	setWindowPosition(this, "SignalPropertiesDialog/geometry");
}


void SignalPropertiesDialog::checkAndSaveSignal()
{
	// Check
	//
	for (auto object : m_objList)
	{
		auto signalProperties = dynamic_cast<SignalProperties*>(object.get());
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

		signal = dynamic_cast<SignalProperties*>(m_objList[i].get())->signal();

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
	QSettings settings;
	settings.setValue("SignalPropertiesDialog/geometry", geometry());
}

void SignalPropertiesDialog::onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects)
{
	if (m_tryCheckout)
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

		Signal& signal = signalProperties->signal();

		int precision = signal.isAnalog() ? signal.decimalPlaces() : 0;

		for (auto property : signalProperties->propertiesDependentOnPrecision())
		{
			property->setPrecision(precision);
			m_propertyEditor->updatePropertyValues(property->caption());
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
		if (objectState.errCode == ERR_SIGNAL_ALREADY_CHECKED_OUT
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
		case ERR_SIGNAL_ALREADY_CHECKED_OUT:
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
	QSettings settings(QSettings::UserScope, qApp->organizationName());
	Signal& signal = *m_signalVector[0];

	auto saver = [&settings](const QString& name, auto value)
	{
		settings.setValue(lastEditedSignalFieldValuePlace + name, value);
	};

	saver(lowADCCaption, signal.lowADC());
	saver(highADCCaption, signal.highADC());
	saver(lowEngeneeringUnitsCaption, signal.lowEngeneeringUnits());
	saver(highEngeneeringUnitsCaption, signal.highEngeneeringUnits());
	saver(unitCaption, signal.unit());
	saver(lowValidRangeCaption, signal.lowValidRange());
	saver(highValidRangeCaption, signal.highValidRange());
	saver(electricLowLimitCaption, signal.electricLowLimit());
	saver(electricHighLimitCaption, signal.electricHighLimit());
	saver(electricUnitCaption, signal.electricUnit());
	saver(sensorTypeCaption, signal.sensorType());
	saver(outputModeCaption, signal.outputMode());
	saver(acquireCaption, signal.acquire());
	saver(decimalPlacesCaption, signal.decimalPlaces());
	saver(coarseApertureCaption, signal.coarseAperture());
	saver(fineApertureCaption, signal.fineAperture());
	saver(filteringTimeCaption, signal.filteringTime());
	saver(spreadToleranceCaption, signal.spreadTolerance());
	saver(byteOrderCaption, signal.byteOrder());
}

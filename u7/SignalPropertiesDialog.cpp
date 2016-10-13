#include "SignalPropertiesDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include "../lib/SignalProperties.h"
#include "SignalsTabPage.h"
#include "../lib/PropertyEditor.h"
#include "../lib/DbController.h"
#include "Stable.h"


std::vector<std::pair<QString, QString>> editApplicationSignals(const QStringList& signalId, DbController* dbController, QWidget* parent)
{
	SignalSet signalSet;

	SignalsModel* model = SignalsModel::instance();
	if (model == nullptr)
	{
		assert(false);
		dbController->getSignals(&signalSet, parent);
		dbController->getUnits(Signal::unitList.get(), parent);
	}
	else
	{
		for (const QString& id : signalId)
		{
			Signal* signal = new Signal(*model->getSignalByStrID(id));
			signalSet.append(signal->ID(), signal);
		}
	}

	int readOnly = false;
	QVector<Signal*> signalVector;
	QMap<QString, int> signalIndexMap;
	int lastIndexProcessed = -1;
	QString wrongIds;
	QStringList foundIds;
	std::vector<std::pair<QString, QString>> result;

	result.resize(signalId.count());

	for (int i = 0; i < signalId.count(); i++)
	{
		QString id = signalId[i];
		id = id.trimmed();
		result[i].first = id;
		result[i].second = id;
		if (signalIndexMap.contains(id))
		{
			int index = signalIndexMap[id];
			signalVector.push_back(&signalSet[index]);
			foundIds.push_back(id);
			continue;
		}
		for (lastIndexProcessed++; lastIndexProcessed < signalSet.count(); lastIndexProcessed++)
		{
			QString currentId = signalSet[lastIndexProcessed].appSignalID();
			signalIndexMap.insert(currentId, lastIndexProcessed);
			if (currentId == id)
			{
				signalVector.push_back(&signalSet[lastIndexProcessed]);
				foundIds.push_back(id);
				break;
			}
		}
		if (lastIndexProcessed == signalSet.count())
		{
			wrongIds += id + "\n";
		}
	}
	if (!wrongIds.isEmpty())
	{
		QMessageBox::critical(parent, "Error", "Signal ID not found:\n\n" + wrongIds);
	}
	for (Signal* signal : signalVector)
	{
		if (signal->checkedOut() && signal->userID() != dbController->currentUser().userId())
		{
			readOnly = true;
		}
	}

	if (signalVector.isEmpty())
	{
		return result;
	}

	SignalPropertiesDialog dlg(signalVector, *Signal::unitList.get(), readOnly, model, parent);

	if (dlg.exec() == QDialog::Accepted)
	{
		QString message;
		for (int i = 0; i < signalVector.count(); i++)
		{
			ObjectState state;
			dbController->setSignalWorkcopy(signalVector[i], &state, parent);
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
						assert(false);
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

	for (int i = 0; i < foundIds.count(); i++)
	{
		for (size_t j = 0; j < result.size(); j++)
		{
			if (result[j].first == foundIds[i])
			{
				result[j].second = signalVector[i]->appSignalID();
			}
		}
	}
	return result;
}


SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, UnitList &unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent) :
	SignalPropertiesDialog::SignalPropertiesDialog(QVector<Signal*>() << &signal, unitInfo, readOnly, signalsModel, parent)
{

}

SignalPropertiesDialog::SignalPropertiesDialog(QVector<Signal*> signalVector, UnitList &unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent) :
	QDialog(parent),
	m_signalVector(signalVector),
	m_unitInfo(unitInfo),
	m_signalsModel(signalsModel)
{
	*Signal::unitList.get() = unitInfo;
	QSettings settings;

	QVBoxLayout* vl = new QVBoxLayout;
	ExtWidgets::PropertyEditor* pe = new ExtWidgets::PropertyEditor(this);

	connect(pe, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::onSignalPropertyChanged);

	for (int i = 0; i < signalVector.count(); i++)
	{
		//std::shared_ptr<SharedIdSignalProperties> signalProperties = std::make_shared<SharedIdSignalProperties>(signalVector, i);
		std::shared_ptr<SignalProperties> signalProperties = std::make_shared<SignalProperties>(*signalVector[i]);

		if (readOnly)
		{
			for (auto property : signalProperties->properties())
			{
				property->setReadOnly(true);
			}
		}

		signalProperties->propertyByCaption("Type")->setReadOnly(true);
		signalProperties->propertyByCaption("InOutType")->setReadOnly(true);
		signalProperties->propertyByCaption("DataSize")->setReadOnly(true);

		if (signalProperties->signal().isDiscrete())
		{
			/* WhiteMan 04.10.2016
			 * if (signalProperties->signal().dataFormat() != E::DataFormat::UnsignedInt)
			{
				checkoutSignal(QList<std::shared_ptr<PropertyObject>>() << signalProperties);
				signalProperties->signal().setAnalogSignalFormat(E::DataFormat::UnsignedInt);
			}
			if (signalProperties->signal().dataSize() != 1)
			{
				checkoutSignal(QList<std::shared_ptr<PropertyObject>>() << signalProperties);
				signalProperties->signal().setDataSize(1);
			}*/
			signalProperties->propertyByCaption("DataFormat")->setVisible(false);
		}

		if (!signalProperties->signal().isInternal())
		{
			signalProperties->propertyByCaption("EnableTuning")->setVisible(false);
			signalProperties->propertyByCaption("TuningDefaultValue")->setVisible(false);
		}

		if (signalProperties->signal().isAnalog())
		{
			if (!signalProperties->signal().isInput())
			{
				signalProperties->propertyByCaption("LowValidRange")->setVisible(false);
				signalProperties->propertyByCaption("HighValidRange")->setVisible(false);
				signalProperties->propertyByCaption("FilteringTime")->setVisible(false);
				signalProperties->propertyByCaption("SpreadTolerance")->setVisible(false);
			}

			if (signalProperties->signal().isInternal())
			{
				signalProperties->propertyByCaption("LowADC")->setVisible(false);
				signalProperties->propertyByCaption("HighADC")->setVisible(false);
			}

			if (signalProperties->signal().isOutput())
			{
				signalProperties->propertyByCaption("LowADC")->setVisible(false);
				signalProperties->propertyByCaption("HighADC")->setVisible(false);
			}
			else
			{
				signalProperties->propertyByCaption("LowDAC")->setVisible(false);
				signalProperties->propertyByCaption("HighDAC")->setVisible(false);
				signalProperties->propertyByCaption("OutputMode")->setVisible(false);
			}
		}
		m_objList.push_back(signalProperties);
	}

	pe->setObjects(m_objList);
	pe->resizeColumnToContents(0);
	vl->addWidget(pe);

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
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertiesDialog::reject);
	connect(this, &SignalPropertiesDialog::finished, this, &SignalPropertiesDialog::saveDialogSettings);
	if (m_signalsModel != nullptr)
	{
		connect(this, &SignalPropertiesDialog::onError, m_signalsModel, static_cast<void (SignalsModel::*)(QString)>(&SignalsModel::showError));
	}

	vl->addWidget(m_buttonBox);
	setLayout(vl);

	QRect desktopRect = QApplication::desktop()->screenGeometry(this);
	QPoint center = desktopRect.center();
	desktopRect.setSize(QSize(desktopRect.width() * 2 / 3, desktopRect.height() * 2 / 3));
	desktopRect.moveCenter(center);
	QRect windowRect = settings.value("SignalPropertiesDialog/geometry", desktopRect).toRect();
	if (windowRect.height() > desktopRect.height())
	{
		windowRect.setHeight(desktopRect.height());
	}
	if (windowRect.width() > desktopRect.width())
	{
		windowRect.setWidth(desktopRect.width());
	}
	setGeometry(windowRect);
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

		/* WhiteMan 04.10.2016
		 *
		if (signal.isDiscrete() && signal.dataFormat() != E::DataFormat::UnsignedInt)
		{
			QMessageBox::critical(this, "Could not save signal", "Error: Discrete signal has not UnsignedInt DataFormat");
			return;
		}
		if (signal.isAnalog() && signal.dataFormat() == E::DataFormat::UnsignedInt)
		{
			QMessageBox::critical(this, "Could not save signal", "Error: Analog signal has UnsignedInt DataFormat");
			return;
		}*/
	}

	// Save
	//
	for (int i = m_signalVector.count() - 1; i >= 0; i--)
	{
		Signal& signal = *m_signalVector[i];

		signal = dynamic_cast<SignalProperties*>(m_objList[i].get())->signal();

		signal.setAppSignalID(signal.appSignalID().trimmed());
		if (signal.appSignalID()[0] != '#')
		{
			signal.setAppSignalID("#" + signal.appSignalID());
		}

		signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
		if (signal.customAppSignalID().isEmpty())
		{
			signal.setCustomAppSignalID(signal.appSignalID().mid(1));
		}

		signal.setEquipmentID(signal.equipmentID().trimmed());

		if (signal.caption().isEmpty())
		{
			signal.setCaption("Signal " + signal.customAppSignalID());
		}
	}

	saveLastEditedSignalProperties();

	accept();
}


void SignalPropertiesDialog::saveDialogSettings()
{
	QSettings settings;
	settings.setValue("SignalPropertiesDialog/geometry", geometry());
}

void SignalPropertiesDialog::onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects)
{
	checkoutSignal(objects);
	for (std::shared_ptr<PropertyObject> object : objects)
	{
		/* WhiteMan 04.10.2016
		 *
		SignalProperties* signalProperites = dynamic_cast<SignalProperties*>(object.get());

		Signal& signal = signalProperites->signal();

		if (signal.isDiscrete() && signal.dataFormat() != E::DataFormat::UnsignedInt)
		{
			signal.setAnalogSignalFormat(E::DataFormat::UnsignedInt);
		}

		if (signal.isAnalog() && signal.dataFormat() == E::DataFormat::UnsignedInt)
		{
			signal.setAnalogSignalFormat(E::DataFormat::SignedInt);
		}*/
	}
}

void SignalPropertiesDialog::checkoutSignal(QList<std::shared_ptr<PropertyObject> > objects)
{
	if (m_signalsModel == nullptr)
	{
		return;
	}

	for (std::shared_ptr<PropertyObject> object : objects)
	{
		SignalProperties* signalProperites = dynamic_cast<SignalProperties*>(object.get());
		int id = signalProperites->signal().ID();
		int row = m_signalsModel->keyIndex(id);
		QString message;
		if (!m_signalsModel->checkoutSignal(row, message) && !message.isEmpty())
		{
			emit onError(message);
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

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	QSettings settings;
	Signal& signal = *m_signalVector[0];
	settings.setValue("SignalsTabPage/LastEditedSignal/lowADC", signal.lowADC());
	settings.setValue("SignalsTabPage/LastEditedSignal/highADC", signal.highADC());
	settings.setValue("SignalsTabPage/LastEditedSignal/lowEngeneeringUnits", signal.lowEngeneeringUnits());
	settings.setValue("SignalsTabPage/LastEditedSignal/highEngeneeringUnits", signal.highEngeneeringUnits());
	settings.setValue("SignalsTabPage/LastEditedSignal/unitID", signal.unitID());
	settings.setValue("SignalsTabPage/LastEditedSignal/lowValidRange", signal.lowValidRange());
	settings.setValue("SignalsTabPage/LastEditedSignal/highValidRange", signal.highValidRange());
	settings.setValue("SignalsTabPage/LastEditedSignal/unbalanceLimit", signal.unbalanceLimit());
	settings.setValue("SignalsTabPage/LastEditedSignal/inputLowLimit", signal.inputLowLimit());
	settings.setValue("SignalsTabPage/LastEditedSignal/inputHighLimit", signal.inputHighLimit());
	settings.setValue("SignalsTabPage/LastEditedSignal/inputUnitID", signal.inputUnitID());
	settings.setValue("SignalsTabPage/LastEditedSignal/inputSensorID", signal.inputSensorID());
	settings.setValue("SignalsTabPage/LastEditedSignal/outputLowLimit", signal.outputLowLimit());
	settings.setValue("SignalsTabPage/LastEditedSignal/outputHighLimit", signal.outputHighLimit());
	settings.setValue("SignalsTabPage/LastEditedSignal/outputUnitID", signal.outputUnitID());
	settings.setValue("SignalsTabPage/LastEditedSignal/outputSensorID", signal.outputSensorID());
	settings.setValue("SignalsTabPage/LastEditedSignal/outputMode", signal.outputMode());
	settings.setValue("SignalsTabPage/LastEditedSignal/acquire", signal.acquire());
	settings.setValue("SignalsTabPage/LastEditedSignal/calculated", signal.calculated());
	settings.setValue("SignalsTabPage/LastEditedSignal/normalState", signal.normalState());
	settings.setValue("SignalsTabPage/LastEditedSignal/decimalPlaces", signal.decimalPlaces());
	settings.setValue("SignalsTabPage/LastEditedSignal/aperture", signal.aperture());
	settings.setValue("SignalsTabPage/LastEditedSignal/filteringTime", signal.filteringTime());
	settings.setValue("SignalsTabPage/LastEditedSignal/spreadTolerance", signal.spreadTolerance());
	settings.setValue("SignalsTabPage/LastEditedSignal/byteOrder", signal.byteOrder());
}

#include "SignalPropertiesDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include "../include/Signal.h"
#include "SignalsTabPage.h"
#include "../include/PropertyEditor.h"
#include "../include/DbController.h"
#include "Stable.h"


void editApplicationSignals(const QStringList& signalId, DbController* dbController, QWidget* parent)
{
	SignalSet signalSet;
	dbController->getSignals(&signalSet, parent);
	dbController->getUnits(Signal::m_unitList.get(), parent);
	int readOnly = false;
	QVector<Signal*> signalVector;
	QMap<QString, int> signalIndexMap;
	int lastIndexProcessed = -1;
	QString wrongIds;
	for (QString id : signalId)
	{
		id = id.trimmed();
		if (signalIndexMap.contains(id))
		{
			int index = signalIndexMap[id];
			signalVector.push_back(&signalSet[index]);
			continue;
		}
		for (lastIndexProcessed++; lastIndexProcessed < signalSet.count(); lastIndexProcessed++)
		{
			QString currentId = signalSet[lastIndexProcessed].appSignalID();
			signalIndexMap.insert(currentId, lastIndexProcessed);
			if (currentId == id)
			{
				signalVector.push_back(&signalSet[lastIndexProcessed]);
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
		return;
	}

	SignalPropertiesDialog dlg(signalVector, *Signal::m_unitList.get(), readOnly, nullptr, parent);

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
	*Signal::m_unitList.get() = unitInfo;
	QSettings settings;

	QVBoxLayout* vl = new QVBoxLayout;
	ExtWidgets::PropertyEditor* pe = new ExtWidgets::PropertyEditor(this);

	connect(pe, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SignalPropertiesDialog::checkoutSignal);

	for (int i = 0; i < signalVector.count(); i++)
	{
		std::shared_ptr<Signal> signal = std::make_shared<Signal>(*signalVector[i]);
		signal->setReadOnly(readOnly);
		if (signal->isDiscrete())
		{
			signal->setDataFormat(E::UnsignedInt);
			signal->setDataSize(1);
			signal->propertyByCaption("DataFormat")->setReadOnly(true);
			signal->propertyByCaption("DataSize")->setReadOnly(true);
		}
		if (!signal->isInternal())
		{
			signal->propertyByCaption("EnableTuning")->setVisible(false);
			signal->propertyByCaption("TuningDefaultValue")->setVisible(false);
		}
		if (signal->isAnalog() && !signal->isOutput())
		{
			signal->propertyByCaption("OutputMode")->setVisible(false);
		}
		m_objList.push_back(signal);
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
		auto signal = dynamic_cast<Signal*>(object.get());
		if (signal->appSignalID().trimmed().isEmpty())
		{
			QMessageBox::critical(this, "Error: Application signal ID is empty", "Fill Application signal ID");
			return;
		}
		if (signal->isDiscrete() && signal->dataFormat() != E::UnsignedInt)
		{
			QMessageBox::critical(this, "Could not save signal", "Error: Discrete signal has not UnsignedInt DataFormat");
			return;
		}
	}

	// Save
	//
	for (int i = m_signalVector.count() - 1; i >= 0; i--)
	{
		Signal& signal = *m_signalVector[i];

		signal = *(dynamic_cast<Signal*>(m_objList[i].get()));

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

void SignalPropertiesDialog::checkoutSignal(QList<std::shared_ptr<PropertyObject> > objects)
{
	if (m_signalsModel == nullptr)
	{
		return;
	}

	for (std::shared_ptr<PropertyObject> object : objects)
	{
		Signal* signal = dynamic_cast<Signal*>(object.get());
		int row = m_signalsModel->keyIndex(signal->ID());
		QString message;
		if (!m_signalsModel->checkoutSignal(row, message) && !message.isEmpty())
		{
			emit onError(message);
			setWindowTitle("Signal properties (read only)");
			m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
			return;
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

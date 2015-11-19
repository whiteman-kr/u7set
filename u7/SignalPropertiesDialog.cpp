#include "SignalPropertiesDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include "../include/Signal.h"
#include "SignalsTabPage.h"
#include "../include/PropertyEditor.h"


/*void editSignals(const QStringList& signalId, DbController* dbController, QWidget* parent)
{
	int readOnly = false;
	QVector<Signal*> signalVector;
	for (int id : signalId)
	{
		Signal signal = m_signalSet[row];
		if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId())
		{
			readOnly = true;
		}
		signalVector << signal;
	}
	SignalPropertiesDialog dlg(signalVector, m_dataFormatInfo, m_unitInfo, readOnly, this, parent);

	if (dlg.exec() == QDialog::Accepted)
	{
		ObjectState state;
		dbController()->setSignalWorkcopy(&signal, &state, parrentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			showError(state);
		}

		loadSignal(row);
		emit signalActivated(row);
		return true;
	}
}*/


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

	for (int i = 0; i < signalVector.count(); i++)
	{
		std::shared_ptr<Signal> signal = std::make_shared<Signal>(*signalVector[i]);
		signal->setReadOnly(readOnly);
		if (signal->isDiscrete())
		{
			signal->setDataSize(1);
			signal->propertyByCaption("DataSize")->setReadOnly(true);
		}
		m_objList.push_back(signal);
	}
	pe->setObjects(m_objList);
	vl->addWidget(pe);

	QDialogButtonBox* buttonBox;

	if (!readOnly)
	{
		setWindowTitle("Signal properties editing");
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::checkAndSaveSignal);
	}
	else
	{
		setWindowTitle("Signal properties (read only)");
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::reject);
	}
	connect(buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertiesDialog::reject);
	connect(this, &SignalPropertiesDialog::finished, this, &SignalPropertiesDialog::saveDialogSettings);
	if (m_signalsModel != nullptr)
	{
		connect(this, &SignalPropertiesDialog::onError, m_signalsModel, static_cast<void (SignalsModel::*)(QString)>(&SignalsModel::showError));
	}

	vl->addWidget(buttonBox);
	setLayout(vl);

	resize(settings.value("Signal properties dialog: size", QSize(320, 640)).toSize());
}


void SignalPropertiesDialog::checkAndSaveSignal()
{
	for (int i = m_signalVector.count() - 1; i >= 0; i--)
	{
		Signal& signal = *m_signalVector[i];

		signal = *(dynamic_cast<Signal*>(m_objList[i].get()));
	}

	saveLastEditedSignalProperties();

	accept();
}


void SignalPropertiesDialog::saveDialogSettings()
{
	//Save expand state ???
}

void SignalPropertiesDialog::checkoutSignal()
{
	if (m_signalsModel == nullptr)
	{
		return;
	}

	for (int i = 0; i < m_signalVector.count(); i++)
	{
		int row = m_signalsModel->keyIndex(m_signalVector[i]->ID());
		QString message;
		if (!m_signalsModel->checkoutSignal(row, message) && !message.isEmpty())
		{
			emit onError(message);
			return;
		}
	}
}

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	// Save one of signals for signal adding template
}

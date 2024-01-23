#include "CreateSignalsDialog.h"

CreateSignalsDialog::CreateSignalsDialog(QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle("Create signals");

	QFormLayout* fl = new QFormLayout(this);

	m_equipmentIdEdit = new QLineEdit(this);
	m_equipmentIdEdit->setText("");

	fl->addRow(tr("EquipmentID"), m_equipmentIdEdit);

	m_signalTypeCombo = new QComboBox(this);

	std::vector<std::pair<int, QString>> signalTypes = E::enumValues<E::SignalType>();

	for(const auto& [key, value] : signalTypes)
	{
		m_signalTypeCombo->addItem(value, key);
	}

	m_signalTypeCombo->setCurrentIndex(1);

	fl->addRow(tr("Signal type"), m_signalTypeCombo);

	m_signalChannelCountEdit = new QLineEdit(this);
	m_signalChannelCountEdit->setText("1");
	QRegularExpression channelRegExp("[1-6]");
	QValidator* validator = new QRegularExpressionValidator(channelRegExp, this);
	m_signalChannelCountEdit->setValidator(validator);

	fl->addRow(tr("Signal channel count"), m_signalChannelCountEdit);

	m_signalCountEdit = new QLineEdit(this);
	m_signalCountEdit->setText("1");
	QRegularExpression countRegExp("[1-9]\\d{0,3}");
	validator = new QRegularExpressionValidator(countRegExp, this);
	m_signalCountEdit->setValidator(validator);

	fl->addRow(tr("Signal count"), m_signalCountEdit);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	fl->addRow(buttonBox);

	setLayout(fl);
}

QString CreateSignalsDialog::getEquipmentID() const
{
	return m_equipmentIdEdit->text();
}

E::SignalType CreateSignalsDialog::getSignalType() const
{
	bool ok = false;

	E::SignalType type = E::stringToValue<E::SignalType>(m_signalTypeCombo->currentText(), &ok);

	if (ok == false)
	{
		Q_ASSERT(false);
		type = E::Discrete;
	}

	return type;
}

int CreateSignalsDialog::getChannelCount() const
{
	return m_signalChannelCountEdit->text().toInt();
}

int CreateSignalsDialog::getSignalCount() const
{
	return m_signalCountEdit->text().toInt();
}


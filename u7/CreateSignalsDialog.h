#pragma once

class CreateSignalsDialog : public QDialog
{
public:
	CreateSignalsDialog(QWidget* parent);

	QString getEquipmentID() const;
	E::SignalType getSignalType() const;
	int getChannelCount() const;
	int getSignalCount() const;

private:
	QLineEdit* m_equipmentIdEdit = nullptr;
	QComboBox* m_signalTypeCombo = nullptr;
	QLineEdit* m_signalChannelCountEdit = nullptr;
	QLineEdit* m_signalCountEdit = nullptr;
};



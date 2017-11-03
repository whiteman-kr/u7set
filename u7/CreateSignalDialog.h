#ifndef CREATESIGNALDIALOG_H
#define CREATESIGNALDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include "../lib/Types.h"


class DbController;
class CreateSignalDialog;

struct CreatingSignalDialogOptions
{
	void init(QString schemaId, QString schemaCaption, QStringList equipmentIds, QStringList proposedAppSignalIds);

	// Fill it
	//
	QString m_schemaId;
	QString m_schemaCaption;
	QStringList m_equipmentIds;
	QStringList m_proposedAppSignalIds;

	// State variables, CreateSignalDialog has access to these
	//
	enum class SignalTypeAndFormat
	{
		Discrete,
		AnalogFloat32,
		AnalogSignedInt32,
		Bus
	};

private:
	QStringList m_lastEquipmentIds;
	SignalTypeAndFormat m_lastSignalType = SignalTypeAndFormat::Discrete;
	QString m_lastBusTypeId;
	QByteArray m_lastGeometry;

	friend class CreateSignalDialog;
};

struct CreateSignalDialogResult
{
	CreatingSignalDialogOptions::SignalTypeAndFormat signalType = CreatingSignalDialogOptions::SignalTypeAndFormat::Discrete;

	QStringList equipmentIds;
	QStringList appSignalIds;
	QStringList customSignalIds;

	QString busTypeId;
};


class CreateSignalDialog : public QDialog
{
	Q_OBJECT

protected:
	CreateSignalDialog(DbController* dbc, CreatingSignalDialogOptions* options, QWidget* parent);

	void initSignalIds();
	void initBusTypes();

public:
	// Call this static function to show dialog get result back
	//
	static QStringList showDialog(DbController* dbc, CreatingSignalDialogOptions* options, QWidget* parent);

	CreateSignalDialogResult resultData() const;

protected slots:
	void generateNewSignalIds();
	void equipmentIdToggled(bool checked);
	void busTypeToggled(bool checked);

	virtual void accept() override;

	// Properties
	//
private:
	const CreatingSignalDialogOptions& options() const;

private:
	DbController* m_dbc = nullptr;
	CreatingSignalDialogOptions m_options;

	std::vector<QCheckBox*> m_equipmentCheckBoxes;
	std::vector<QLineEdit*> m_appSiganalIds;
	std::vector<QLineEdit*> m_customSiganalIds;

	QRadioButton* m_signalTypeDiscrete = nullptr;
	QRadioButton* m_signalTypeFloatingPoint = nullptr;
	QRadioButton* m_signalTypeSignedInteger = nullptr;
	QRadioButton* m_signalTypeBus = nullptr;
	std::vector<std::pair<CreatingSignalDialogOptions::SignalTypeAndFormat, QRadioButton*>> m_signalTypeRadios;

	QComboBox* m_busTypeCombo = nullptr;

	QPushButton* m_generateNewIdsButton = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	CreateSignalDialogResult m_result;
};

#endif // CREATESIGNALDIALOG_H

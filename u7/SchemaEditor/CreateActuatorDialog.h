#pragma once

namespace VFrame30
{
	class ActuatorHeader;
} // namespace VFrame30

namespace Hardware
{
	class DeviceModule;
}

class QComboBox;
class QDialogButtonBox;
class QGridLayout;


class CreateActuatorDialog : public QDialog
{
	Q_OBJECT

public:
	CreateActuatorDialog(std::shared_ptr<VFrame30::ActuatorHeader> actuatorHeader, DbController* db, QWidget* parent);
	virtual ~CreateActuatorDialog();

private:
	void getAcmPresets();
	QString getModuleDescriptionFile(const QString& acmPresetName) const;

protected slots:
	void validate();
	void newPresetNameIsSet(const QString& newPresetName);

	virtual void subsystemsClicked();

	virtual void accept();
	virtual void reject();

private:
	DbController* m_db = nullptr;
	std::shared_ptr<VFrame30::ActuatorHeader> m_actuatorHeader;

	std::vector<std::shared_ptr<Hardware::DeviceModule>> m_presets;
	QStringList m_subsystemIds;

	// Ui
	//
	QGridLayout* m_layout = nullptr;

	QLabel* m_actuatorTypeIdLabel = nullptr;
	QLineEdit* m_actuatorTypeIdLineEdit = nullptr;

	QLabel* m_captionLabel = nullptr;
	QLineEdit* m_captionLineEdit = nullptr;

	QFrame* m_separator1 = nullptr;

	QLabel* m_acmPresetNameLabel = nullptr;
	QComboBox* m_acmPresetNameComboBox = nullptr;

	QLabel* m_moduleDescriptionFileLabel = nullptr;
	QLineEdit* m_moduleDescriptionFileLineEdit = nullptr;

	QFrame* m_separator2 = nullptr;

	QLabel* m_subsystemIdLabel = nullptr;
	QComboBox* m_subsystemIdComboBox = nullptr;
	QPushButton* m_subsystemsButton = nullptr;

	QLabel* m_lmNumberLabel = nullptr;
	QLineEdit* m_lmNumberLineEdit = nullptr;
	QLabel* m_lmNumberHelpLabel = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;
};

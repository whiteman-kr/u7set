#pragma once

namespace VFrame30
{
	class ActuatorHeader;
}

class QDialogButtonBox;

class ChooseActuatorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseActuatorDialog(const std::vector<std::shared_ptr<VFrame30::ActuatorHeader>>& actuators, QWidget* parent = nullptr);
	~ChooseActuatorDialog();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	void fillTree();
	void itemSelectionChanged();
	void acceptSelection();
	void itemDoubleClicked(QModelIndex index);
	// void okPressed();

public:
	std::shared_ptr<VFrame30::ActuatorHeader> result();

private:
	std::vector<std::shared_ptr<VFrame30::ActuatorHeader>> m_actuators;
	std::shared_ptr<VFrame30::ActuatorHeader> m_selectedActuator;

	QLabel* m_quickSearchLabel = nullptr;
	QLineEdit* m_quickSearchLineEdit = nullptr;

	QTreeWidget* m_actuatorsTreeWidget = nullptr;

	QLabel* m_captionLabel = nullptr;
	QLineEdit* m_captionLineEdit = nullptr;

	QLabel* m_descriptionLabel = nullptr;
	QPlainTextEdit* m_descriptionTextEdit = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;
};

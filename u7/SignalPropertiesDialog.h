#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "IdePropertyEditor.h"
#include "AppSignalSetProvider.h"

namespace ExtWidgets
{
	class PropertyEditor;
}

class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(const std::vector<AppSignal*>& signalsToEdit,
									bool readOnly, bool isExistSignals, QWidget* parent = nullptr);

	bool isEditedSignal(int id) const;
	bool hasEditedSignals() const;
	bool isValid() const;

	//

	static void initNewSignal(AppSignal& signal);
	static std::vector<std::pair<QString, QString>> editApplicationSignals(QStringList& signalId,
																		   DbController* dbController,
																		   QWidget* parent = nullptr);
signals:
	void signalChanged(int id, bool updateView);

private slots:
	void onSignalsPropChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void onOk();
	void onCancel();

private:
	void uppercaseAppSignalIDs();
	void createSignalsProps();

	void checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects);
	bool checkoutSignal(AppSignal&s , QString* message);

	void limitPropsPrecisionOnPropChanged(const QList<std::shared_ptr<PropertyObject>>& objects);

	bool checkAndSaveSignal();
	void undoCheckouts();


	void saveLastEditedSignalProperties();
	void saveDialogSettings();

	void setDialogEditable();
	void setDialogReadOnly();

	void showError(const QString& errMsg);

private:
	const std::vector<AppSignal*> m_signalsToEdit;
	std::vector<std::shared_ptr<PropertyObject>> m_signalsProps;

	AppSignalSetProvider* m_signalSetProvider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	bool m_readOnly = false;
	bool m_isExistSignals = false;						// exist signals should be checked out before properties changing
														// new signals don't require checkout
	bool m_uppercaseAppSignalID = false;
	bool m_isValid = false;

	bool m_firstPropChange = true;
	std::set<int> m_editedSignalsId;
	std::set<int> m_checkedOutSignalsId;						// signals checked out by SignalPropertiesDialog
	std::set<QString> m_propsWithPrecision;

	//

	IdePropertyEditor* m_propertyEditor = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;
};


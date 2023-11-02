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
	explicit SignalPropertiesDialog(const std::vector<AppSignal*>& signalVector,
									bool readOnly, bool tryCheckout, QWidget* parent = nullptr);

	bool isEditedSignal(int id) const { return m_editedSignalsId.contains(id); }
	bool hasEditedSignals() const { return m_editedSignalsId.empty() == false; }
	bool isValid() const { return m_isValid; }

	//

	static std::vector<std::pair<QString, QString>> editApplicationSignals(QStringList& signalId,
																		   DbController* dbController,
																		   QWidget* parent = nullptr);
	static void initNewSignal(AppSignal& signal);


signals:
	void signalChanged(int id, bool updateView);

public slots:
	void checkAndSaveSignal();
	void undoCheckouts();
	void saveDialogSettings();
	void onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects);
	void checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects);
	void saveLastEditedSignalProperties();
	void showError(const QString&errorString);

protected:
	void closeEvent(QCloseEvent* event);

private:
	bool checkoutSignal(const AppSignal& s, QString* message);

	bool isPropertyDependentOnPrecision(const QString& propName) const;
	void addPropertyDependentOnPrecision(const QString& propName);

private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
	AppSignalPropertyManager* m_propManager = nullptr;

	bool m_uppercaseAppSignalID = false;

	const std::vector<AppSignal*>& m_signalVector;
	std::set<int> m_editedSignalsId;
	std::set<int> m_checkedOutSignalsId;						// signals checked out by SignalPropertiesDialog
	std::vector<std::shared_ptr<PropertyObject>> m_objList;
	bool m_tryCheckout;

	QWidget* m_parent = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	IdePropertyEditor* m_propertyEditor = nullptr;

	bool m_isValid = false;

	std::set<QString> m_propertiesDependentOnPrecision;
};


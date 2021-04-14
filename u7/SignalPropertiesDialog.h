#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include "../lib/AppSignal.h"
#include "IdePropertyEditor.h"

class QtProperty;
class QtStringPropertyManager;
class QtEnumPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtBoolPropertyManager;
class QtTreePropertyBrowser;
class QDialogButtonBox;
enum class SignalType;


namespace ExtWidgets
{
	class PropertyEditor;
}


std::vector<std::pair<QString, QString> > editApplicationSignals(QStringList& signalId, DbController* dbController, QWidget *parent = 0);
void initNewSignal(Signal& signal);


class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(DbController* dbController, QVector<Signal*> signalVector, bool readOnly, bool tryCheckout, QWidget *parent = 0);

	bool isEditedSignal(int id) const { return m_editedSignalsId.contains(id); }
	bool hasEditedSignals() const { return m_editedSignalsId.isEmpty() == false; }

	bool isValid() const { return m_isValid; }

signals:
	void signalChanged(int id, bool updateView);

public slots:
	void checkAndSaveSignal();
	void rejectCheckoutProperty();
	void saveDialogSettings();
	void onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects);
	void checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects);
	void saveLastEditedSignalProperties();
	void showError(QString errorString);

protected:
	void closeEvent(QCloseEvent* event);

private:
	bool checkoutSignal(Signal& s, QString& message);
	QString errorMessage(const ObjectState& state) const;

	bool isPropertyDependentOnPrecision(const QString& propName) { return m_propertiesDependentOnPrecision.value(propName, false); }
	void addPropertyDependentOnPrecision(const QString& propName) { m_propertiesDependentOnPrecision.insert(propName, true); }

private:
	DbController* m_dbController;
	QVector<Signal*> m_signalVector;
	QVector<int> m_editedSignalsId;
	QDialogButtonBox* m_buttonBox;
	QList<std::shared_ptr<PropertyObject>> m_objList;
	bool m_tryCheckout;
	QWidget* m_parent;
	IdePropertyEditor* m_propertyEditor;

	bool m_isValid = false;

	QHash<QString, bool> m_propertiesDependentOnPrecision;
};

#endif // SIGNALPROPERTIESDIALOG_H

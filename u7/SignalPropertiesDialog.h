#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>
#include "../lib/Signal.h"

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
	DbController* m_dbController;
	QVector<Signal*> m_signalVector;
	QVector<int> m_editedSignalsId;
	QDialogButtonBox* m_buttonBox;
	QList<std::shared_ptr<PropertyObject>> m_objList;
	bool m_tryCheckout;
	QWidget* m_parent;
	ExtWidgets::PropertyEditor* m_propertyEditor;

	bool m_isValid = false;

	bool checkoutSignal(Signal& s, QString& message);
	QString errorMessage(const ObjectState& state) const;
};

#endif // SIGNALPROPERTIESDIALOG_H

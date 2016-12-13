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


std::vector<std::pair<QString, QString> > editApplicationSignals(const QStringList& signalId, DbController* dbController, QWidget *parent = 0);


class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(DbController* dbController, QVector<Signal*> signalVector, bool readOnly, bool tryCheckout, QWidget *parent = 0);

	bool isEditedSignal(int id) { return m_editedSignalsId.contains(id); }

signals:
	void signalChanged(int id, bool updateView);

public slots:
	void checkAndSaveSignal();
	void saveDialogSettings();
	void onSignalPropertyChanged(QList<std::shared_ptr<PropertyObject> > objects);
	void checkoutSignals(QList<std::shared_ptr<PropertyObject>> objects);
	void saveLastEditedSignalProperties();
	void showError(QString errorString);

private:
	DbController* m_dbController;
	QVector<Signal*> m_signalVector;
	QVector<int> m_editedSignalsId;
	QDialogButtonBox* m_buttonBox;
	QList<std::shared_ptr<PropertyObject>> m_objList;
	bool m_tryCheckout;
	QWidget* m_parent;

	bool checkoutSignal(Signal& s, QString& message);
	QString errorMessage(const ObjectState& state) const;
};

#endif // SIGNALPROPERTIESDIALOG_H

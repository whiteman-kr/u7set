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
class SignalsModel;
class QDialogButtonBox;
enum class SignalType;


void editApplicationSignals(const QStringList& signalId, DbController* dbController, QWidget *parent = 0);


class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(Signal& signal, UnitList& unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent = 0);
	explicit SignalPropertiesDialog(QVector<Signal*> signalVector, UnitList& unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent = 0);

signals:
	void onError(QString message);

public slots:
	void checkAndSaveSignal();
	void saveDialogSettings();
	void checkoutSignal(QList<std::shared_ptr<PropertyObject>> objects);
	void saveLastEditedSignalProperties();

private:
	QVector<Signal*> m_signalVector;
	QDialogButtonBox* m_buttonBox;
	UnitList& m_unitInfo;
	SignalsModel* m_signalsModel;
	QList<std::shared_ptr<PropertyObject>> m_objList;
};

#endif // SIGNALPROPERTIESDIALOG_H

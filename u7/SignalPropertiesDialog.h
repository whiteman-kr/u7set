#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include <QDialog>

class Signal;
class QtProperty;
class QtStringPropertyManager;

class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(Signal& signal, QWidget *parent = 0);

signals:

public slots:
	void saveSignal();

private:
	Signal& m_signal;

	QtStringPropertyManager* m_stringManager;

	QtProperty* m_strIDProperty;
	QtProperty* m_extStrIDProperty;
	QtProperty* m_nameProperty;
};

#endif // SIGNALPROPERTIESDIALOG_H

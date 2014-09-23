#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include <QDialog>

class Signal;
struct DataFormat;
struct Unit;
class QtProperty;
class QtStringPropertyManager;
class QtEnumPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;

class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(Signal& signal, QVector<DataFormat>& dataFormatInfo, QVector<Unit>& unitInfo, QWidget *parent = 0);

signals:

public slots:
	void saveSignal();

private:
	Signal& m_signal;

	QtStringPropertyManager* m_stringManager;
	QtEnumPropertyManager* m_enumManager;
	QtIntPropertyManager* m_intManager;
	QtDoublePropertyManager* m_doubleManager;

	QtProperty* m_strIDProperty;
	QtProperty* m_extStrIDProperty;
	QtProperty* m_nameProperty;
	QtProperty* m_dataFormatProperty;
	QtProperty *m_dataSizeProperty;
	QtProperty *m_lowAdcProperty;
	QtProperty *m_highAdcProperty;
	QtProperty *m_lowLimitProperty;
	QtProperty *m_highLimitProperty;
	QtProperty *m_unitProperty;
};

#endif // SIGNALPROPERTIESDIALOG_H

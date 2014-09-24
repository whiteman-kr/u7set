#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>

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
	QVector<DataFormat>& m_dataFormatInfo;
	QVector<Unit>& m_unitInfo;

	QtStringPropertyManager* m_stringManager;
	QtEnumPropertyManager* m_enumManager;
	QtIntPropertyManager* m_intManager;
	QtDoublePropertyManager* m_doubleManager;

	QtProperty* m_strIDProperty;
	QtProperty* m_extStrIDProperty;
	QtProperty* m_nameProperty;
	QtProperty* m_dataFormatProperty;
	QtProperty* m_dataSizeProperty;
	QtProperty* m_lowAdcProperty;
	QtProperty* m_highAdcProperty;
	QtProperty* m_lowLimitProperty;
	QtProperty* m_highLimitProperty;
	QtProperty* m_unitProperty;
	QtProperty* m_adjustmentProperty;
	QtProperty* m_excessLimitProperty;
	QtProperty* m_unbalanceLimitProperty;

	QtProperty* m_inputLowLimitProperty;
	QtProperty* m_inputHighLimitProperty;
	QtProperty* m_inputUnitProperty;
	QtProperty* m_inputSensorProperty;

	QtProperty* m_outputLowLimitProperty;
	QtProperty* m_outputHighLimitProperty;
	QtProperty* m_outputUnitProperty;
	QtProperty* m_outputSensorProperty;
};

#endif // SIGNALPROPERTIESDIALOG_H

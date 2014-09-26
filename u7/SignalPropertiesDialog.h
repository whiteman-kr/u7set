#ifndef SIGNALPROPERTIESDIALOG_H
#define SIGNALPROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>
#include "../include/Signal.h"

class QtProperty;
class QtStringPropertyManager;
class QtEnumPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtBoolPropertyManager;
enum SignalType;

class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(Signal& signal, SignalType signalType, DataFormatList& dataFormatInfo, UnitList& unitInfo, QWidget *parent = 0);

signals:

public slots:
	void checkAndSaveSignal();

private:
	Signal& m_signal;
	DataFormatList& m_dataFormatInfo;
	UnitList& m_unitInfo;

	QtStringPropertyManager* m_stringManager;
	QtEnumPropertyManager* m_enumManager;
	QtIntPropertyManager* m_intManager;
	QtDoublePropertyManager* m_doubleManager;
	QtBoolPropertyManager* m_boolManager;

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
	QtProperty* m_dropLimitProperty;
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

	QtProperty* m_acquireProperty;
	QtProperty* m_calculatedProperty;
	QtProperty* m_normalStateProperty;
	QtProperty* m_decimalPlacesProperty;
	QtProperty* m_apertureProperty;
	QtProperty* m_inOutTypeProperty;
	QtProperty* m_deviceIDProperty;
};

#endif // SIGNALPROPERTIESDIALOG_H

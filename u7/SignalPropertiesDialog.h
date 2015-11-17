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
class QtTreePropertyBrowser;
class SignalsModel;
enum class SignalType;

class SignalPropertiesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SignalPropertiesDialog(Signal& signal, E::SignalType signalType, DataFormatList& dataFormatInfo, UnitList& unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent = 0);
	explicit SignalPropertiesDialog(QVector<Signal*> signalVector, E::SignalType signalType, DataFormatList& dataFormatInfo, UnitList& unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent = 0);

signals:
	void onError(QString message);

public slots:
	void checkAndSaveSignal();
	void saveDialogSettings();
	void checkoutSignal();
	void saveLastEditedSignalProperties();

private:
	QVector<Signal*> m_signalVector;
	DataFormatList m_dataFormatInfo;
	UnitList& m_unitInfo;
	SignalsModel* m_signalsModel;
	QList<std::shared_ptr<PropertyObject>> m_objList;

	E::SignalType m_signalType;

	QtStringPropertyManager* m_stringManager;
	QtEnumPropertyManager* m_enumManager;
	QtIntPropertyManager* m_intManager;
	QtDoublePropertyManager* m_doubleManager;
	QtBoolPropertyManager* m_boolManager;
	QtTreePropertyBrowser* m_browser;

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

	QtProperty* m_inputTreeProperty;
	QtProperty* m_inputLowLimitProperty;
	QtProperty* m_inputHighLimitProperty;
	QtProperty* m_inputUnitProperty;
	QtProperty* m_inputSensorProperty;

	QtProperty* m_outputTreeProperty;
	QtProperty* m_outputLowLimitProperty;
	QtProperty* m_outputHighLimitProperty;
	QtProperty* m_outputUnitProperty;
	QtProperty* m_outputRangeModeProperty;
	QtProperty* m_outputSensorProperty;

	QtProperty* m_acquireProperty;
	QtProperty* m_calculatedProperty;
	QtProperty* m_normalStateProperty;
	QtProperty* m_decimalPlacesProperty;
	QtProperty* m_apertureProperty;
	QtProperty* m_filteringTimeProperty;
	QtProperty* m_maxDifferenceProperty;
	QtProperty* m_inOutTypeProperty;
	QtProperty* m_byteOrderProperty;
	QtProperty* m_deviceIDProperty;
};

#endif // SIGNALPROPERTIESDIALOG_H

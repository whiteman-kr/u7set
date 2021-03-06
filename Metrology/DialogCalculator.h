#ifndef DIALOGCALCULATOR_H
#define DIALOGCALCULATOR_H

#include <QObject>
#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "../lib/Types.h"
#include "../lib/UnitsConvertor.h"

// ==============================================================================================

struct UnitSensorTypePair
{
	int unitID;
	int sensorType;
};

const UnitSensorTypePair SensorTypeByUnit[] =
{
	// types of thermistors
	//
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_391 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_385 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_428 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_426 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni_a_617 },

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt21 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu23 },

	// types of thermocouple
	//
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_B },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_E },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_J },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_K },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_N },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_R },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_S },
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_T },
};

const int	SENSOR_TYPE_BY_UNIT_COUNT = sizeof(SensorTypeByUnit) / sizeof(SensorTypeByUnit[0]);

// ==============================================================================================

class DialogCalculator : public QDialog
{
	Q_OBJECT

public:

	explicit DialogCalculator(QWidget* parent = nullptr);
	virtual ~DialogCalculator();

private:

	void			createInterface();
	void			initDialog();

	QRadioButton*	m_pLinInRadio = nullptr;
	QLineEdit*		m_pLinInValEdit = nullptr;
	QRadioButton*	m_pLinOutRadio = nullptr;
	QLineEdit*		m_pLinOutValEdit = nullptr;
	QLineEdit*		m_pLinInLowEdit = nullptr;
	QLineEdit*		m_pLinInHighEdit = nullptr;
	QLineEdit*		m_pLinOutLowEdit = nullptr;
	QLineEdit*		m_pLinOutHighEdit = nullptr;

	QComboBox*		m_pTrList = nullptr;
	QRadioButton*	m_pTrDegreeRadio = nullptr;
	QLineEdit*		m_pTrDegreeEdit = nullptr;
	QRadioButton*	m_pTrElectricRadio = nullptr;
	QLineEdit*		m_pTrElectricEdit = nullptr;
	QHBoxLayout*	m_tr_R0_Layout = nullptr;
	QLineEdit*		m_pTrR0Edit = nullptr;

	QComboBox*		m_pTcList = nullptr;
	QRadioButton*	m_pTcDegreeRadio = nullptr;
	QLineEdit*		m_pTcDegreeEdit = nullptr;
	QRadioButton*	m_pTcElectricRadio = nullptr;
	QLineEdit*		m_pTcElectricEdit = nullptr;

	QRadioButton*	m_pDpfPRadio = nullptr;
	QLineEdit*		m_pDpfPValEdit = nullptr;
	QRadioButton*	m_pDpfFRadio = nullptr;
	QLineEdit*		m_pDpfFValEdit = nullptr;
	QLineEdit*		m_pDpfPLowEdit = nullptr;
	QLineEdit*		m_pDpfPHighEdit = nullptr;
	QLineEdit*		m_pDpfFLowEdit = nullptr;
	QLineEdit*		m_pDpfFHighEdit = nullptr;

	QRadioButton*	m_pDrСelsiusRadio = nullptr;
	QLineEdit*		m_pDrСelsiusEdit = nullptr;
	QRadioButton*	m_pDrFahrenheitRadio = nullptr;
	QLineEdit*		m_pDrFahrenheitEdit = nullptr;

	QFont*			m_digitFont = nullptr;

	UnitsConvertor	m_uc;

	void			conversionLin();
	void			conversionTr();
	void			conversionTc();
	void			conversionDpf();
	void			conversionDr();

private slots:


	void			onLinRadio() { conversionLin(); }
	void			onLinValue(QString) { conversionLin(); }

	void			onTrSensorTypeChanged(int) { conversionTr(); }
	void			onTrRadio() { conversionTr(); }
	void			onTrValue(QString) { conversionTr(); }

	void			onTcSensorTypeChanged(int) { conversionTc(); }
	void			onTcRadio() { conversionTc(); }
	void			onTcValue(QString) { conversionTc(); }

	void			onDpfRadio() { conversionDpf(); }
	void			onDpfValue(QString) { conversionDpf(); }

	void			onDrRadio() { conversionDr(); }
	void			onDrValue(QString) { conversionDr(); }
};

// ==============================================================================================

#endif // DIALOGCALCULATOR_H

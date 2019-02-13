#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QDialog>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "../lib/Types.h"

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
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1391 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1391 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1385 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1385 },

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_50_W1428 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_100_W1428 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_50_W1426 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_100_W1426 },

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni50_W1617 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni100_W1617 },

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt21 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu23 },

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_391 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_385 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_428 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_426 },
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni_a_617 },

	// types of thermocouple
	//
	{ E::ElectricUnit::mV,		E::SensorType::mV_K_TXA },
	{ E::ElectricUnit::mV,		E::SensorType::mV_L_TXK },
	{ E::ElectricUnit::mV,		E::SensorType::mV_N_THH },

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


class Calculator : public QDialog
{
	Q_OBJECT

public:

	explicit Calculator(QWidget* parent = 0);
	virtual ~Calculator();

private:

	void			createInterface();
	void			initDialog();

	QComboBox*		m_pTrList = nullptr;
	QRadioButton*	m_pTrDegreeRadio = nullptr;
	QLineEdit*		m_pTrDegreeEdit = nullptr;
	QRadioButton*	m_pTrElectricRadio = nullptr;
	QLineEdit*		m_pTrElectricEdit = nullptr;

	QComboBox*		m_pTcList = nullptr;
	QRadioButton*	m_pTcDegreeRadio = nullptr;
	QLineEdit*		m_pTcDegreeEdit = nullptr;
	QRadioButton*	m_pTcElectricRadio = nullptr;
	QLineEdit*		m_pTcElectricEdit = nullptr;

	QRadioButton*	m_pLinInRadio = nullptr;
	QLineEdit*		m_pLinInValEdit = nullptr;
	QRadioButton*	m_pLinOutRadio = nullptr;
	QLineEdit*		m_pLinOutValEdit = nullptr;
	QLineEdit*		m_pLinInLowEdit = nullptr;
	QLineEdit*		m_pLinInHighEdit = nullptr;
	QLineEdit*		m_pLinOutLowEdit = nullptr;
	QLineEdit*		m_pLinOutHighEdit = nullptr;

	void			conversionTr();
	void			conversionTc();
	void			conversionLin();

private slots:

	void			onTrSensorTypeChanged(int) { conversionTr(); }
	void			onTrRadio() { conversionTr(); }
	void			onTrValue(QString) { conversionTr(); }

	void			onTcSensorTypeChanged(int) { conversionTc(); }
	void			onTcRadio() { conversionTc(); }
	void			onTcValue(QString) { conversionTc(); }

	void			onLinRadio() { conversionLin(); }
	void			onLinValue(QString) { conversionLin(); }

};

// ==============================================================================================

#endif // CALCULATOR_H

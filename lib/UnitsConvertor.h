#ifndef UNITSCONVERTOR_H
#define UNITSCONVERTOR_H

#include <assert.h>

#include "Types.h"


const double RESISTOR_V_0_5 = 0.25;	 // 250 Ohm

//
//
const double V_0_5_LOW_LIMIT = 0;
const double V_0_5_HIGH_LIMIT = 5.1;

//
//
const double V_m10_p10_LOW_LIMIT = -11.5;
const double V_m10_p10_HIGH_LIMIT = 11.5;

//
//
class UnitsConvertor : public QObject
{
	Q_OBJECT

public:

	explicit UnitsConvertor(QObject *parent = nullptr);
	virtual ~UnitsConvertor();

public:

	Q_INVOKABLE QVariant physicalToElectric(double val, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);
	Q_INVOKABLE QVariant electricToPhysical(double val, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);

};


#endif // UNITSCONVERTOR_H

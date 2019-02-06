#ifndef UNITSCONVERTOR_H
#define UNITSCONVERTOR_H

#include <assert.h>

#include "Types.h"


const double RESISTOR_V_0_5 = 0.25;


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

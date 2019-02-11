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

enum class UnitsConvertResultError
{
	NoError = 0,
	Generic = 1,
	LowLimitOutOfRange = 2,
	HighLimitOutOfRange = 3
};

class UnitsConvertResult
{
	Q_GADGET

public:
	UnitsConvertResult();

	explicit UnitsConvertResult(double result);						// Good result constructor
	explicit UnitsConvertResult(UnitsConvertResultError errorCode, const QString& errorMessage);		// Generic Error constructor
	explicit UnitsConvertResult(UnitsConvertResultError errorCode, double expectedLowValidRange, double expectedHighValidRange);	// Range Error constructor

	bool ok() const;

	// Functions accessed if ok() is true

	double toDouble() const;

	// Functions accessed if ok() is false

	int errorCode() const;

	QString errorMessage() const;

	double expectedLowValidRange() const;
	double expectedHighValidRange() const;

	// Properties accessed from scripts

	Q_PROPERTY(bool ok READ ok)

	Q_PROPERTY(double toDouble READ toDouble)

	Q_PROPERTY(int errorCode READ errorCode)
	Q_PROPERTY(QString errorMessage READ errorMessage)
	Q_PROPERTY(double expectedLowValidRange READ expectedLowValidRange)
	Q_PROPERTY(double expectedHighValidRange READ expectedHighValidRange)

private:

	bool m_ok = false;

	double m_result = 0;

	UnitsConvertResultError m_errorCode = UnitsConvertResultError::NoError;

	QString m_errorMessage;

	double m_expectedLowValidRange = 0;
	double m_expectedHighValidRange = 0;
};

Q_DECLARE_METATYPE(UnitsConvertResult)

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
	Q_INVOKABLE UnitsConvertResult electricToPhysical(double val, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);

};


#endif // UNITSCONVERTOR_H

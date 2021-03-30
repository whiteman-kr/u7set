#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"

#include "SignalBase.h"

namespace Measure
{
	// ==============================================================================================

	enum Type
	{
		NoMeasureType	= -1,
		Linearity		= 0,
		Comparators		= 1,
	};

	const int TypeCount	= 2;

	#define ERR_MEASURE_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= Measure::TypeCount)

	QString TypeCaption(int measureType);

	// ==============================================================================================

	enum Kind
	{
		NoMeasureKind	= -1,
		OneRack			= 0,
		OneModule		= 1,
		MultiRack		= 2,
	};

	const int KindCount	= 3;

	#define ERR_MEASURE_KIND(kind) (TO_INT(kind) < 0 || TO_INT(kind) >= Measure::KindCount)

	QString KindCaption(int measureKind);

	// ==============================================================================================

	enum LimitType
	{
		NoLimitType	= -1,
		Electric	= 0,
		Engineering	= 1,
	};

	const int LimitTypeCount = 2;

	#define ERR_MEASURE_LIMIT_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= Measure::LimitTypeCount)

	QString LimitTypeCaption(int measureType);

	// ==============================================================================================

	enum ErrorType
	{
		NoErrorType	= -1,
		Absolute	= 0,
		Reduce		= 1,
		Relative	= 2,
	};

	const int ErrorTypeCount	= 3;

	#define ERR_MEASURE_ERROR_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= Measure::ErrorTypeCount)

	QString ErrorTypeCaption(int errorType);

	// ==============================================================================================

	enum ErrorResult
	{
		NoErrorResult	= -1,
		Ok				= 0,
		Failed			= 1,
	};

	const int ErrorResultCount = 2;

	#define ERR_MEASURE_ERROR_RESULT(result) (TO_INT(result) < 0 || TO_INT(result) >= Measure::ErrorResultCount)

	QString ErrorResultCaption(int errorResult);

	// ==============================================================================================

	enum AdditionalParam
	{
		NoAdditionalParam	= -1,
		MaxValue			= 0,
		SystemDeviation		= 1,
		StandardDeviation	= 2,
		LowHighBorder		= 3,
		Uncertainty			= 4,
	};

	const int AdditionalParamCount = 5;

				// now used 5 (1 .. 5)
				// maximum 16 items (0 .. 15)

	#define ERR_MEASURE_ADDITIONAL_PARAM(param) (TO_INT(param) < 0 || TO_INT(param) >= Measure::AdditionalParamCount)

	QString AdditionalParamCaption(int param);

	// ==============================================================================================

	const int Timeout[] =
	{
		0, 1, 2, 3, 5, 10, 15, 20, 30, 45, 60,  // default value of seconds
	};

	const int TimeoutCount = sizeof(Timeout)/sizeof(Timeout[0]);

	// ==============================================================================================

	const int MaxMeasurementInPoint = 20;

	// ==============================================================================================

	#define MEASURE_TIME_FORMAT "dd-MM-yyyy hh:mm:ss"

	// ==============================================================================================

	class Item
	{

	public:

		explicit Item(Type measureType = Measure::Type::NoMeasureType);
		virtual ~Item();

	public:

		void virtual clear();

		Measure::Type measureType() const { return m_measureType; }
		int measureTypeInt() const { return TO_INT(m_measureType); }
		void setMeasureType(Measure::Type type) { m_measureType = type; }
		void setMeasureType(int type) { m_measureType = static_cast<Measure::Type>(type); }

		Hash signalHash() const { return m_signalHash; }
		void setSignalHash(const Hash& hash) { m_signalHash = hash; }
		void setSignalHash(const QString& id) { m_signalHash = calcHash(id); }

		int measureID() const { return m_measureID; }
		void setMeasureID(int id) { m_measureID = id; }

		bool filter() const { return m_filter; }
		void setFilter(bool filter) { m_filter = filter; }

		bool isSignalValid() const { return m_signalValid; }
		void setSignalValid(bool valid) { m_signalValid = valid; }

		QString connectionAppSignalID() const;
		void setConnectionAppSignalID(const QString& appSignalID) { m_connectionAppSignalID = appSignalID; }

		Metrology::ConnectionType connectionType() const { return m_connectionType; }
		int connectionTypeInt() const { return TO_INT(m_connectionType); }
		QString connectionTypeStr() const;
		void setConnectionType(Metrology::ConnectionType type) { m_connectionType = type; }
		void setConnectionType(int type) { m_connectionType =  static_cast<Metrology::ConnectionType>(type); }

		QString appSignalID() const { return m_appSignalID; }
		void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; setSignalHash(m_appSignalID); }

		QString customAppSignalID() const { return m_customAppSignalID; }
		void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

		QString equipmentID() const { return m_equipmentID; }
		void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

		QString caption() const { return m_caption; }
		void setCaption(const QString& caption) { m_caption = caption; }

		Metrology::SignalLocation& location() { return m_location; }
		void setLocation(const Metrology::SignalLocation& location) { m_location = location; }

		int calibratorPrecision() const { return m_calibratorPrecision; }
		void setCalibratorPrecision(int precision) { m_calibratorPrecision = precision; }

		double nominal(LimitType limitType) const;
		QString nominalStr(LimitType limitType) const;
		void setNominal(LimitType limitType, double value);

		double measure(LimitType limitType) const;
		QString measureStr(LimitType limitType) const;
		void setMeasure(LimitType limitType, double value);

		void setLimits(const IoSignalParam &ioParam);

		double lowLimit(LimitType limitType) const;
		void setLowLimit(LimitType limitType, double lowLimit);

		double highLimit(LimitType limitType) const;
		void setHighLimit(LimitType limitType, double highLimit);

		QString unit(LimitType limitType) const;
		void setUnit(LimitType limitType, QString unit);

		int limitPrecision(LimitType limitType) const;
		void setLimitPrecision(LimitType limitType, int precision);

		QString limitStr(LimitType limitType) const;

		void calcError();

		double error(LimitType limitType, ErrorType errorType) const;
		QString errorStr() const;
		void setError(LimitType limitType, ErrorType errorType, double value);

		double errorLimit(LimitType limitType, ErrorType errorType) const;
		QString errorLimitStr() const;
		void setErrorLimit(LimitType limitType, ErrorType errorType, double value);

		int errorResult() const;
		QString errorResultStr() const;

		QDateTime measureTime() const { return m_measureTime; }
		QString measureTimeStr() const;
		void setMeasureTime(const QDateTime& time) { m_measureTime = time; }

		QString calibrator() const { return m_calibrator; }
		void setCalibrator(const QString& calibrator) { m_calibrator = calibrator; }

		void setCalibratorData(const IoSignalParam &ioParam);

		int reportType() const { return m_reportType; }
		void setReportType(int type) { m_reportType = type; }

		bool foundInStatistics() const { return m_foundInStatistics; }
		void setFoundInStatistics(bool signalIsfound) { m_foundInStatistics = signalIsfound; }

		Item* at(int index);

		virtual bool findInStatisticsItem(const StatisticsItem& si);
		virtual void updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si);

		Item& operator=(Item& from);

	private:

		Measure::Type m_measureType = Measure::Type::NoMeasureType;			// measure type
		Hash m_signalHash = UNDEFINED_HASH;									// hash calced from AppSignalID by function calcHash()

		int m_measureID = -1;												// primary key of record in SQL table
		bool m_filter = false;												// filter for record, if "true" - hide record

		bool m_signalValid = true;											// signal is valid during the measurement

		QString m_connectionAppSignalID;
		Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

		QString m_appSignalID;
		QString m_customAppSignalID;
		QString m_equipmentID;
		QString m_caption;

		Metrology::SignalLocation m_location;

		int m_calibratorPrecision = DefaultElectricUnitPrecesion;			// precision of electric range of calibrator

		double m_nominal[LimitTypeCount];
		double m_measure[LimitTypeCount];

		double m_lowLimit[LimitTypeCount];
		double m_highLimit[LimitTypeCount];
		QString m_unit[LimitTypeCount];
		int m_limitPrecision[LimitTypeCount];

		double m_adjustment = 0;

		double m_error[LimitTypeCount][ErrorTypeCount];
		double m_errorLimit[LimitTypeCount][ErrorTypeCount];

		QDateTime m_measureTime;											// measure time
		QString m_calibrator;												// calibrator name and calibrator SN
		int m_reportType = -1;												// report type

		bool m_foundInStatistics = true;									// after loading find signal in the statistics list
	};

	// ==============================================================================================

	class LinearityItem : public Item
	{

	public:

		LinearityItem();
		LinearityItem(const IoSignalParam& ioParam);
		~LinearityItem() override;

	public:

		void clear() override;

		void fill_measure_input(const IoSignalParam& ioParam);
		void fill_measure_internal(const IoSignalParam& ioParam);
		void fill_measure_output(const IoSignalParam& ioParam);

		void calcAdditionalParam(const IoSignalParam &ioParam);
		double calcUcertainty(const IoSignalParam &ioParam, LimitType limitType) const;

		double percent() const { return m_percent; }
		void setPercent(double percent) { m_percent = percent; }

		int measureCount() const { return m_measureCount; }
		void setMeasureCount(int count) { m_measureCount = count; }

		double measureItemArray(LimitType limitType, int index) const;
		QString measureItemStr(LimitType limitType, int index) const;
		void setMeasureItemArray(LimitType limitType, int index, double value);

		int additionalParamCount() const { return m_additionalParamCount; }
		void setAdditionalParamCount(int count) { m_additionalParamCount = count; }

		double additionalParam(LimitType limitType, int paramType) const;
		QString additionalParamStr(LimitType limitType, int paramType) const;
		void setAdditionalParam(LimitType limitType, int paramType, double value);

		void updateMeasureArray(LimitType limitType, Item* pMeasurement);
		void updateAdditionalParam(LimitType limitType, Item* pMeasurement);

		bool findInStatisticsItem(const StatisticsItem& si) override;
		void updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si) override;

		LinearityItem& operator=(const LinearityItem& from);

	private:

		double m_percent = 0;

		int m_measureCount = 0;
		double m_measureArray[LimitTypeCount][MaxMeasurementInPoint];

		int	m_additionalParamCount = 0;
		double m_additionalParam[LimitTypeCount][AdditionalParamCount];
	};

	// ==============================================================================================

	class ComparatorItem : public Item
	{

	public:

		ComparatorItem();
		explicit ComparatorItem(const IoSignalParam& ioParam);
		~ComparatorItem() override;

	public:

		void clear() override;

		void fill_measure_input(const IoSignalParam& ioParam);
		void fill_measure_internal(const IoSignalParam &ioParam);

		QString compareAppSignalID() const { return m_compareAppSignalID; }
		void setCompareAppSignalID(const QString& appSignalID) { m_compareAppSignalID = appSignalID; }

		QString outputAppSignalID() const { return m_outputAppSignalID; }
		void setOutputAppSignalID(const QString& appSignalID) { m_outputAppSignalID = appSignalID; }

		Metrology::CmpValueType cmpValueType() const { return m_cmpValueType; }
		int	cmpValueTypeInt() const { return TO_INT(m_cmpValueType); }
		QString cmpValueTypeStr() const;
		void setCmpValueType(Metrology::CmpValueType type) { m_cmpValueType = type; }
		void setCmpValueType(int type) { m_cmpValueType = static_cast<Metrology::CmpValueType>(type); }

		E::CmpType cmpType() const { return m_cmpType; }
		int cmpTypeInt() const { return TO_INT(m_cmpType); }
		QString cmpTypeStr() const;
		void setCmpType(Metrology::CmpValueType cmpValueType, E::CmpType cmpType);
		void setCmpType(int cmpType) { m_cmpType = static_cast<E::CmpType>(cmpType); }

		bool findInStatisticsItem(const StatisticsItem& si) override;
		void updateStatisticsItem(LimitType limitType, ErrorType errorType, StatisticsItem& si) override;

		ComparatorItem& operator=(const ComparatorItem& from);

	private:

		QString m_compareAppSignalID;
		QString m_outputAppSignalID;

		Metrology::CmpValueType m_cmpValueType = Metrology::CmpValueType::NoCmpValueType;
		E::CmpType m_cmpType = E::CmpType::Greate;
	};

	// ==============================================================================================

	class Base : public QObject
	{
		Q_OBJECT

	public:

		explicit Base(QObject* parent = nullptr);
		virtual ~Base() override;

	public:

		int count() const;
		void clear();

		int load(Measure::Type measureType);

		int append(Measure::Item* pMeasurement);
		Measure::Item* measurement(int index) const;
		bool remove(int index);
		bool remove(Measure::Type measureType, const std::vector<int>& keyList);	// keyList this is list of measureID

		// Statistics
		//
		void updateStatisticsItem(Measure::Type measureType, StatisticsItem& si);
		void updateStatisticsBase(Measure::Type measureType);
		void updateStatisticsBase(Measure::Type measureType, Hash signalHash);
		static void markNotExistMeasuremetsFromStatistics(Measure::Base* pThis);

	private:

		Measure::Type m_measureType = Measure::Type::NoMeasureType;

		mutable QMutex m_measureMutex;
		std::vector<Item*> m_measureList;

	signals:

		void updatedMeasureBase(Hash signalHash);
		void updateMeasureView();

	public slots:

		void signalBaseLoaded();

		void appendToBase(Measure::Item* pMeasurement);
		void removeFromBase(Measure::Type measureType, const std::vector<int>& keyList);
	};
}

// ==============================================================================================

#endif // MEASUREBASE_H

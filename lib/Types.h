#pragma once

#include <type_traits>
#include <assert.h>
#include <QObject>
#include <QMetaEnum>
#include <QVariant>

/*! \brief Contains enumerations used in RPCT*/
class E : public QObject
{
	Q_OBJECT

public:
	E() = delete;

	// Horz/Vert text Align enums
	// This enum type is used to describe alignment. It contains horizontal and vertical flags that can be
	// combined to produce the required effect.
	//

	/** \brief This enum describes horizontal text alignment
	 *
	This enum describes horizontal text alignment.\n\n
	<b>Example</b>
	\code
	var AlignLeft = 0x01;
	var AlignRight = 0x02;
	var AlignHCenter = 0x04;
	var AlignJustify = 0x08;
	...
	schemaItem.AlignHorz = AlignRight;
	\endcode
	 */
	enum HorzAlign
	{
		AlignLeft = Qt::AlignLeft,			/**< AlignLeft = 0x01*/
		AlignRight = Qt::AlignRight,		/**< AlignRight = 0x02*/
		AlignHCenter = Qt::AlignHCenter,	/**< AlignHCenter = 0x04*/
		AlignJustify = Qt::AlignJustify		/**< AlignJustify = 0x08*/
	};
	Q_ENUM(HorzAlign)

	/** \brief This enum describes vertical text alignment
	 *
	This enum describes vertical text alignment.\n\n
	<b>Example</b>
	\code
	var AlignTop = 0x20;
	var AlignBottom = 0x40;
	var AlignVCenter = 0x80;
	var AlignBaseline = 0x100;
	...
	schemaItem.AlignVert = AlignBottom;
	\endcode
	 */
	enum VertAlign
	{
		AlignTop = Qt::AlignTop,				/**< AlignTop = 0x20*/
		AlignBottom = Qt::AlignBottom,			/**< AlignBottom = 0x40*/
		AlignVCenter = Qt::AlignVCenter,		/**< AlignVCenter = 0x80*/
		AlignBaseline = Qt::AlignBaseline		/**< AlignBaseline = 0x100*/
	};
	Q_ENUM(VertAlign)

	// UserTextPos
	//
	enum class TextPos
	{
		LeftTop,
		Top,
		RightTop,
		Right,
		RightBottom,
		Bottom,
		LeftBottom,
		Left
	};
	Q_ENUM(TextPos)

	// Format analog
	//
	enum class AnalogFormat
	{
		e_9e = 'e',			// format as [-]9.9e[+|-]999
		E_9E = 'E',			// format as [-]9.9E[+|-]999
		f_9 = 'f',			// format as [-]9.9
		g_9_or_9e = 'g',	// use e or f format, whichever is the most concise
		G_9_or_9E = 'G'		// use E or f format, whichever is the most concise
	};
	Q_ENUM(AnalogFormat)

	// Signal Source
	//
	enum class SignalSource
	{
		AppDataService,
		TuningService
	};
	Q_ENUM(SignalSource)

	// Column Data
	//
	enum class ColumnData
	{
		AppSignalID = 0,
		CustomSignalID = 1,
		Caption = 2,
		State = 3,
		ImpactAppSignalID = 32,
		ImpactCustomSignalID = 33,
		ImpactCaption = 34,
		ImpactState = 35,
		CustomText = 64				// Add new
	};
	Q_ENUM(ColumnData)

	// SignalType
	//
	enum SignalType
	{
		Analog,
		Discrete,
		Bus
	};
	Q_ENUM(SignalType)

	enum class BusDataFormat
	{
		Discrete,
		Mixed
		//AnalogFloat32 ???
		//AnalogSigneInt32 ???
		//AnalogUnsignedInt32 ???
		//AnalogSigneInt16 ???
		//AnalogUnsignedInt16 ???
	};
	Q_ENUM(BusDataFormat)

	// SignalFunction
	//
	enum class SignalFunction
	{
		Input,					// physical input, application logic signal
		Output,					// physical output, application logic signal
		Validity,				// input/output validity, application logic signal
		Diagnostics				// Diagnostics signal
	};
	Q_ENUM(SignalFunction)

	// ByteOrder
	//
	enum ByteOrder
	{
		LittleEndian,
		BigEndian,
		NoEndian				// Dirrect write byte order is not applicable
	};
	Q_ENUM(ByteOrder)

	// DataFormat is used only for analog signals
	// ATTENTION: Keep in sync with AnalogAppSignalFormat (below)
	//
	enum class DataFormat
	{
		UnsignedInt = 0,
		SignedInt = 1,
		Float = 2
	};
	Q_ENUM(DataFormat)

	// AnalogAppSignalFormat
	//
	enum class AnalogAppSignalFormat
	{
		SignedInt32 = static_cast<int>(E::DataFormat::SignedInt),
		Float32 = static_cast<int>(E::DataFormat::Float)
	};
	Q_ENUM(AnalogAppSignalFormat)

	// MemoryArea
	//
	enum class MemoryArea
	{
		ApplicationData,
		DiagnosticsData
	};
	Q_ENUM(MemoryArea)

	// LogicModuleRamAccess
	//
	enum class LogicModuleRamAccess
	{
		Undefined = 0x00,
		Read  = 0x01,
		Write = 0x02,
		ReadWrite = 0x03
	};
	Q_ENUM(LogicModuleRamAccess)

	// Lan Controller Type Identifiers
	//
	enum class LanControllerType
	{
		Unknown = 0,
		Tuning = 1,
		AppData = 2,
		DiagData = 4,
		AppAndDiagData = 6
	};
	Q_ENUM(LanControllerType)

	// Software Module Type Identifiers
	//
	enum SoftwareType
	{
		Unknown = 8000,
		BaseService = 8999,
		Monitor = 9000,
		ConfigurationService = 9001,
		AppDataService = 9002,
		ArchiveService = 9003,
		TuningService = 9004,
		DiagDataService = 9005,
		TuningClient = 9006,
		Metrology = 9007,
		ServiceControlManager = 9008,
		TestClient = 9009
	};
	Q_ENUM(SoftwareType)

	// OutputMode
	//
	enum OutputMode
	{
		Plus0_Plus5_V = 0,
		Plus4_Plus20_mA = 1,
		Minus10_Plus10_V = 2,
		Plus0_Plus5_mA = 3,
		Plus0_Plus20_mA = 4,
		Plus0_Plus24_mA = 5,
	};
	Q_ENUM(OutputMode)

	// InputUnit
	//
	enum ElectricUnit
	{
		NoUnit = 0,
		mA = 1,
		mV = 2,
		Ohm = 3,
		V = 4,

		// oder version
		// NoInputUnit = 1,
		// mA = 15,
		// mV = 11,
		// Ohm = 20,
		// V = 12,
	};
	Q_ENUM(ElectricUnit)

	// SensorType
	//
	enum SensorType
	{
		NoSensor = 0,

		Ohm_Pt50_W1391 = 1,
		Ohm_Pt100_W1391 = 2,
		Ohm_Pt50_W1385 = 3,
		Ohm_Pt100_W1385 = 4,

		Ohm_Cu_50_W1428 = 5,
		Ohm_Cu_100_W1428 = 6,
		Ohm_Cu_50_W1426 = 7,
		Ohm_Cu_100_W1426 = 8,

		Ohm_Pt21 = 9,
		Ohm_Cu23 = 10,

		mV_K_TXA = 11,
		mV_L_TXK = 12,
		mV_N_THH = 13,

        //

        mV_Type_B = 14,
        mV_Type_E = 15,
        mV_Type_J = 16,
        mV_Type_K = 17,
        mV_Type_N = 18,
        mV_Type_R = 19,
        mV_Type_S = 20,
        mV_Type_T = 21,

        mV_Raw_Mul_8 = 22,
        mV_Raw_Mul_32 = 23,

		Ohm_Ni50_W1617 = 24,
		Ohm_Ni100_W1617 = 25,

		V_0_5 = 26,
		V_m10_p10 = 27,

		Ohm_Pt_a_391 = 28,
		Ohm_Pt_a_385 = 29,
		Ohm_Cu_a_428 = 30,
		Ohm_Cu_a_426 = 31,
		Ohm_Ni_a_617 = 32,
		Ohm_Raw = 33,
	};
	Q_ENUM(SensorType)

	// SignalInOutType
	//
	enum class SignalInOutType
	{
		Input = 0,
		Output = 1,
		Internal = 2
	};
	Q_ENUM(SignalInOutType)

	/** \brief This enum describes logic module and signal channel.
	 *
	This enum describes logic module and signal channel.\n\n
	<b>Example</b>
	\code
	var ChannelA = 0;
	var ChannelB = 1;
	var ChannelC = 2;
	var ChannelD = 3;
	...
	if (param.Channel == ChannelA)
	{
		...
	}
	\endcode
	 */
	enum class Channel
	{
		A = 0, /**< Channel A = 0*/
		B = 1, /**< Channel B = 1*/
		C = 2, /**< Channel C = 2 */
		D = 3  /**< Channel D = 3 */
	};
	Q_ENUM(Channel)

	// DataSourceState
	//
	enum class DataSourceState
	{
		NoData = 0,
		ReceiveData = 1,
		Stopped = 2
	};
	Q_ENUM(DataSourceState)

	// ConfigCheckerState
	//
	enum class ConfigCheckerState
	{
		Unknown = 0,
		Check = 1,
		Copy = 2,
		Verification = 3,
		Switch = 4,
		Actual = 5
	};
	Q_ENUM(ConfigCheckerState)

	// TimeType
	//
	enum class TimeType
	{
		Plant,
		System,
		Local,
		ArchiveId
	};
	Q_ENUM(TimeType)

	// RtTrendsSamplePeriod
	//
	// ATTENTION: After changes in this enum function getSamplePeriodCounter() MUST BE CHECKED !!!
	//
	enum class RtTrendsSamplePeriod
	{
		sp_5ms,
		sp_10ms,
		sp_20ms,
		sp_50ms,
		sp_100ms,
		sp_250ms,
		sp_500ms,
		sp_1s,
		sp_5s,
		sp_10s,
	};
	Q_ENUM(RtTrendsSamplePeriod)

	enum class TrendMode
	{
		Archive,
		Realtime
	};
	Q_ENUM(TrendMode)

	// Property editor type
	//
	enum class PropertySpecificEditor : qint16
	{
		None = 0,
		Password,
		Script,
		TuningFilter,
		SpecificPropertyStruct,
		LoadFileDialog,
		Svg
	};
	Q_ENUM(PropertySpecificEditor)

	enum class SpecificPropertyType
	{
		pt_int32,
		pt_uint32,
		pt_double,
		pt_bool,
		pt_e_channel,
		pt_string,
		pt_dynamicEnum,
	};
	Q_ENUM(SpecificPropertyType)

	enum class PropertyBehaviourType
	{
		Hide,
		Read,
		Write,
		Expert
	};
	Q_ENUM(PropertyBehaviourType)

	//

	enum class UalItemType
	{
		Unknown,
		Signal,
		Afb,
		Const,
		Transmitter,
		Receiver,
		Terminator,
		BusComposer,
		BusExtractor,
		LoopbackSource,
		LoopbackTarget
	};
	Q_ENUM(UalItemType)

	//

	enum class AppSignalStateFlagType
	{
		Validity,
		StateAvailable,
		Simulated,
		Blocked,
		Mismatch,
		AboveHighLimit,
		BelowLowLimit
	};
	Q_ENUM(AppSignalStateFlagType)

	// For Monitor
	//
	enum class ValueViewType
	{
		Dec, Hex, Bin16, Bin32, Bin64, Exp, Count
	};
	Q_ENUM(ValueViewType)

	// SchemaItemIndicator Type
	//
	enum class IndicatorType	// MUST BE SEQUENTIAL, AS VALUE IS A VECTOR INDEX
	{
		HistogramVert,
		ArrowIndicator,
		//Trend,
		//CustomDraw
		// !!!! COUNT IS DEFINED IN THE NEXT FUNCTION IndicatorTypeCount !!!
	};
	Q_ENUM(IndicatorType)

	static const size_t IndicatorTypeCount = 2;

	enum class CmpType
	{
		Equal,
		Greate,
		Less,
		NotEqual
	};
	Q_ENUM(CmpType)

	enum class IndicatorDrawSetpoints
	{
		AutoGenerated,
		CustomSetpoints,
		NoSetpoints
	};
	Q_ENUM(IndicatorDrawSetpoints)

	enum class IndicatorColorSource
	{
		ClientBehaviorByOutputSignalTag,
		StaticColorFromStruct
	};
	Q_ENUM(IndicatorColorSource)

public:
	template <typename ENUM_TYPE>
	static QMetaEnum metaEnum()
	{
		QMetaEnum me = QMetaEnum::fromType<ENUM_TYPE>();
		Q_ASSERT(me.isValid() == true);

		return me;
	}

	// Convert enum value (not index) to QString
	//
	template <typename ENUM_TYPE>
	static QString valueToString(int value)
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);
		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		const char* str = me.valueToKey(value);
		if (str == nullptr)
		{
			Q_ASSERT(str);
			return QString();
		}

		return {str};
	}

	// Convert enum value (not index) to QString
	//
	template <typename ENUM_TYPE>
	static QString valueToString(ENUM_TYPE value)
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);
		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		const char* str = me.valueToKey(static_cast<int>(value));
		if (str == nullptr)
		{
			Q_ASSERT(str);
			return {};
		}

		return {str};
	}

	// Convert QString to enum value (not index)
	//
	template <typename ENUM_TYPE>
	static std::pair<ENUM_TYPE, bool> stringToValue(const QString& str)
	{
		bool ok = false;
		auto resultVal = stringToValue<ENUM_TYPE>(str, &ok);

		return {resultVal, ok};
	}

	template <typename ENUM_TYPE>
	static ENUM_TYPE stringToValue(const QString& str, bool* ok)
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);

		if (ok != nullptr)
		{
			*ok = true;
		}

		static QMetaEnum me = metaEnum<ENUM_TYPE>();
		if (me.isValid() == false)
		{
			return static_cast<ENUM_TYPE>(me.value(0));
		}

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			if (QString::fromLocal8Bit(me.key(i)) == str)
			{
				return static_cast<ENUM_TYPE>(me.value(i));
			}
		}

		if (ok == nullptr)
		{
			Q_ASSERT(false);		// key is not found!
		}
		else
		{
			*ok = false;
		}

		return static_cast<ENUM_TYPE>(me.value(0));
	}

	// Get list of enum values and assigned String
	//
	template <typename ENUM_TYPE>
	static std::vector<std::pair<int, QString>> enumValues()
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);

		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		std::vector<std::pair<int, QString>> result;

		int keyCount = me.keyCount();
		result.reserve(keyCount);

		for (int i = 0; i < keyCount; i++)
		{
			result.push_back(std::make_pair(me.value(i), QString::fromLocal8Bit(me.key(i))));
		}

		return result;
	}

	// Get list of enum keys converted to QString
	//
	template <typename ENUM_TYPE>
	static QStringList enumKeyStrings()
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);

		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		int keyCount = me.keyCount();

		QStringList result;
		result.reserve(keyCount);

		for (int i = 0; i < keyCount; i++)
		{
			result.append(QString::fromLocal8Bit(me.key(i)));
		}

		return result;
	}

	// Check if enum containes value
	//
	template <typename ENUM_TYPE>
	static bool contains(int value)
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);

		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			if (me.value(i) == value)
			{
				return true;
			}
		}

		return false;
	}

	// Builds array of all enum values
	//
	template <typename ENUM_TYPE>
	static std::vector<ENUM_TYPE> values()
	{
		static_assert(std::is_enum<ENUM_TYPE>::value);

		static QMetaEnum me = metaEnum<ENUM_TYPE>();

		int keyCount = me.keyCount();

		std::vector<ENUM_TYPE> valuesArray;
		valuesArray.reserve(keyCount);

		for (int i = 0; i < keyCount; i++)
		{
			valuesArray.push_back(static_cast<ENUM_TYPE>(me.value(i)));
		}

		return valuesArray;
	}
};


inline uint qHash(E::AppSignalStateFlagType t, uint seed)
{
	return ::qHash(static_cast<int>(t), seed);
}

int getSamplePeriodCounter(E::RtTrendsSamplePeriod period, int lmWorkcycle_ms);


typedef QPair<QString, QString> StringPair;

const char* const DataFormatStr[] =
{
	"Unsigned Int",
	"Signed Int",
	"Float",
};


#define TO_INT(enumValue) (static_cast<int>(enumValue))
#define ENUM_COUNT(enumName) (static_cast<int>(enumName::Count))

template <typename EnumType>
EnumType IntToEnum(int value)
{
	return static_cast<EnumType>(value);
}

//
// OutputMessageLevel
//
enum class OutputMessageLevel
{
	Message,
	Success,
	Warning2,		// The least important warning
	Warning1,		// Just warning
	Warning0,		// The most important warning
	Error
};


const int	WORD_SIZE = 16,
			DWORD_SIZE = 32,

			FLOAT32_SIZE = 32,
			SIGNED_INT32_SIZE = 32,

			DISCRETE_SIZE = 1;

const int	ANALOG_SIZE_W = 2;

const int	WORD_SIZE_IN_BYTES = 2;				// WORD size in bytes

const int	SIZE_1BIT = 1;
const int	SIZE_8BIT = 8;
const int	SIZE_16BIT = 16;
const int	SIZE_32BIT = 32;

const int	SIZE_1WORD = 1;
const int	SIZE_2WORD = 2;

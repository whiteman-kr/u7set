#pragma once
#include <type_traits>
#include <assert.h>
#include <QObject>
#include <QMetaEnum>
#include <QVariant>

class E : public QObject
{
	Q_OBJECT
public:
	E();

	// Horz/Vert text Align enums
	// This enum type is used to describe alignment. It contains horizontal and vertical flags that can be
	// combined to produce the required effect.
	//
	enum HorzAlign
	{
		AlignLeft = Qt::AlignLeft,				// Aligns with the left edge.
		AlignRight = Qt::AlignRight,			// Aligns with the right edge.
		AlignHCenter = Qt::AlignHCenter,		// Centers horizontally in the available space.
		AlignJustify = Qt::AlignJustify			// Justifies the text in the available space.
	};
	Q_ENUM(HorzAlign)

	enum VertAlign
	{
		AlignTop = Qt::AlignTop,				// Aligns with the top.
		AlignBottom = Qt::AlignBottom,			// Aligns with the bottom.
		AlignVCenter = Qt::AlignVCenter,		// Centers vertically in the available space.
		AlignBaseline = Qt::AlignBaseline		// Aligns with the baseline.
	};
	Q_ENUM(VertAlign)

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

	// Column Data
	//
	enum class ColumnData
	{
		AppSignalID,
		CustomSignalID,
		Caption,
		State
	};
	Q_ENUM(ColumnData)

	// SignalType
	//
	enum SignalType
	{
		Analog,
		Discrete
	};
	Q_ENUM(SignalType)

	enum class SignalFunction
	{
		Input,					// physical input, application logic signal
		Output,					// physical output, application logic signal
		Validity,				// input/output validity, application logic signal
		Diagnostics				// Diagnostics signal
	};
	Q_ENUM(SignalFunction)

	enum ByteOrder
	{
		LittleEndian,
		BigEndian
	};
	Q_ENUM(ByteOrder)

	// DataFormat is used only for analog signals
	// ATTENTION: Keep in sync with AppSignalDataFormat
	//
	enum class DataFormat
	{
		UnsignedInt = 0,
		SignedInt = 1,
		Float = 2
	};
	Q_ENUM(DataFormat)

	enum class AnalogAppSignalFormat
	{
		SignedInt32 = static_cast<int>(E::DataFormat::SignedInt),
		Float32 = static_cast<int>(E::DataFormat::Float)
	};
	Q_ENUM(AnalogAppSignalFormat)

	enum class MemoryArea
	{
		ApplicationData,
		DiagnosticsData
	};
	Q_ENUM(MemoryArea)

	// Software Module Type Identifiers
	//
	enum SoftwareType
	{
		Monitor = 9000,
		ConfigurationService = 9001,
		AppDataService = 9002,
		ArchiveService = 9003,
		TuningService = 9004,
		DiagDataService = 9005,
		TuningClient = 9006,
	};
	Q_ENUM(SoftwareType)

	enum OutputMode
	{
		Plus0_Plus5_V = 0,
		Plus4_Plus20_mA = 1,
		Minus10_Plus10_V = 2,
		Plus0_Plus5_mA = 3,
	};
	Q_ENUM(OutputMode)

    enum InputUnit
    {
        NoInputUnit = 1,
        mA = 15,
        mV = 11,
        Ohm = 20,
        V = 12,
    };
    Q_ENUM(InputUnit)

    enum SensorType
    {
        NoSensorType = 0,

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
    };
    Q_ENUM(SensorType)

	enum class SignalInOutType
	{
		Input = 0,
		Output = 1,
		Internal = 2
	};
	Q_ENUM(SignalInOutType)

	enum class Channel
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3
	};
	Q_ENUM(Channel)

	enum class DataSourceState
	{
		NoData = 0,
		ReceiveData = 1,
		Stopped = 2
	};
	Q_ENUM(DataSourceState)

public:
	// Convert enum value (not index) to QString
	//
	template <typename ENUM_TYPE>
	static QString valueToString(int value)
	{
		assert(std::is_enum<ENUM_TYPE>::value);

		QMetaEnum me = QMetaEnum::fromType<ENUM_TYPE>();
		if (me.isValid() == false)
		{
			assert(me.isValid() == true);
			return QString();
		}

		const char* str = me.valueToKey(value);
		if (str == nullptr)
		{
			assert(str);
			return QString();
		}

		QString result(str);
		return result;
	}

	// Get list of enum values and assigned String
	//
	template <typename ENUM_TYPE>
	static std::list<std::pair<int, QString>> enumValues()
	{
		assert(std::is_enum<ENUM_TYPE>::value);

		std::list<std::pair<int, QString>> result;

		QMetaEnum me = QMetaEnum::fromType<ENUM_TYPE>();
		if (me.isValid() == false)
		{
			assert(me.isValid() == true);
			return result;
		}

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			result.push_back(std::make_pair(me.value(i), QString::fromLocal8Bit(me.key(i))));
		}

		return result;
	}
};


typedef QPair<QString, QString> StringPair;


const char* const DataFormatStr[] =
{
	"Unsigned Int",
	"Signed Int",
	"Float",
};

enum class UartID
{
	LmAppLogicUart = 0x101,
	LmConfigUart = 0x102,
	LmServiceUart = 0x103,
	LmTuningUart = 0x104,
	AimUart = 0x203,
	AomUart = 0x303,
	DimUart = 0x403,
	DomUart = 0x503,
	AifmUart = 0x603,
	OcmUart = 0x703

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

const int	SIZE_1BIT = 1;
const int	SIZE_8BIT = 8;
const int	SIZE_16BIT = 16;
const int	SIZE_32BIT = 32;


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


	enum DataFormat
	{
		UnsignedInt,
		SignedInt,
		Float
	};
	Q_ENUM(DataFormat)

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
	};
	Q_ENUM(SoftwareType)


	enum InstanceAction
	{
		Added = 1,
		Modified = 2,
		Deleted = 3
	};
	Q_ENUM(InstanceAction)

	enum OutputRangeMode
	{
		Plus0_Plus5_V = 0,
		Plus4_Plus20_mA = 1,
		Minus10_Plus10_V = 2,
		Plus0_Plus5_mA = 3,
	};
	Q_ENUM(OutputRangeMode)

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



const char* const DataFormatStr[] =
{
	"Unsigned Int",
	"Signed Int",
	"Float",
};

Q_DECLARE_METATYPE(E::Channel)

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

#define C_STR(qstring) qstring.toStdString().c_str()

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
	Warning,
	Error
};


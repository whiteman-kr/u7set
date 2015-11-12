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


	enum SignalType
	{
		Analog,
		Discrete
	};
	Q_ENUM(SignalType)


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
		DataAcquisitionService = 9002,
		DataArchivingService = 9003,
	};
	Q_ENUM(SoftwareType)

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



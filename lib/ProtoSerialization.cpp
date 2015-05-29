//#include "Stable.h"
#include "QUuid"

#include "../include/ProtoSerialization.h"

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4125)
#pragma warning(disable : 6011)
#pragma warning(disable : 4100)
#pragma warning(disable : 4267)
#pragma warning(disable : 4512)
#pragma warning(disable : 4127)
#pragma warning(disable : 4996)
#endif

#include "../Proto/serialization.pb.cc"		// File generated by proto buffers compiller

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace Proto
{

	// ������� ��� ������������ ������
	//
	const QUuid& Read(const Proto::Uuid& message)
	{
		return *(reinterpret_cast<const QUuid*>(message.uuid().c_str()));
	}

	void Write(Proto::Uuid* pMessage, const QUuid& guid)
	{
		if (pMessage == nullptr)
		{
			assert(pMessage != nullptr);
			return;
		}
		pMessage->set_uuid(&guid, sizeof(guid));
	}

	// Read/write wstring message
	//
	void Read(const Proto::wstring& message, QString* dst)
	{
		*dst = QString::fromUtf16(reinterpret_cast<const ushort*>(message.text().data()),
										  static_cast<int>(message.text().size() / 2) - 1);
		return;
	}

	void Write(Proto::wstring* pMessage, const QString& str)
	{
        assert(sizeof(QChar) == 2);
        //assert(sizeof(wchar_t) == 2);
		//static_assert(sizeof(wchar_t) == 2, "wchar_t must be 16-bit.");

		if (pMessage == nullptr)
		{
			assert(pMessage != nullptr);
			return;
		}

        //pMessage->set_text(str.toStdWString().c_str(), (str.length() + 1) * sizeof(wchar_t));
        pMessage->set_text(str.data(), (str.length() + 1) * sizeof(QChar));
	}

	// Read/write wstring message
	//
	const QVariant Read(const Proto::qvariant& message)
	{
		switch (static_cast<QVariant::Type>(message.type()))
		{
		case QVariant::Int:
		{
			return QVariant(message.intvalue());
			break;
		}
		case QVariant::UInt:
		{
			return QVariant(message.uintvalue());
			break;
		}
		case QVariant::Double:
		{
			return QVariant(message.doublevalue());
			break;
		}
		case QVariant::Bool:
		{
			return QVariant(message.boolvalue());
			break;
		}
		default:
			assert(false);
		}

		return QVariant();
	}

	void Write(Proto::qvariant* pMessage, const QVariant& value)
	{
		pMessage->set_type(value.type());

		switch (value.type())
		{
		case QVariant::Int:
		{
			pMessage->set_intvalue(value.toInt());
			break;
		}
		case QVariant::UInt:
		{
			pMessage->set_uintvalue(value.toUInt());
			break;
		}
		case QVariant::Double:
		{
			pMessage->set_doublevalue(value.toDouble());
			break;
		}
		case QVariant::Bool:
		{
			pMessage->set_boolvalue(value.toBool());
			break;
		}
		default:
			assert(false);
		}
	}
}



#include "QUuid"

#include "../lib/ProtoSerialization.h"
#include "PropertyObject.h"

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
	bool ParseFromIstream(::google::protobuf::Message& message, std::fstream& stream)
	{
		bool result = message.ParseFromIstream(&stream);
		return result;
	}

	bool ParseFromString(::google::protobuf::Message& message, const char* str)
	{
		bool result = message.ParseFromString(str);
		return result;
	}

	bool ParseFromArray(::google::protobuf::Message& message, const QByteArray& data)
	{
		bool result = message.ParseFromArray(data.constData(), data.size());
		return result;
	}


	// Helper serialization functions
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

		if (pMessage == nullptr)
		{
			assert(pMessage != nullptr);
			return;
		}

        pMessage->set_text(str.data(), (str.length() + 1) * sizeof(QChar));
		return;
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

		return;
	}

	void saveProperty(::Proto::Property* protoProperty, const std::shared_ptr<::Property>& property)
	{
		return saveProperty(protoProperty, property.get());
	}

	void saveProperty(::Proto::Property* protoProperty, const ::Property* property)
	{
		assert(property);

		protoProperty->set_name(property->caption().toStdString());

		QString valueStr;
		QVariant value = property->value();

		if (property->isEnum() == true)
		{
			valueStr = value.toString();
		}
		else
		{
			switch (value.type())
			{
			case QVariant::Bool:
				valueStr = value.toBool() ? "t" : "f";
				break;
			case QVariant::Int:
				valueStr.setNum(value.toInt());
				break;
			case QVariant::UInt:
				valueStr.setNum(value.toUInt());
				break;
			case QVariant::String:
				valueStr = value.toString();
				break;
			case QVariant::Double:
				valueStr.setNum(value.toDouble());
				break;
			default:
				assert(false);
			}
		}

		protoProperty->set_value(valueStr.toUtf8());

		return;
	}

	bool loadProperty(const ::Proto::Property& protoProperty, const std::shared_ptr<::Property>& property)
	{
		return loadProperty(protoProperty, property.get());
	}

	bool loadProperty(const ::Proto::Property& protoProperty, ::Property* property)
	{
		assert(property);

		if (protoProperty.name() != property->caption().toStdString())
		{
			assert(protoProperty.name() == property->caption().toStdString());
			return false;
		}

		bool ok = false;
		QString sv = QString::fromUtf8(protoProperty.value().c_str());
		QVariant value = property->value();

		if (property->isEnum() == true)
		{
			property->setValue(protoProperty.value().c_str());
			return true;
		}

		switch (value.type())
		{
			case QVariant::Bool:
				{
					value = (sv == "t") ? true : false;
					ok = true;
				}
				break;
			case QVariant::Int:
				{
					qint32 i = sv.toInt(&ok);
					value = QVariant(i);
				}
				break;
			case QVariant::UInt:
				{
					quint32 ui = sv.toUInt(&ok);
					value = QVariant(ui);
				}
				break;
			case QVariant::String:
				{
					value = sv;
					ok = true;
				}
				break;
			case QVariant::Double:
				{
					double d = sv.toDouble(&ok);
					value = QVariant(d);
				}
				break;
			default:
				assert(false);
		}

		if (ok == true)
		{
			property->setValue(value);
		}

		return ok;
	}
}



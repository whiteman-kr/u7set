#include "ProtoSerialization.h"
#include <QUuid>
#include "../lib/WUtils.h"
#include "../lib/PropertyObject.h"

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
		switch (static_cast<QMetaType::Type>(message.type()))
		{
		case QMetaType::Int:
			return QVariant(message.intvalue());
		case QMetaType::UInt:
			return QVariant(message.uintvalue());
		case QMetaType::UShort:
			return QVariant::fromValue<unsigned short>(static_cast<unsigned short>(message.uintvalue()));
		case QMetaType::Float:
			return QVariant::fromValue<float>(message.floatvalue());
		case QMetaType::Double:
			return QVariant(message.doublevalue());
		case QMetaType::Bool:
			return QVariant(message.boolvalue());
		default:
			assert(false);
		}

		return QVariant();
	}

	void Write(Proto::qvariant* pMessage, const QVariant& value)
	{
		pMessage->set_type(value.type());

		switch (static_cast<QMetaType::Type>(value.type()))
		{
		case QMetaType::Int:
			pMessage->set_intvalue(value.toInt());
			break;
		case QMetaType::UInt:
			pMessage->set_uintvalue(value.toUInt());
			break;
		case QMetaType::UShort:
			pMessage->set_uintvalue(value.value<unsigned short>());
			break;
		case QMetaType::Float:
			pMessage->set_floatvalue(value.toFloat());
			break;
		case QMetaType::Double:
			pMessage->set_doublevalue(value.toDouble());
			break;
		case QMetaType::Bool:
			pMessage->set_boolvalue(value.toBool());
			break;
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
			valueStr = property->enumValue().toString();
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

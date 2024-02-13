#include <QUuid>

#include "ProtoSerialization.h"
#include "../CommonLib/PropertyObject.h"

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

}

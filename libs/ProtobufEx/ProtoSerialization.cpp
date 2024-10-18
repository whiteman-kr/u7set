#include "ProtoSerialization.h"

namespace Proto
{
	bool ParseFromIstream(::google::protobuf::Message& message, std::fstream& stream)
	{
		return message.ParseFromIstream(&stream);
	}

	bool ParseFromString(::google::protobuf::Message& message, const char* str)
	{
		return message.ParseFromString(str);
	}

	bool ParseFromArray(::google::protobuf::Message& message, const QByteArray& data)
	{
		return message.ParseFromArray(data.constData(), data.size());
	}

}

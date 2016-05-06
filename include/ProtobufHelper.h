#pragma once

#include <QByteArrayData>

#include "../Proto/serialization.pb.h"


template <typename MESSAGE>
class ProtobufSerializable
{
private:
	MESSAGE m_msg;

public:
	virtual bool setProtobufMessageRequiredFields(MESSAGE& msg) = 0;		// must be implemented in derived classes

	virtual bool setProtobufMessageAllFields(MESSAGE& msg) { return setRequiredFields(msg); }

	MESSAGE& protobufMessage() { return m_msg; }
};


template <typename TYPE, typename PROTO_TYPE>
class ProtobufSerializator : public PROTO_TYPE
{
private:
	const TYPE& m_object;

public:
	ProtobufSerializator(const TYPE& object) : m_object(object) {}

	bool setProtobufMessageRequiredFields()
	{
		// TYPE::setProtobufMessageRequiredFields(PROTO_TYPE& message)
		// must be implemented in TYPE class
		//
		return m_object.setProtobufMessageRequiredFields(*this);
	}

	bool setProtobufMessageAllFields()
	{
		// TYPE::setProtobufMessageAllFields(PROTO_TYPE& message)
		// must be implemented in TYPE class
		//
		return m_object.setProtobufMessageAllFields(*this);
	}
};


typedef google::protobuf::Message ProtobufMessage;

class ProtobufHelper
{
protected:
	static const int FINALIZER;

	char* m_buffer = nullptr;
	int m_bufferSize = 0;

public:
	bool bufferSize() const { return m_bufferSize; }
	char* getBuffer() const { return m_buffer; }
};


class ProtobufWriter : public ProtobufHelper
{
private:
	int m_writeIndex = 0;

	bool m_finalized = false;
	bool m_nativeBuffer = false;

	inline bool haveEnouthSpace(ProtobufMessage* message) { return haveEnouthSpace(message->ByteSize()); }
	inline bool haveEnouthSpace(int messageSize);

	bool serializeMessage(int messageSize, ProtobufMessage* message);

	bool reallocateBuffer(int messageSize);

public:
	ProtobufWriter(char* buffer, int bufferSize);
	ProtobufWriter();
	~ProtobufWriter();

	bool serialize(ProtobufMessage* message);
	bool finalize();

	bool currentSize() const { return m_writeIndex; }
};


class ProtobufReader : public ProtobufHelper
{
private:
	int m_readIndex = 0;

public:
	ProtobufReader(char* buffer, int bufferSize);

	bool parse(ProtobufMessage* message);
};


#include "../lib/ProtobufHelper.h"


const int ProtobufHelper::FINALIZER = 0;


ProtobufWriter::ProtobufWriter(char* buffer, int bufferSize) :
	m_nativeBuffer(false)
{
	if (buffer == nullptr ||
		bufferSize <= 0)
	{
		assert(false);
		return;
	}

	m_buffer = buffer;
	m_bufferSize = bufferSize;
}


ProtobufWriter::ProtobufWriter() :
	m_nativeBuffer(true)
{
	m_bufferSize = 1024;
	m_buffer = new char [m_bufferSize];
}


ProtobufWriter::~ProtobufWriter()
{
	if (m_nativeBuffer == true && m_buffer != nullptr)
	{
		delete [] m_buffer;
	}
}


bool ProtobufWriter::reallocateBuffer(int messageSize)
{
	if (m_nativeBuffer == false)
	{
		assert(false);						// can reallocate native buffer only
		return false;
	}

	char* tmp = m_buffer;					// save pointer to old buffer
	int oldBufferSize = m_bufferSize;

	m_bufferSize *= 2;						// double buffer size

	if (m_bufferSize < oldBufferSize + messageSize + sizeof(int)*2)
	{
		m_bufferSize = oldBufferSize + messageSize + sizeof(int)*2;
	}

	m_buffer = new char [m_bufferSize];		// allocate expanded buffer

	memcpy(m_buffer, tmp, m_writeIndex);	// copy data from old to new buffer

	delete [] tmp;							// free old buffer

	return true;
}


bool ProtobufWriter::serializeMessage(int messageSize, ProtobufMessage* message)
{
	// write message size
	//
	*reinterpret_cast<int*>(m_buffer + m_writeIndex) = messageSize;
	m_writeIndex += sizeof(int);

	// write message
	//
	message->SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_buffer + m_writeIndex));
	m_writeIndex += messageSize;

	return true;
}


bool ProtobufWriter::serialize(ProtobufMessage* message)
{
	if (message == nullptr)
	{
		assert(false);
		return false;
	}

	if (m_finalized == true)
	{
		assert(false);
		return false;
	}

	int messageSize = message->ByteSize();

	if (haveEnouthSpace(messageSize) == true)
	{
		return serializeMessage(messageSize, message);
	}

	// has not enouth space in buffer
	//
	if (m_nativeBuffer == false)
	{
		// can't reallocate non-native buffer
		//
		return false;
	}

	if (reallocateBuffer(messageSize) == false)
	{
		return false;
	}

	// verify accessible space again
	//
	if (haveEnouthSpace(messageSize) == false)
	{
		// WTF?
		//
		assert(false);
		return false;
	}

	return serializeMessage(messageSize, message);
}


bool ProtobufWriter::finalize()
{
	if (m_finalized == true)
	{
		assert(false);
		return false;
	}

	if (m_writeIndex + sizeof(int) > m_bufferSize)
	{
		// WTF?
		//
		assert(false);
		return false;
	}

	*reinterpret_cast<int*>(m_buffer + m_writeIndex) = FINALIZER;
	m_writeIndex += sizeof(int);

	m_finalized = true;

	return true;
}


bool ProtobufReader::parse(ProtobufMessage* message)
{
	if (message == nullptr)
	{
		assert(false);
		return false;
	}

	int messageSize = *reinterpret_cast<int*>(m_buffer + m_readIndex);

	if (messageSize == 0)
	{
		return false;
	}

	m_readIndex += sizeof(int);

	return message->ParseFromArray(m_buffer + m_readIndex, messageSize);
}


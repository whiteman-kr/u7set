#pragma once

#include <fstream>
#include <memory>

#include <QtCore/QUuid>
#include <QVariant>

#include "PropertyObject.h"

#ifdef Q_OS_WIN
#pragma warning (push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4125)
#pragma warning(disable : 6011)
#pragma warning(disable : 4267)
#pragma warning(disable : 4512)
#pragma warning(disable : 4127)
#pragma warning(disable : 4996)
#endif // Q_OS_WIN

#include "../Proto/serialization.pb.h"
#ifdef Q_OS_WIN
#pragma warning (pop)
#endif // Q_OS_WIN


namespace Proto
{
	bool ParseFromIstream(::google::protobuf::Message& message, std::fstream& stream);
	bool ParseFromString(::google::protobuf::Message& message, const char* str);
	bool ParseFromArray(::google::protobuf::Message& message, const QByteArray& data);


	enum class ProtoCompress
	{
		Auto,
		Always,
		Never
	};


	// Template and imlementation seriazlization functions
	// VFrameType must have functions CreateObject, SaveData, LoadData
	//
	template <typename VFrameType>
	class ObjectSerialization
	{
	public:
		ObjectSerialization(ProtoCompress compression = ProtoCompress::Auto, size_t autoCompressionLimit = 4096) noexcept:
			m_compression(compression),
			m_autoCompressionLimit(autoCompressionLimit)
		{
		}

		bool saveToFile(const QString& fileName) const
		{
			std::fstream output(fileName.toStdString(), std::ios::out | std::ios::binary);
			if (output.is_open() == false || output.bad() == true)
			{
				return false;
			}

			return saveToStream(output);
		}
		bool saveToFile(const wchar_t* fileName) const
		{
			QString fn = QString::fromWCharArray(fileName);

			std::fstream output(fn.toStdString(), std::ios::out | std::ios::binary);
			if (output.is_open() == false || output.bad() == true)
			{
				return false;
			}

			return saveToStream(output);
		}
		bool saveToStream(std::fstream& stream) const
		{
			if (stream.is_open() == false || stream.bad() == true)
			{
				return false;
			}

			Proto::Envelope message;

			bool result = Save(&message);
			if (result == false)
			{
				return false;
			}

			try
			{
				switch (m_compression)
				{
				case ProtoCompress::Auto:
				case ProtoCompress::Always:
					{
						std::string serializedString;
						serializedString.reserve(255000);

						bool ok = message.SerializeToString(&serializedString);
						if (ok == false)
						{
							return false;
						}

						if (serializedString.size() >= m_autoCompressionLimit ||
							m_compression == ProtoCompress::Always)
						{
							// Compress
							//

							// The bytes are not copied!!!, keep serializedString alive
							//
							QByteArray ba = QByteArray::fromRawData(serializedString.data(), static_cast<int>(serializedString.size()));
							QByteArray compressedData = qCompress(ba, -1);

							// If compressed size is somehow bigger than uncompressed, just leave it uncompressed
							//
							if (static_cast<size_t>(compressedData.size()) >= serializedString.size())
							{
								stream.write(serializedString.data(), serializedString.size());
								return true;
							}

							// Create another Envelope, with compressed data,
							//
							Proto::Envelope compressMessage;

							compressMessage.set_classnamehash(message.classnamehash());
							compressMessage.set_compressedobject(compressedData.constData(), compressedData.size());

							return compressMessage.SerializeToOstream(&stream);
						}
						else
						{
							// Do not compress
							//
							stream.write(serializedString.data(), serializedString.size());
							return stream.rdstate() == std::ios_base::goodbit;
						}
					}

				case ProtoCompress::Never:
					return message.SerializeToOstream(&stream);
				default:
					assert(false);
					return false;
				}
			}
			catch(...)
			{
				assert(false);
				return false;
			}
		}
		bool saveToByteArray(QByteArray* data) const
		{
			Proto::Envelope message;
			this->SaveData(&message);

			switch (m_compression)
			{
			case ProtoCompress::Auto:
			case ProtoCompress::Always:
				{
					std::string serializedString;
					serializedString.reserve(255000);

					bool ok = message.SerializeToString(&serializedString);
					if (ok == false)
					{
						return false;
					}

					if (serializedString.size() >= m_autoCompressionLimit ||
						m_compression == ProtoCompress::Always)
					{
						// Compress
						//

						// The bytes are not copied !!!, keep serializedString alive
						//
						QByteArray ba = QByteArray::fromRawData(serializedString.data(), static_cast<int>(serializedString.size()));
						QByteArray compressedData = qCompress(ba, -1);

						// If compressed size is somehow bigger than uncompressed, just leave it uncompressed
						//
						if (static_cast<size_t>(compressedData.size()) >= serializedString.size())
						{
							*data = QByteArray(serializedString.data(), static_cast<int>(serializedString.size()));
							return true;
						}

						// Create another Envelope, with compressed data,
						//
						Proto::Envelope compressMessage;

						compressMessage.set_classnamehash(message.classnamehash());
						compressMessage.set_compressedobject(compressedData.constData(), compressedData.size());

						std::string str = compressMessage.SerializeAsString();
						*data = QByteArray(str.data(), static_cast<int>(str.size()));

						return true;
					}
					else
					{
						// Do not compress
						//
						*data = QByteArray(serializedString.data(), static_cast<int>(serializedString.size()));
						return true;
					}
				}
				break;

			case ProtoCompress::Never:
				{
					std::string str = message.SerializeAsString();
					*data = QByteArray(str.data(), static_cast<int>(str.size()));
					return true;
				}

			default:
				assert(false);
				return false;
			}
		}
		bool Save(Proto::Envelope* message) const
		{
			try
			{
				return this->SaveData(message);
			}
			catch (...)
			{
				assert(false);
				return false;
			}
		}

		[[nodiscard]] bool Load(const QString& fileName)
		{
			std::fstream input(fileName.toStdString(), std::ios::in | std::ios::binary);
			if (input.is_open() == false || input.bad() == true)
			{
				return false;
			}

			return Load(input);
		}
		bool Load(const wchar_t* fileName)
		{
			QString fn = QString::fromWCharArray(fileName);

			std::fstream input(fn.toStdString(), std::ios::in | std::ios::binary);
			if (input.is_open() == false || input.bad() == true)
			{
				return false;
			}

			return Load(input);
		}
		bool Load(std::fstream& stream)
		{
			if (stream.is_open() == false || stream.bad() == true)
			{
				return false;
			}

			Proto::Envelope message;

			bool result = ParseFromIstream(message, stream);
			if (result == false)
			{
				return false;
			}

			return Load(message);
		}
		bool Load(const QByteArray& data)
		{
			Proto::Envelope message;

			bool result = ParseFromArray(message, data);
			if (result == false)
			{
				return false;
			}

			return Load(message);
		}
		bool Load(const Proto::Envelope& message)
		{
			try
			{
				// This code disables emitting propertyListChanged if object is propertyObject
				//
				PropertyObject* propertyObject = dynamic_cast<PropertyObject*>(this);
				if (propertyObject != nullptr)
				{
					propertyObject->blockSignals(true);
				}

				std::shared_ptr<int> emitPropertyListChanged(nullptr, [propertyObject](void*)
					{
						if (propertyObject != nullptr)
						{
							propertyObject->blockSignals(false);
						}
					});

				//--
				//
				if (message.has_compressedobject() == true)
				{
					// it is compressed Envelope, uncompress it and after it it will be possible to use it
					//
					const std::string& compressedString = message.compressedobject();
					QByteArray uncompressedData = qUncompress(reinterpret_cast<const uchar*>(compressedString.data()), static_cast<int>(compressedString.size()));

					Proto::Envelope uncompressedMessage;

					bool result = ParseFromArray(uncompressedMessage, uncompressedData);
					if (result == false)
					{
						return false;
					}

					return this->LoadData(uncompressedMessage);
				}
				else
				{
					return this->LoadData(message);
				}
			}
			catch (...)
			{
				assert(false);
				return false;
			}
		}

		static std::shared_ptr<VFrameType> Create(QString fileName)
		{
			return Create(fileName.toStdWString().data());
		}

		static std::shared_ptr<VFrameType> Create(const wchar_t* fileName)
		{
			QString fn = QString::fromWCharArray(fileName);

			std::fstream input(fn.toStdString(), std::ios::in | std::ios::binary);
			if (input.bad() == true)
			{
				return nullptr;
			}

			return Create(input);
		}
		static std::shared_ptr<VFrameType> Create(std::fstream& stream)
		{
			if (stream.bad() == true)
			{
				return nullptr;
			}

			Proto::Envelope message;

			bool result = ParseFromIstream(message, stream);
			if (result == false)
			{
				return nullptr;
			}

			return Create(message);
		}
		static std::shared_ptr<VFrameType> Create(const QByteArray& data)
		{
			Proto::Envelope message;

			bool result = ParseFromArray(message, data);
			if (result == false)
			{
				return nullptr;
			}

			return Create(message);
		}
		static std::shared_ptr<VFrameType> Create(const Proto::Envelope& message)
		{
			if (message.has_compressedobject() == true)
			{
				// it is compressed Envelope, uncompress it and after it it will be possible to use it
				//
				const std::string& compressedString = message.compressedobject();
				QByteArray uncompressedData = qUncompress(reinterpret_cast<const uchar*>(compressedString.data()), static_cast<int>(compressedString.size()));

				Proto::Envelope uncompressedMessage;

				bool result = ParseFromArray(uncompressedMessage, uncompressedData);
				if (result == false)
				{
					return nullptr;
				}

				std::shared_ptr<VFrameType> newItem = VFrameType::CreateObject(uncompressedMessage);
				assert(newItem != nullptr);

				return newItem;
			}
			else
			{
				std::shared_ptr<VFrameType> newItem = VFrameType::CreateObject(message);
				assert(newItem != nullptr);

				return newItem;
			}
		}

	protected:
		virtual bool SaveData(Proto::Envelope* message) const = 0;
		virtual bool LoadData(const Proto::Envelope& message) = 0;

	public:
		ProtoCompress compression() const
		{
			return m_compression;
		}
		void setCompression(ProtoCompress value)
		{
			m_compression = value;
		}

		size_t autoCompressionLimit() const
		{
			return m_autoCompressionLimit;
		}
		void setAutoCompressionLimit(size_t value)
		{
			m_autoCompressionLimit = value;
		}

	private:
		ProtoCompress m_compression = ProtoCompress::Auto;
		size_t m_autoCompressionLimit = 4096;
	};

	// Helper serialization functions
	//
	const QUuid& Read(const Proto::Uuid& message);
	void Write(Proto::Uuid* pMessage, const QUuid& guid);

	// Read/write wstring message
	//
	void Read(const Proto::wstring& message, QString* dst);
	void Write(Proto::wstring* pMessage, const QString& str);

	// Read/write qvariant message
	//
	const QVariant Read(const Proto::qvariant& message);
	void Write(Proto::qvariant* pMessage, const QVariant& value);

	void saveProperty(::Proto::Property* protoProperty, const std::shared_ptr<::Property>& property);
	void saveProperty(::Proto::Property* protoProperty, const ::Property* property);

	bool loadProperty(const ::Proto::Property& protoProperty, const std::shared_ptr<::Property>& property);
	bool loadProperty(const ::Proto::Property& protoProperty, ::Property* property);
}




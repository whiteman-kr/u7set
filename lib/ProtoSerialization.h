#pragma once

#include <fstream>
#include <memory>

#include <QtCore/QUuid>
#include <QVariant>


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


class Property;

namespace Proto
{
	bool ParseFromIstream(::google::protobuf::Message& message, std::fstream& stream);
	bool ParseFromString(::google::protobuf::Message& message, const char* str);
	bool ParseFromArray(::google::protobuf::Message& message, const QByteArray& data);



	// Шаблон и реализация необходимых фукнций сериализации
	// у VFrameType должны быть реализованы функции CreateObject, SaveData, LoadData
	//
	template <typename VFrameType>
	class ObjectSerialization
	{
	public:
		bool Save(const QString& fileName) const
		{
			std::wstring wfnstr(fileName.toStdWString());
			std::string fnstr(wfnstr.begin(), wfnstr.end());

			std::fstream output(fnstr, std::ios::out | std::ios::binary);
			if (output.is_open() == false || output.bad() == true)
			{
				assert(false);
				return false;
			}

			return Save(output);
		}
		bool Save(const wchar_t* fileName) const
		{
			std::wstring wfnstr(fileName);
			std::string fnstr(wfnstr.begin(), wfnstr.end());

			std::fstream output(fnstr, std::ios::out | std::ios::binary);
			if (output.is_open() == false || output.bad() == true)
			{
				assert(false);
				return false;
			}

			return Save(output);
		}
		bool Save(std::fstream& stream) const
		{
			if (stream.is_open() == false || stream.bad() == true)
			{
				assert(false);
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
				return message.SerializeToOstream(&stream);
			}
			catch(...)
			{
				assert(false);
				return false;
			}
		}
		bool Save(QByteArray& data) const
		{
			Proto::Envelope message;
			this->SaveData(&message);

			std::string str = message.SerializeAsString();
			data = QByteArray(str.data(), static_cast<int>(str.size()));
			return true;
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

		bool Load(const QString& fileName)
		{
			std::wstring wfnstr(fileName.toStdWString());
			std::string fnstr(wfnstr.begin(), wfnstr.end());

			std::fstream input(fnstr, std::ios::in | std::ios::binary);
			if (input.is_open() == false || input.bad() == true)
			{
				assert(false);
				return false;
			}

			return Load(input);
		}
		bool Load(const wchar_t* fileName)
		{
			std::wstring wfnstr(fileName);
			std::string fnstr(wfnstr.begin(), wfnstr.end());

			std::fstream input(fnstr, std::ios::in | std::ios::binary);
			if (input.is_open() == false || input.bad() == true)
			{
				assert(false);
				return false;
			}

			return Load(input);
		}
		bool Load(std::fstream& stream)
		{
			if (stream.is_open() == false || stream.bad() == true)
			{
				assert(false);
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
				return this->LoadData(message);
			}
			catch (...)
			{
				assert(false);
				return false;
			}
		}

		static std::shared_ptr<VFrameType> Create(const wchar_t* fileName)
		{
			std::wstring wfnstr(fileName);
			std::string fnstr(wfnstr.begin(), wfnstr.end());

			std::fstream input(fnstr, std::ios::in | std::ios::binary);
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

			std::shared_ptr<VFrameType> newItem = VFrameType::CreateObject(message);
			assert(newItem != nullptr);

			return newItem;
		}
		static std::shared_ptr<VFrameType> Create(const QByteArray& data)
		{
			Proto::Envelope message;

			bool result = ParseFromArray(message, data);
			if (result == false)
			{
				return nullptr;
			}

			std::shared_ptr<VFrameType> newItem = VFrameType::CreateObject(message);
			assert(newItem != nullptr);

			return newItem;
		}
		static std::shared_ptr<VFrameType> Create(const Proto::Envelope& message)
		{
			// function "static VFrameType* CreateObject(const Proto::Envelope& message)"
			// must be defined in VFrameType
			//
			return VFrameType::CreateObject(message);
		}

	protected:
		virtual bool SaveData(Proto::Envelope* message) const = 0;
		virtual bool LoadData(const Proto::Envelope& message) = 0;
	};


	// Функции для сериализации данных
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




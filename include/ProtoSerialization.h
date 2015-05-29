#pragma once

#include <QtCore/QUuid>
#include <fstream>

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


#include "StreamedData.h"


namespace Proto
{
	// ������ � ���������� ����������� ������� ������������
	// � VFrameType ������ ���� ����������� ������� CreateObject, SaveData, LoadData
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
		bool Save(Proto::StreamedData& data) const
		{
			Proto::Envelope message;
			this->SaveData(&message);

			auto mutable_data = data.mutable_data();
			auto str = message.SerializeAsString();

			mutable_data = QByteArray(str.data(), static_cast<int>(str.size()));

			return true;
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

			bool result = message.ParseFromIstream(&stream);
			if (result == false)
			{
				return false;
			}

			return Load(message);
		}
		bool Load(const Proto::StreamedData& data)
		{
			Proto::Envelope message;

			bool result = message.ParseFromString(data.data());
			if (result == false)
			{
				return false;
			}

			return Load(message);
		}
		bool Load(const QByteArray& data)
		{
			Proto::Envelope message;

			bool result = message.ParseFromArray(data.data(), data.size());
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

		static VFrameType* Create(const wchar_t* fileName)
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
		static VFrameType* Create(std::fstream& stream)
		{
			if (stream.bad() == true)
			{
				return nullptr;
			}

			Proto::Envelope message;

			bool result = message.ParseFromIstream(&stream);
			if (result == false)
			{
				return nullptr;
			}

			VFrameType* pNewItem = VFrameType::CreateObject(message);
			assert(pNewItem != nullptr);

			return pNewItem;
		}
		static VFrameType* Create(const Proto::StreamedData& data)
		{
			Proto::Envelope message;

			bool result = message.ParseFromString(data.data());
			if (result == false)
			{
				return nullptr;
			}

			VFrameType* pNewItem = VFrameType::CreateObject(message);
			assert(pNewItem != nullptr);

			return pNewItem;
		}
		static VFrameType* Create(const QByteArray& data)
		{
			Proto::Envelope message;

			bool result = message.ParseFromArray(data.constData(), data.size());
			if (result == false)
			{
				return nullptr;
			}

			VFrameType* pNewItem = VFrameType::CreateObject(message);
			assert(pNewItem != nullptr);

			return pNewItem;
		}
		static VFrameType* Create(const Proto::Envelope& message)
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


	// ������� ��� ������������ ������
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
}




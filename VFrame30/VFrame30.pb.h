#pragma once

#include <QtCore/QUuid>
#include <fstream>
#include "DebugInstCounter.h"

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
#include "../VFrame30/VideoFrame.pb.h"
#ifdef Q_OS_WIN
#pragma warning (pop)
#endif // Q_OS_WIN


// Forward declarations
//
namespace Fbl
{
	class FblElement;
}

namespace VFrame30
{ 
	class CVideoFrame;
	class CVideoLayer;
	class CVideoItem;
	class Configuration;
}


namespace VFrame30
{
	namespace Proto
	{
		template <typename VFrameType> class CVFrameObjectSerialization;	// forwaed declaration for CStreamedData 

		// Класс для сериализации
		//
		class VFRAME30LIBSHARED_EXPORT CStreamedData :
			public DebugInstCounter<CStreamedData>
		{
			// Эти классы непосредтвенно стучат к data.
			//
			friend class CVFrameObjectSerialization<VFrame30::CVideoFrame>;
			friend class CVFrameObjectSerialization<VFrame30::CVideoLayer>;
			friend class CVFrameObjectSerialization<VFrame30::CVideoItem>;
			friend class CVFrameObjectSerialization<VFrame30::Configuration>;
			friend class CVFrameObjectSerialization<Fbl::FblElement>;

		public:
			CStreamedData();
			CStreamedData(const char* pSrc, size_t size);			// создать объект и проинницализировать data данными из src
			explicit CStreamedData(const QByteArray& src);			// создать объект и проинницализировать data данными из src
			explicit CStreamedData(size_t capacity);

			virtual ~CStreamedData();

		private:
			CStreamedData(const CStreamedData&);				// Запрещена к использованию
			CStreamedData& operator= (const CStreamedData&);	// Запрещена к использованию

		public:
			const char* data() const;		// Получить указатель на даннные, указатель не сохранять, данные не изменять.
			size_t length() const;			// Получить размер данных (GetData()) в байтах

			void clear();					// clear data

		protected:
			std::string m_data;
		};


		// Шаблон и реализация необходимых фукнций сериализации
		// у VFrameType должны быть реализованы функции CreateObject, SaveData, LoadData
		//
		template <typename VFrameType>
		class CVFrameObjectSerialization
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

				VFrame30::Proto::Envelope message;

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
			bool Save(CStreamedData& data) const
			{
				Envelope message;
				this->SaveData(&message);

				data.m_data = message.SerializeAsString();
				return true;
			}
			bool Save(QByteArray& data) const
			{
				Envelope message;
				this->SaveData(&message);

				std::string str = message.SerializeAsString();
				data = QByteArray(str.data(), static_cast<int>(str.size()));
				return true;
			}
			bool Save(VFrame30::Proto::Envelope* message) const
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

				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromIstream(&stream);
				if (result == false)
				{
					return false;
				}

				return Load(message);
			}
			bool Load(const VFrame30::Proto::CStreamedData& data)
			{
				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromString(data.m_data);
				if (result == false)
				{
					return false;
				}

				return Load(message);
			}
			bool Load(const QByteArray& data)
			{
				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromArray(data.data(), data.size());
				if (result == false)
				{
					return false;
				}

				return Load(message);
			}
			bool Load(const VFrame30::Proto::Envelope& message)
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

				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromIstream(&stream);
				if (result == false)
				{
					return nullptr;
				}

				VFrameType* pNewItem = VFrameType::CreateObject(message);
				assert(pNewItem != nullptr);

				return pNewItem;
			}
			static VFrameType* Create(const VFrame30::Proto::CStreamedData& data)
			{
				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromString(data.m_data);
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
				VFrame30::Proto::Envelope message;

				bool result = message.ParseFromArray(data.data(), data.size());
				if (result == false)
				{
					return nullptr;
				}

				VFrameType* pNewItem = VFrameType::CreateObject(message);
				assert(pNewItem != nullptr);

				return pNewItem;
			}
			static VFrameType* Create(const VFrame30::Proto::Envelope& message)
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
		const QUuid& Read(const VFrame30::Proto::Guid& message);
		void Write(VFrame30::Proto::Guid* pMessage, const QUuid& guid);

		// Read/write wstring message
		//
		QString Read(const VFrame30::Proto::wstring& message);
		void Write(VFrame30::Proto::wstring* pMessage, const QString& str);
	}
}



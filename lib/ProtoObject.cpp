#include "../include/ProtoObject.h"

/*
	// Функции для сериализации данных
	//
	const QUuid& Read(const Proto::Guid& message)
	{
		return *(reinterpret_cast<const QUuid*>(message.guid().c_str()));
	}

	void Write(Proto::Guid* pMessage, const QUuid& guid)
	{
		if (pMessage == nullptr)
		{
			assert(pMessage != nullptr);
			return;
		}
		pMessage->set_guid(&guid, sizeof(guid));
	}

	// Read/write wstring message
	//
	QString Read(const Proto::wstring& message)
	{
		QString qstr = QString::fromUtf16(reinterpret_cast<const ushort*>(message.text().data()),
										  static_cast<int>(message.text().size() / 2) - 1);
		return qstr;
	}

	void Write(Proto::wstring* pMessage, const QString& str)
	{
		assert(sizeof(wchar_t) == 2);
		//static_assert(sizeof(wchar_t) == 2, "wchar_t must be 16-bit.");

		if (pMessage == nullptr)
		{
			assert(pMessage != nullptr);
			return;
		}

		pMessage->set_text(str.toStdWString().c_str(), (str.length() + 1) * sizeof(wchar_t));
	}*/

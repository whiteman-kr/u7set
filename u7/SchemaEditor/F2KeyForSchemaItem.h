#pragma once

namespace EditEngine
{
	class EditEngine;
}

namespace VFrame30
{
	class SchemaItem;
}

namespace AppSignalLib
{
	class Bus;
}

// Quick schema item property editor, activated by F2 key
//
class F2KeyForSchemaItem : public QObject
{
	Q_OBJECT

public:
	using SchemaItemPtr = std::shared_ptr<VFrame30::SchemaItem>;

	explicit F2KeyForSchemaItem(DbController* db, EditEngine::EditEngine* editEngine, QWidget* view, QWidget* parent);

	void show(SchemaItemPtr schemaItem);

	bool f2KeyForReceiver(SchemaItemPtr item, bool setViaEditEngine);
	bool f2KeyForTransmitter(SchemaItemPtr item, bool setViaEditEngine);

	static bool loadBusses(DbController* db, std::vector<AppSignalLib::Bus>* out, QWidget* parentWidget);

private:
	void f2KeyForRect(SchemaItemPtr item);
	void f2KeyForConst(SchemaItemPtr item);
	void f2KeyForSignal(SchemaItemPtr item);
	void f2KeyForLoopback(SchemaItemPtr item);
	void f2KeyForValue(SchemaItemPtr item);
	void f2KeyForImageValue(SchemaItemPtr item);
	void f2KeyForBus(SchemaItemPtr item);
	void f2KeyForAfb(SchemaItemPtr item);
	void f2KeyForVduValue(SchemaItemPtr item);
	void f2KeyForVduRect(SchemaItemPtr item);
	void f2KeyForVduImageValue(SchemaItemPtr item);
	void f2KeyForPushButton(SchemaItemPtr item);
	void f2KeyForLineEdit(SchemaItemPtr item);

private:
	DbController* m_db;
	EditEngine::EditEngine* m_editEngine = nullptr;
	QWidget* m_view = nullptr;
	QWidget* m_parent = nullptr;

public:
	inline static QString m_lastUsedLoopbackId;
};
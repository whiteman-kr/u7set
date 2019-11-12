#pragma once

#include "../VFrame30/SchemaItem.h"

class EditSchemaView;

namespace VFrame30
{
	class SchemaLayer;
	class Schema;
}

namespace EditEngine
{
	enum class SetOrder
	{
		BringToFront,
		BringForward,
		SendToBack,
		SendBackward
	};

	class EditCommand;

	//
	//
	// EditEngine - Класс обеспечивающий выполнение всех команд изменения SchemaView (Undo )
	//
	//
	class EditEngine : public QObject
	{
		Q_OBJECT

	private:
		EditEngine();		// deleted;

	public:
		EditEngine(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar, QObject* parent);
		virtual ~EditEngine();

	public:
		void reset();

	public:
		bool addCommand(std::shared_ptr<EditCommand> command, bool runCommand);

		[[nodiscard]] bool startBatch();			// For performing several commands in a time
		bool endBatch(bool runCommands = true);

		void redo();
		void undo();

		bool canUndo() const;
		bool canRedo() const;

		bool readOnly() const;
		void setReadOnly(bool value);

		bool modified() const;
		void setModified();
		void resetModified();

	private:
		void execute(std::shared_ptr<EditCommand> command);
		void unExecute(std::shared_ptr<EditCommand> command);

		void selectItems(const std::vector<SchemaItemPtr>& items);

	public:
		void runAddItem(std::list<SchemaItemPtr> items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runAddItem(std::vector<SchemaItemPtr> items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runAddItem(SchemaItemPtr item, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runDeleteItem(const std::vector<SchemaItemPtr>& items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runDeleteItem(SchemaItemPtr item, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runSetPoints(const std::vector<std::vector<VFrame30::SchemaPoint>>& points, const std::vector<SchemaItemPtr>& items, bool selectChangedItems);
		void runSetPoints(const std::vector<VFrame30::SchemaPoint>& points, const SchemaItemPtr& item, bool selectChangedItems);

		void runMoveItem(double xdiff, double ydiff, const std::vector<SchemaItemPtr>& items, bool snapToGrid);
		void runMoveItem(double xdiff, double ydiff, const SchemaItemPtr& item, bool snapToGrid);

		void runSetOrder(SetOrder setOrder, const std::vector<SchemaItemPtr>& items, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runSetProperty(const QString& propertyName, QVariant value, const std::vector<SchemaItemPtr>& items);
		void runSetProperty(const QString& propertyName, QVariant value, const SchemaItemPtr& item);

		void runSetObject(const QByteArray& currentState, const QByteArray& newState, const std::vector<SchemaItemPtr>& items);
		void runSetObject(const QByteArray& currentState, const QByteArray& newState, const SchemaItemPtr& item);

		void runSetSchemaProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::Schema>& schema);

		void runNopItem(const std::vector<SchemaItemPtr>& items);
		void runNopItem(const SchemaItemPtr& item);

	signals:
		void stateChanged(bool canUndo, bool canRedo);
		void modifiedChanged(bool modified);
		void propertiesChanged();

		// Data
		//
	private:
		static const int MaxCommandCount = 2048;

		EditSchemaView* m_schemaView = nullptr;
		QScrollBar* m_hScrollBar = nullptr;
		QScrollBar* m_vScrollBar = nullptr;

		std::vector<std::shared_ptr<EditCommand>> m_commands;
		int m_current = 0;

		bool m_readOnly = false;
		bool m_modified = false;

		bool m_batch = false;
		std::vector<std::shared_ptr<EditCommand>> m_batchCommands;
	};


	//
	//
	// EditCommand
	//
	//
	class EditCommand
	{
	public:
		EditCommand() = delete;		// use EditCommand(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);
		EditCommand(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	public:
		void execute(std::vector<SchemaItemPtr>* itemsToSelect);
		void unExecute(std::vector<SchemaItemPtr>* itemsToSelect);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) = 0;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) = 0;

		void saveViewPos();
		void restoreViewPos();

		// Data
		//
	protected:
		std::shared_ptr<VFrame30::SchemaLayer> m_activeLayer;		// Active Layer on operation start

		EditSchemaView* m_schemaView = nullptr;
		QScrollBar* m_hScrollBar = nullptr;
		QScrollBar* m_vScrollBar = nullptr;

		double m_zoom;

		QScrollBar m_hScrollBarCopy;
		QScrollBar m_vScrollBarCopy;

		//--
		friend class BatchCommand;
	};
}


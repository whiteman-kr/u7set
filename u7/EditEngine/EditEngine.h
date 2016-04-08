#pragma once

#include "../VFrame30/SchemaItem.h"

class EditSchemaView;

namespace VFrame30
{
	class SchemaLayer;
	class Schema;
}

enum class SetOrder
{
	BringToFront,
	BringForward,
	SendToBack,
	SendBackward
};

namespace EditEngine
{
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

		void redo(int levels);
		void undo(int levels);

		bool canUndo() const;
		bool canRedo() const;

		bool readOnly() const;
		void setReadOnly(bool value);

		bool modified() const;
		void setModified();
		void resetModified();

	public:
		void runAddItem(std::list<std::shared_ptr<VFrame30::SchemaItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runAddItem(std::vector<std::shared_ptr<VFrame30::SchemaItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runAddItem(std::shared_ptr<VFrame30::SchemaItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runDeleteItem(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, std::shared_ptr<VFrame30::SchemaLayer> layer);
		void runDeleteItem(std::shared_ptr<VFrame30::SchemaItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runSetPoints(const std::vector<std::vector<VFrame30::SchemaPoint>>& points, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items);
		void runSetPoints(const std::vector<VFrame30::SchemaPoint>& points, const std::shared_ptr<VFrame30::SchemaItem>& item);

		void runMoveItem(double xdiff, double ydiff, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, bool snapToGrid);
		void runMoveItem(double xdiff, double ydiff, const std::shared_ptr<VFrame30::SchemaItem>& item, bool snapToGrid);

		//void runSetOrder(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, std::shared_ptr<VFrame30::SchemaLayer> layer);

		void runSetProperty(const QString& propertyName, QVariant value, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items);
		void runSetProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::SchemaItem>& item);

		void runSetSchemaProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::Schema>& schema);

//		/// <summary>
//		/// Команда "Изменить размер"
//		/// </summary>
//		/// <param name="width">размер объетка по горизонтали (может быть меньше 0)</param>
//		/// <param name="height">размер объетка по вертикали (может быть меньше 0)</param>
//		/// <param name="items">Список объектов для выполнения команды</param>
//		public void RunResizeCommand(double width, double height, List<VFrame30Ext.SchemaItem> items)
//		{
//			AddCommand(new ResizeSchemaItemCommand<SchemaType>(EditSchemaView, width, height, items, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "Изменить размер"
//		/// </summary>
//		/// <param name="size">список размеров объектов (может быть меньше 0)</param>
//		/// <param name="items">Список объектов для выполнения команды</param>
//		public void RunResizeCommand(List<Tuple<double, double>> size, List<VFrame30Ext.SchemaItem> items)
//		{
//			AddCommand(new ResizeSchemaItemCommand<SchemaType>(EditSchemaView, size, items, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "Изменить свойства"
//		/// </summary>
//		/// <param name="item">schema item</param>
//		/// <param name="oldItem">old value</param>
//		/// <param name="newItem">new value</param>
//		public void RunSetSchemaItemProperties(VFrame30Ext.SchemaItem item, VFrame30Ext.StreamedData oldItem, VFrame30Ext.StreamedData newItem)
//		{
//			List<VFrame30Ext.SchemaItem> items = new List<VFrame30Ext.SchemaItem>(1);
//			items.Add(item);

//			List<VFrame30Ext.StreamedData> oldSd = new List<VFrame30Ext.StreamedData>(1);
//			oldSd.Add(oldItem);

//			List<VFrame30Ext.StreamedData> newSd = new List<VFrame30Ext.StreamedData>(1);
//			newSd.Add(newItem);

//			AddCommand(new SetSchemaItemPropertiesCommand<SchemaType>(EditSchemaView, items, oldSd, newSd, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "Изменить свойства"
//		/// </summary>
//		/// <param name="items">спиосок ссылок на элементы для установки свойств</param>
//		/// <param name="oldItems">список старых значений элементов</param>
//		/// <param name="newItems">список новых значений элементов</param>
//		public void RunSetSchemaItemProperties(List<VFrame30Ext.SchemaItem> items, List<VFrame30Ext.StreamedData> oldItems, List<VFrame30Ext.StreamedData> newItems)
//		{
//			AddCommand(new SetSchemaItemPropertiesCommand<SchemaType>(EditSchemaView, items, oldItems, newItems, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "На передний план"
//		/// </summary>
//		/// <param name="items">спиосок ссылок на элементы для установки свойств</param>
//		public void RunBringToFrontSchemaItem(List<VFrame30Ext.SchemaItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemaItemCommand<SchemaType>(EditSchemaView, items, ChangeOrderSchemaItemCommand<SchemaType>.ChangeOrderType.Front, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "На задний план"
//		/// </summary>
//		/// <param name="items">спиосок ссылок на элементы для установки свойств</param>
//		public void RunSendToBackSchemaItem(List<VFrame30Ext.SchemaItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemaItemCommand<SchemaType>(EditSchemaView, items, ChangeOrderSchemaItemCommand<SchemaType>.ChangeOrderType.Back, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "На уровень вверх"
//		/// </summary>
//		/// <param name="items">спиосок ссылок на элементы для установки свойств</param>
//		public void RunLevelUpSchemaItem(List<VFrame30Ext.SchemaItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemaItemCommand<SchemaType>(EditSchemaView, items, ChangeOrderSchemaItemCommand<SchemaType>.ChangeOrderType.Up, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// Команда "На уровень вниз"
//		/// </summary>
//		/// <param name="items">спиосок ссылок на элементы для установки свойств</param>
//		public void RunLevelDownSchemaItem(List<VFrame30Ext.SchemaItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemaItemCommand<SchemaType>(EditSchemaView, items, ChangeOrderSchemaItemCommand<SchemaType>.ChangeOrderType.Down, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

	signals:
		void stateChanged(bool canUndo, bool canRedo);
		void modifiedChanged(bool modified);
		void propertiesChanged();

		// Data
		//
	private:
		static const int MaxCommandCount = 2048;

		EditSchemaView* m_schemaView;
		QScrollBar* m_hScrollBar;
		QScrollBar* m_vScrollBar;

		std::vector<std::shared_ptr<EditCommand>> m_commands;
		int m_current;

		bool m_readOnly;
		bool m_modified;
	};


	//
	//
	// EditCommand
	//
	//
	class EditCommand
	{
	private:
		EditCommand();		// deleted;
	public:
		EditCommand(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	public:
		void execute(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);
		void unExecute(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemaView* schemaView) = 0;
		virtual void unExecuteCommand(EditSchemaView* schemaView) = 0;

		void saveViewPos(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);
		void restoreViewPos(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

		// Data
		//
	protected:
		std::shared_ptr<VFrame30::SchemaLayer> m_activeLayer;		// Active Layer on operation start

		double m_zoom;

		QScrollBar m_hScrollBar;
		QScrollBar m_vScrollBar;
	};
}


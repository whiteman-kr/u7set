#pragma once

#include "../VFrame30/SchemeItem.h"

class EditSchemeView;

namespace VFrame30
{
	class SchemeLayer;
	class Scheme;
}

namespace EditEngine
{
	class EditCommand;

	//
	//
	// EditEngine - ����� �������������� ���������� ���� ������ ��������� SchemeView (Undo )
	//
	//
	class EditEngine : public QObject
	{
		Q_OBJECT

	private:
		EditEngine();		// deleted;

	public:
		EditEngine(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar, QObject* parent);
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
		void runAddItem(std::list<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemeLayer> layer);
		void runAddItem(std::vector<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemeLayer> layer);
		void runAddItem(std::shared_ptr<VFrame30::SchemeItem> item, std::shared_ptr<VFrame30::SchemeLayer> layer);

		void runDeleteItem(const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items, std::shared_ptr<VFrame30::SchemeLayer> layer);
		void runDeleteItem(std::shared_ptr<VFrame30::SchemeItem> item, std::shared_ptr<VFrame30::SchemeLayer> layer);

		void runSetPoints(const std::vector<std::vector<VFrame30::SchemePoint>>& points, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items);
		void runSetPoints(const std::vector<VFrame30::SchemePoint>& points, const std::shared_ptr<VFrame30::SchemeItem>& item);

		void runMoveItem(double xdiff, double ydiff, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items, bool snapToGrid);
		void runMoveItem(double xdiff, double ydiff, const std::shared_ptr<VFrame30::SchemeItem>& item, bool snapToGrid);

		void runSetProperty(const QString& propertyName, QVariant value, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items);
		void runSetProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::SchemeItem>& item);

		void runSetSchemeProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::Scheme>& scheme);

//		/// <summary>
//		/// ������� "�������� ������"
//		/// </summary>
//		/// <param name="width">������ ������� �� ����������� (����� ���� ������ 0)</param>
//		/// <param name="height">������ ������� �� ��������� (����� ���� ������ 0)</param>
//		/// <param name="items">������ �������� ��� ���������� �������</param>
//		public void RunResizeCommand(double width, double height, List<VFrame30Ext.SchemeItem> items)
//		{
//			AddCommand(new ResizeSchemeItemCommand<SchemeType>(EditSchemeView, width, height, items, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�������� ������"
//		/// </summary>
//		/// <param name="size">������ �������� �������� (����� ���� ������ 0)</param>
//		/// <param name="items">������ �������� ��� ���������� �������</param>
//		public void RunResizeCommand(List<Tuple<double, double>> size, List<VFrame30Ext.SchemeItem> items)
//		{
//			AddCommand(new ResizeSchemeItemCommand<SchemeType>(EditSchemeView, size, items, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�������� ��������"
//		/// </summary>
//		/// <param name="item">scheme item</param>
//		/// <param name="oldItem">old value</param>
//		/// <param name="newItem">new value</param>
//		public void RunSetSchemeItemProperties(VFrame30Ext.SchemeItem item, VFrame30Ext.StreamedData oldItem, VFrame30Ext.StreamedData newItem)
//		{
//			List<VFrame30Ext.SchemeItem> items = new List<VFrame30Ext.SchemeItem>(1);
//			items.Add(item);

//			List<VFrame30Ext.StreamedData> oldSd = new List<VFrame30Ext.StreamedData>(1);
//			oldSd.Add(oldItem);

//			List<VFrame30Ext.StreamedData> newSd = new List<VFrame30Ext.StreamedData>(1);
//			newSd.Add(newItem);

//			AddCommand(new SetSchemeItemPropertiesCommand<SchemeType>(EditSchemeView, items, oldSd, newSd, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�������� ��������"
//		/// </summary>
//		/// <param name="items">������� ������ �� �������� ��� ��������� �������</param>
//		/// <param name="oldItems">������ ������ �������� ���������</param>
//		/// <param name="newItems">������ ����� �������� ���������</param>
//		public void RunSetSchemeItemProperties(List<VFrame30Ext.SchemeItem> items, List<VFrame30Ext.StreamedData> oldItems, List<VFrame30Ext.StreamedData> newItems)
//		{
//			AddCommand(new SetSchemeItemPropertiesCommand<SchemeType>(EditSchemeView, items, oldItems, newItems, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�� �������� ����"
//		/// </summary>
//		/// <param name="items">������� ������ �� �������� ��� ��������� �������</param>
//		public void RunBringToFrontSchemeItem(List<VFrame30Ext.SchemeItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemeItemCommand<SchemeType>(EditSchemeView, items, ChangeOrderSchemeItemCommand<SchemeType>.ChangeOrderType.Front, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�� ������ ����"
//		/// </summary>
//		/// <param name="items">������� ������ �� �������� ��� ��������� �������</param>
//		public void RunSendToBackSchemeItem(List<VFrame30Ext.SchemeItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemeItemCommand<SchemeType>(EditSchemeView, items, ChangeOrderSchemeItemCommand<SchemeType>.ChangeOrderType.Back, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�� ������� �����"
//		/// </summary>
//		/// <param name="items">������� ������ �� �������� ��� ��������� �������</param>
//		public void RunLevelUpSchemeItem(List<VFrame30Ext.SchemeItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemeItemCommand<SchemeType>(EditSchemeView, items, ChangeOrderSchemeItemCommand<SchemeType>.ChangeOrderType.Up, layerGuid, hScrollBar, vScrollBar), true);
//			return;
//		}

//		/// <summary>
//		/// ������� "�� ������� ����"
//		/// </summary>
//		/// <param name="items">������� ������ �� �������� ��� ��������� �������</param>
//		public void RunLevelDownSchemeItem(List<VFrame30Ext.SchemeItem> items, Guid layerGuid)
//		{
//			AddCommand(new ChangeOrderSchemeItemCommand<SchemeType>(EditSchemeView, items, ChangeOrderSchemeItemCommand<SchemeType>.ChangeOrderType.Down, layerGuid, hScrollBar, vScrollBar), true);
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

		EditSchemeView* m_schemeView;
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
		EditCommand(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	public:
		void execute(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);
		void unExecute(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) = 0;
		virtual void unExecuteCommand(EditSchemeView* schemeView) = 0;

		void saveViewPos(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);
		void restoreViewPos(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

		// Data
		//
	protected:
		std::shared_ptr<VFrame30::SchemeLayer> m_activeLayer;		// Active Layer on operation start

		double m_zoom;

		QScrollBar m_hScrollBar;
		QScrollBar m_vScrollBar;
	};
}


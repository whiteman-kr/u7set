#pragma once

#include "SignalListEditor.h"

namespace ExtWidgets
{
	class PropertyEditor;
}

namespace AppSignalLists
{
	class DialogSignalListEditor : public QDialog
	{
		Q_OBJECT
	public:
		static void showDialog(AppSignalListSet& appSignalListSet,
							   ISignalManager& signalManager,
							   ITuningSignalManager* tuningSignalManager,
							   QWidget* parent);
		static DialogSignalListEditor* instance();

	private:
		DialogSignalListEditor(AppSignalListSet& appSignalListSet,
							   ISignalManager& signalManager,
							   ITuningSignalManager* tuningSignalManager,
							   QWidget* parent);
		~DialogSignalListEditor();

	public:
		void setFilter(QString filter);

	signals:
		void editingFinished();

	private slots:
		void onMaskReturn();
		void onMaskApply();

		void onSortIndicatorChanged(int column, Qt::SortOrder order);

		void onItemSelectionChanged();
		void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);
		void onSignalsChanged();

		void onAdd();
		void onRemove();

		void onCopy();
		void onPaste();

		void onCopyShortcut();
		void onPasteShortcut();
		void onRemoveShortcut();

		void onCustomContextMenuRequested(const QPoint& pos);

	private:
		bool addList(std::shared_ptr<AppSignalLists::AppSignalList> list);
		bool pasteList(std::shared_ptr<AppSignalLists::AppSignalList> list);

		void fillAppSignalLists();
		void setPropertyEditorObjects();
		bool continueWithDuplicateIds();
		void updateTreeItemText(QTreeWidgetItem* item);
		void updateListEditorEnableState();

		void saveChanges();

	protected:
		virtual void closeEvent(QCloseEvent* e);
		virtual void accept();
		virtual void reject();

	private:
		QLineEdit* m_mask = nullptr;
		QPushButton* m_maskApply = nullptr;

		QPushButton* m_btnAdd = nullptr;
		QPushButton* m_btnRemove = nullptr;
		
		QPushButton* m_btnOk = nullptr;
		QPushButton* m_btnCancel = nullptr;

		QTreeWidget* m_listsTree = nullptr;

		ExtWidgets::PropertyEditor* m_listPropertyEditor = nullptr;
		AppSignalLists::AppSignalListWidget* m_signalListWidget = nullptr;

		QSplitter* m_splitter = nullptr;

		QCompleter* m_completer = nullptr;

		QStringList m_masks;

		AppSignalListSet& m_appLists;	// Original lists
		AppSignalListSet m_editLists;	// Edited lists

		ISignalManager& m_signalManager;

		QMenu* m_popupMenu = nullptr;
		QAction* m_addAction = nullptr;
		QAction* m_removeAction = nullptr;
		QAction* m_copyAction = nullptr;
		QAction* m_pasteAction = nullptr;

		bool m_modified = false;

		inline static DialogSignalListEditor* s_instance = nullptr;
	};

} // namespace AppSignalLists

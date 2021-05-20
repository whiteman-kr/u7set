#ifndef PROPERTYTABLE_H
#define PROPERTYTABLE_H

#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include "../CommonLib/PropertyObject.h"
#include "../lib/PropertyEditor.h"

namespace ExtWidgets
{
	class DialogReplace : public QDialog
	{
		Q_OBJECT

	public:
		DialogReplace(const QString& what, const QString& to, bool caseSensitive, QWidget* parent);

		const QString what() const;
		const QString to() const;
		bool caseSensitive() const;

	protected:
		virtual void accept() override;

	private:

		QLineEdit* m_editWhat = nullptr;
		QLineEdit* m_editTo = nullptr;

		QCheckBox* m_checkCase = nullptr;

		QString m_what;
		QString m_to;
		bool m_caseSensitive = false;

	};

	typedef QMap<QString, std::pair<std::shared_ptr<PropertyObject>, QVariant>> ModifiedObjectsData;

	struct PropertyTableObject
	{
		std::shared_ptr<PropertyObject> propertyObject;

		int rowCount = 1;

		std::vector<std::shared_ptr<Property>> properties;
	};

	class PropertyTable;
	class PropertyTableModel;
	class PropertyTableProxyModel;

	class PropertyTableItemDelegate : public QItemDelegate
	{
		Q_OBJECT
	public:
		explicit PropertyTableItemDelegate(PropertyTable* propertyTable, PropertyTableProxyModel* proxyModel);

		virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
		virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		void setInitText(const QString& text);

	private slots:
		void onValueChanged(QVariant value);

	signals:
		void valueChanged(QVariant value);

	private:
		PropertyTable* m_propertyTable = nullptr;
		PropertyTableProxyModel* m_proxyModel = nullptr;

		mutable PropertyEditCellWidget *m_cellEditor = nullptr;

		mutable QString m_initText;
	};

	class PropertyTableProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT

	public:
		PropertyTableProxyModel(QObject *parent = 0);

		std::shared_ptr<PropertyObject> propertyObjectByIndex(const QModelIndex& mi) const;
		std::shared_ptr<Property> propertyByIndex(const QModelIndex& mi, int* propertyRow) const;

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
	};

	class PropertyTableModel : public QAbstractTableModel
	{
	public:
		PropertyTableModel(PropertyTable* propertyTable);
		~PropertyTableModel();

		void clear();
		void setTableObjects(std::vector<PropertyTableObject>& tableObjects, bool showCategory);

		std::shared_ptr<PropertyObject> propertyObjectByIndex(const QModelIndex& mi) const;
		std::shared_ptr<Property> propertyByIndex(const QModelIndex& mi, int* propertyRow) const;

		void recalculateRowCount(std::shared_ptr<PropertyObject> object);

		bool hasMultiRows() const;

	private:
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

		Qt::ItemFlags flags(const QModelIndex &index) const override;

	private:
		PropertyTable* m_propertyTable = nullptr;

		std::vector<PropertyTableObject> m_tableObjects;
		QStringList m_propertyNames;

	};

	class PropertyTableView : public QTableView
	{
		Q_OBJECT

	public:
		void closeCurrentEditorIfOpen();

	protected:
		virtual void mousePressEvent(QMouseEvent *event) override;
		virtual void keyPressEvent(QKeyEvent *event) override;

	signals:
		void mousePressed();
		void editKeyPressed();
		void symbolKeyPressed(QString key);
		void spaceKeyPressed();
	};

	class PropertyTable : public QWidget, public PropertyEditorBase
	{
		Q_OBJECT
	public:
		explicit PropertyTable(QWidget *parent = nullptr);

		void clear();

		const QList<std::shared_ptr<PropertyObject>>& objects() const;
		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		QString propertyFilter() const;
		void setPropertyFilter(const QString& propertyFilter);

		// Settings to store
		//
		bool expandValuesToAllRows() const;
		void setExpandValuesToAllRows(bool value);

		const QMap<QString, int>& getColumnsWidth();
		void setColumnsWidth(const QMap<QString, int>& columnsWidth);

		bool groupByCategory() const;
		void setGroupByCategory(bool value);

	protected:
		virtual void valueChanged(const ModifiedObjectsData& modifiedObjectsData);
		virtual void hideEvent(QHideEvent* event) override;

	protected slots:
		void updatePropertiesList();
		void updatePropertiesValues();

	private slots:
		void onCellDoubleClicked(const QModelIndex &index);
		void onCellEditKeyPressed();
		void onCellClicked();
		void onCellSymbolKeyPressed(QString key);
		void onCellToggleKeyPressed();
		void onShowErrorMessage (QString message);
		void onPropertyFilterChanged();
		void onTableContextMenuRequested(const QPoint &pos);
		void onGroupByCategoryToggled(bool value);

		// Context menu slots

		void onInsertStringBefore();
		void onInsertStringAfter();
		void onRemoveString();
		void onUniqueRowValuesChanged();

		void onReplace();

	public slots:
		void onValueChanged(QVariant value);
		void onCellEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

	signals:
		void showErrorMessage(QString message);
		void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		void fillProperties();

		int getSelectionType();	// returns -1 if no type is selected or they are different
		bool isSelectionReadOnly();

		void startEditing();
		void toggleSelected();

		void addString(bool after);
		void removeString();

		void saveColumnsWidth();
		void restoreColumnsWidth();

	private:

		PropertyTableView* m_tableView = nullptr;

		QLineEdit* m_editPropertyFilter = nullptr;

		QPushButton* m_buttonGroupByCategory = nullptr;

		PropertyTableProxyModel m_proxyModel;

		PropertyTableModel m_tableModel;

		QMap<QString, int> m_columnsWidth;

		PropertyTableItemDelegate* m_itemDelegate = nullptr;

		QList<std::shared_ptr<PropertyObject>> m_objects;

		QStringList m_propertyFilters;

		bool m_expandValuesToAllRows = true;

	};

}

#endif // PROPERTYTABLE_H

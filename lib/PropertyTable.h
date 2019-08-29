#ifndef PROPERTYTABLE_H
#define PROPERTYTABLE_H

#include <QAbstractItemModel>
#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"

namespace ExtWidgets
{

	struct PropertyTableObject
	{
		std::shared_ptr<PropertyObject> propertyObject;

		int rowCount = 1;

		std::vector<std::shared_ptr<Property>> properties;
	};

	class PropertyTable;
	class PropertyTableModel;

	class PropertyTableItemDelegate : public QItemDelegate
	{
		Q_OBJECT
	public:
		explicit PropertyTableItemDelegate(PropertyTable* propertyTable, PropertyTableModel* model);

		virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;

		virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

		virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	private slots:
		void onValueChanged(QVariant value);

	signals:
		void valueChanged(QVariant value);

	private:
		PropertyTable* m_propertyTable = nullptr;
		PropertyTableModel* m_model = nullptr;

		mutable PropertyEditCellWidget *m_cellEditor = nullptr;
	};

	class PropertyTableModel : public QAbstractTableModel
	{
	public:
		PropertyTableModel(PropertyTable* propertyTable);
		~PropertyTableModel();

		void clear();
		void setTableObjects(std::vector<PropertyTableObject>& tableObjects);

		std::shared_ptr<PropertyObject> propertyObjectByIndex(const QModelIndex& mi) const;

		std::shared_ptr<Property> propertyByIndex(const QModelIndex& mi, int* propertyRow) const;

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
	public:
		void closeCurrentEditorIfOpen();
	};

	class PropertyTable : public QWidget, public PropertyEditorBase
	{
		Q_OBJECT
	public:
		explicit PropertyTable(QWidget *parent = nullptr);

		void clear();

		void closeCurrentEditor();

		const QList<std::shared_ptr<PropertyObject>>& objects() const;
		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		QString propertyMask() const;
		void setPropertyMask(const QString& propertyMask);

	protected:
		virtual void valueChanged(QMap<QString, std::pair<std::shared_ptr<PropertyObject>, QVariant>> modifiedObjectsData);

	protected slots:
		void updatePropertiesList();
		void updatePropertiesValues();

	private slots:
		void onCellDoubleClicked(const QModelIndex &index);
		void onShowErrorMessage (QString message);
		void onPropertyMaskChanged();

	public slots:
		void onValueChanged(QVariant value);

	signals:
		void showErrorMessage(QString message);
		void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		void fillProperties();

	private:

		PropertyTableView* m_tableView = nullptr;

		QLineEdit* m_editPropertyMask = nullptr;

		PropertyTableModel m_tableModel;

		PropertyTableItemDelegate* m_itemDelegate = nullptr;

		QList<std::shared_ptr<PropertyObject>> m_objects;

		QStringList m_propertyMasks;

	};

}

#endif // PROPERTYTABLE_H

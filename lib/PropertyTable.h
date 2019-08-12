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

	class PropertyTableItemDelegate : public QItemDelegate
	{
		Q_OBJECT
	public:
		explicit PropertyTableItemDelegate(QObject *parent = 0);

		// Create Editor when we construct MyDelegate
		QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

		// Then, we set the Editor
		void setEditorData(QWidget *editor, const QModelIndex &index) const;

		// When we modify data, this model reflect the change
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

		// Give the SpinBox the info on size and location
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	signals:

	public slots:

	};

	class PropertyTableModel : public QAbstractTableModel
	{
	public:
		PropertyTableModel(PropertyTable* propertyTable);
		~PropertyTableModel();

		void clear();
		void setTableObjects(std::vector<PropertyTableObject>& tableObjects);

		Property* propertyByIndex(const QModelIndex& mi) const;

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

	class PropertyTable : public QWidget, public PropertyEditorBase
	{
		Q_OBJECT
	public:
		explicit PropertyTable(QWidget *parent = nullptr);

		void setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects);
		void setObjects(const QList<std::shared_ptr<PropertyObject>>& objects);

		const QList<std::shared_ptr<PropertyObject>>& objects() const;

		void clear();

		void updatePropertyValues(const QString& propertyName);

		bool expertMode() const;
		void setExpertMode(bool expertMode);

		bool readOnly() const;
		void setReadOnly(bool readOnly);

	protected:
//		virtual void valueChanged(QtProperty* property, QVariant value);

	protected slots:
		void updatePropertiesList();
		void updatePropertiesValues();

	private slots:
		void onCellDoubleClicked(const QModelIndex &index);

//		void onValueChanged(QtProperty* property, QVariant value);
//		void onShowErrorMessage (QString message);
//		void onCurrentItemChanged(QtBrowserItem* current);

	signals:
//		void showErrorMessage(QString message);
		void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	private:
		void fillProperties();

	private:

		QTableView* m_tableView = nullptr;
		PropertyTableModel* m_tableModel = nullptr;

		PropertyTableItemDelegate* m_itemDelegate = nullptr;

		QList<std::shared_ptr<PropertyObject>> m_objects;

		bool m_expertMode = false;
		bool m_readOnly = false;

	};

}

#endif // PROPERTYTABLE_H

#ifndef SPECIFICPROPERTIESEDITOR_H
#define SPECIFICPROPERTIESEDITOR_H

#include "../lib/PropertyEditor.h"
#include <QSortFilterProxyModel>


enum SpecificPropertyEditorColumns
{
	Category = 0,
	Caption,
	ViewOrder,
	Type,
	Count

};

class SpecificPropertyDescription : public PropertyObject
{
	Q_OBJECT

public:
	SpecificPropertyDescription();
	explicit SpecificPropertyDescription(const SpecificPropertyDescription& source);

	QString caption() const;
	void setCaption(const QString& value);

	QString category() const;
	void setCategory(const QString& value);

	QString description() const;
	void setDescription(const QString& value);

	E::SpecificPropertyType type() const;
	void setType(E::SpecificPropertyType value);

	QString typeDynamicEnum() const;
	void setTypeDynamicEnum(const QString& value);

	QString lowLimit() const;
	void setLowLimit(const QString& value);

	QString highLimit() const;
	void setHighLimit(const QString& value);

	QString defaultValue() const;
	void setDefaultValue(const QString& value);

	int precision() const;
	void setPrecision(int value);

	bool updateFromPreset() const;
	void setUpdateFromPreset(bool value);

	bool expert() const;
	void setExpert(bool value);

	bool visible() const;
	void setVisible(bool value);

	E::PropertySpecificEditor specificEditor() const;
	void setSpecificEditor(E::PropertySpecificEditor value);

	quint16 viewOrder() const;
	void setViewOrder(quint16 value);

	void validateDynamicEnumType(QWidget* parent);

	std::tuple<quint16, QString, int, QString> tuple_order() const;
	std::tuple<QString, quint16, int, QString> tuple_caption() const;
	std::tuple<int, quint16, QString, QString> tuple_type() const;
	std::tuple<QString, quint16, QString, int> tuple_category() const;

private:
	// Warning! Be sure to add new fields to the copy constructor!
	//
	QString m_caption;
	QString m_category;
	QString m_description;
	E::SpecificPropertyType m_type = E::SpecificPropertyType::pt_int32;
	QString m_typeDynamicEnum;
	QString m_lowLimit;
	QString m_highLimit;
	QString m_defaultValue;
	int m_precision = 0;
	bool m_updateFromPreset = false;
	bool m_expert = false;
	bool m_visible = true;
	E::PropertySpecificEditor m_specificEditor = E::PropertySpecificEditor::None;
	quint16 m_viewOrder = 65535;
};

class SpecificPropertyModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SpecificPropertyModel(QObject *parent);

	void clear();

	void add(std::shared_ptr<SpecificPropertyDescription> spd);

	void remove(QModelIndexList indexes);

	std::shared_ptr<SpecificPropertyDescription> get(int index) const;

	int count() const;

	void update();

	void sort(int column, Qt::SortOrder order) override;

	QString toText() const;


protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:

	std::vector<std::shared_ptr<SpecificPropertyDescription>> m_propertyDescriptions;
	std::vector<int> m_sortedIndexes;

	int m_sortColumn = SpecificPropertyEditorColumns::ViewOrder;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

class SpecificPropertyModelSorter
{
public:
	SpecificPropertyModelSorter(int column, Qt::SortOrder order, std::vector<std::shared_ptr<SpecificPropertyDescription>>* propertyDescriptions);

	bool operator()(int index1, int index2) const
	{
		return sortFunction(index1, index2, m_column, m_order);
	}

	bool sortFunction(int index1, int index2, int column, Qt::SortOrder order) const;

private:
	int m_column = -1;

	Qt::SortOrder m_order = Qt::AscendingOrder;

	std::vector<std::shared_ptr<SpecificPropertyDescription>>* m_propertyDescriptions = nullptr;
};

class SpecificPropertiesEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT
public:
	explicit SpecificPropertiesEditor(QWidget* parent);
	virtual ~SpecificPropertiesEditor();

	void setText(const QString& text) override;
	QString text() override;

	void setReadOnly(bool value) override;

private slots:
	void tableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);
	void sortIndicatorChanged(int column, Qt::SortOrder order);



	void onAddProperty();
	void onCloneProperty();
	void onRemoveProperties();

	void onOkClicked();
	void onCancelClicked();


private:
	SpecificPropertyModel m_propertiesModel;

	QTableView* m_propertiesTable = nullptr;
	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;


	QPushButton* m_addButton = nullptr;
	QPushButton* m_cloneButton = nullptr;
	QPushButton* m_removeButton = nullptr;

};

#endif // SPECIFICPROPERTIESEDITOR_H

#ifndef SPECIFICPROPERTIESEDITOR_H
#define SPECIFICPROPERTIESEDITOR_H

#include "../lib/PropertyEditor.h"


class SpecificPropertyDescription : public PropertyObject
{
	Q_OBJECT
public:
	SpecificPropertyDescription();

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

	void updateDynamicEnumType();

private:
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
	void onTreeSelectionChanged();
	void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);


	void onAddProperty();
	void onRemoveProperties();

	void onOkClicked();
	void onCancelClicked();

private:
	void updatePropetyListItem(QTreeWidgetItem* item, SpecificPropertyDescription* spd);

private:
	QTreeWidget* m_propertiesList = nullptr;
	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;

	QPushButton* m_addButton = nullptr;
	QPushButton* m_removeButton = nullptr;

	std::map<QTreeWidgetItem*, std::shared_ptr<SpecificPropertyDescription>> m_propertyDescriptionsMap;

	const int m_columnCaption = 0;
	const int m_columnType = 1;
	const int m_columnCategory = 2;

};

#endif // SPECIFICPROPERTIESEDITOR_H

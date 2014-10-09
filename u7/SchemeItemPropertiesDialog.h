#ifndef SCHEMEITEMPROPERTIESDIALOG_H
#define SCHEMEITEMPROPERTIESDIALOG_H

#include <QDialog>
#include <memory>
#include "../VFrame30/VideoItem.h"
#include "../include/PropertyEditor.h"


namespace Ui {
	class SchemeItemPropertiesDialog;
}

class SchemeItemPropertyEditor;


class SchemeItemPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemeItemPropertiesDialog(QWidget *parent = 0);
	virtual ~SchemeItemPropertiesDialog();

public:
	void setObjects(const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items);

private:
	Ui::SchemeItemPropertiesDialog *ui;

	SchemeItemPropertyEditor* m_propertyEditor = nullptr;

	std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_items;
	std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_itemsLocalCopy;
};

//
//
//	SchemeItemPropertyBrowser
//
//

class SchemeItemPropertyEditor : public PropertyEditor
{
public:
	explicit SchemeItemPropertyEditor(QWidget* parent);
	virtual ~SchemeItemPropertyEditor();

};

#endif // SCHEMEITEMPROPERTIESDIALOG_H

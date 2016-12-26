#ifndef COMPAREPROPERTYOBJECTDIALOG_H
#define COMPAREPROPERTYOBJECTDIALOG_H

#include <QDialog>
#include "GlobalMessanger.h"
#include "../../lib/DbStruct.h"
#include "../../lib/PropertyObject.h"

namespace Ui {
	class ComparePropertyObjectDialog;
}

class ComparePropertyObjectDialog : public QDialog
{
	Q_OBJECT

private:
	ComparePropertyObjectDialog(QString source, QString target, QWidget* parent);

public:
	~ComparePropertyObjectDialog();

	static QString objedctToCompareString(PropertyObject* object);

	static void showDialog(
			DbChangesetObject object,
			CompareData compareData,
			std::shared_ptr<PropertyObject> source,
			std::shared_ptr<PropertyObject> target,
			QWidget* parent);

	static void showDialog(
			DbChangesetObject object,
			CompareData compareData,
			QString source,
			QString target,
			QWidget* parent);

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	Ui::ComparePropertyObjectDialog* ui;
};

#endif // COMPAREPROPERTYOBJECTDIALOG_H

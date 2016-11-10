#ifndef CHANGESETDETAILSDIALOG_H
#define CHANGESETDETAILSDIALOG_H

#include <QDialog>
#include "../lib/DbController.h"

namespace Ui {
	class ChangesetDetailsDialog;
}

class ChangesetDetailsDialog : public QDialog
{
	Q_OBJECT

private:
	explicit ChangesetDetailsDialog(const DbChangesetDetails& changesetDetails, QWidget* parent);
public:
	~ChangesetDetailsDialog();

	static void showChangesetDetails(DbController* db, int changeset, QWidget* parent);

private slots:
	void on_objects_customContextMenuRequested(const QPoint &pos);

	void compare(DbChangesetObject object, int changeset);

private:
	enum class ObjectsColumns
	{
		Type,
		Name,
		Caption,
		Action,
		Parent,

		ColumnCount
	};

	Ui::ChangesetDetailsDialog *ui;
	DbChangesetDetails m_changesetDetails;

	static QByteArray m_splitterState;				// for local use, if want to store it in setting, use theSettings;
};

#endif // CHANGESETDETAILSDIALOG_H
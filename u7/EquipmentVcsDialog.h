#ifndef EQUIPMENTVCSDIALOG_H
#define EQUIPMENTVCSDIALOG_H

#include <QDialog>

namespace Ui {
	class EquipmentVcsDialog;
}

class DbController;

class EquipmentVcsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EquipmentVcsDialog(DbController* db, QWidget* parent);
	~EquipmentVcsDialog();

private slots:
	void on_m_checkIn_clicked();
	void on_m_undo_clicked();
	void on_m_refresh_clicked();
	void on_m_ok_clicked();

private:
	Ui::EquipmentVcsDialog* ui;
	DbController* m_db;

	static const int ColumnStrID = 0;
	static const int ColumnCaption = 1;
	static const int ColumnPlace = 2;
	static const int ColumnType = 3;
	static const int ColumnUser = 4;
	static const int ColumnAction = 5;

	static const wchar_t* ColumnName[];
};

#endif // EQUIPMENTVCSDIALOG_H

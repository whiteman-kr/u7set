#pragma once

#include <QDialog>

class SignalsModel;
class TableDataVisibilityController;
class CheckedoutSignalsModel;

class CheckinSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	CheckinSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QModelIndexList selection, QWidget *parent = nullptr);

public slots:
	void checkinSelected();
	void cancel();

protected:
	void closeEvent(QCloseEvent* event);

private:
	void saveDialogGeometry();

private:
	SignalsModel* m_sourceModel = nullptr;
	CheckedoutSignalsModel* m_proxyModel = nullptr;
	QTableView* m_signalsView = nullptr;
	QPlainTextEdit* m_commentEdit = nullptr;
	QSplitter* m_splitter = nullptr;
};

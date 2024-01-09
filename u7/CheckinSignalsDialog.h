#pragma once

#include <QDialog>

class SignalsModel;
class SignalsProxyModel;
class TableDataVisibilityController;
class CheckedoutSignalsModel;

class CheckinSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	CheckinSignalsDialog(const std::vector<int>& selSignalsIndexes,
						 SignalsModel& sourceModel,
						 const TableDataVisibilityController& columnManager,
						 QWidget* parent = nullptr);

public slots:
	void checkinSelected();
	void cancel();

protected:
	void closeEvent(QCloseEvent* event);

private:
	void saveDialogGeometry();

private:
	SignalsModel& m_sourceModel;
	CheckedoutSignalsModel* m_checkedOutModel = nullptr;		// Checked Out Signals Model

	QTableView* m_signalsView = nullptr;
	QPlainTextEdit* m_commentEdit = nullptr;
	QSplitter* m_splitter = nullptr;
};

#pragma once

class SignalsModel;
class SignalsProxyModel;
class TableDataVisibilityController;
class CheckedOutSignalsModel;

class CheckinSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	CheckinSignalsDialog(const QModelIndexList& selectionSrcIndexes,
						 SignalsModel* signalsModel,
						 const TableDataVisibilityController& columnManager,
						 QWidget* parent = nullptr);

public slots:
	void checkinSelected();
	void cancel();

protected:
	void closeEvent(QCloseEvent* event);

private:
	void checkSelection(const QItemSelection);
	void saveDialogGeometry();

private:
	SignalsModel* m_signalsModel = nullptr;
	CheckedOutSignalsModel* m_checkedOutModel = nullptr;		// Checked Out Signals Model

	QTableView* m_signalsView = nullptr;
	QPlainTextEdit* m_commentEdit = nullptr;
	QSplitter* m_splitter = nullptr;
};

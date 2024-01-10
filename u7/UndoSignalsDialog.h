#pragma once

#include <QDialog>

class SignalsModel;
class TableDataVisibilityController;
class CheckedOutSignalsModel;

class UndoSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	UndoSignalsDialog(const QModelIndexList& selectionSrcIndexes,
					  SignalsModel* signalsModel,
					  const TableDataVisibilityController& columnManager,
					  QWidget* parent = nullptr);

	void saveDialogGeometry();

	const std::vector<int>& undoedSignalsIDs() const { return m_undoedSignalsIDs; }

public slots:
	void undoSelected();

protected:
	void closeEvent(QCloseEvent* event);

private:
	SignalsModel* m_signalsModel = nullptr;
	CheckedOutSignalsModel* m_checkedOutModel = nullptr;

	std::vector<int> m_undoedSignalsIDs;
};

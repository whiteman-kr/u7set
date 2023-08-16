#pragma once

#include <QDialog>

class SignalsModel;
class TableDataVisibilityController;
class CheckedoutSignalsModel;

class UndoSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	UndoSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QWidget *parent = nullptr);

	void setCheckStates(QModelIndexList selection, bool fromSourceModel);
	void saveDialogGeometry();

	const std::vector<int>& undoedSignalsIDs() const { return m_undoedSignalsIDs; }

public slots:
	void undoSelected();

protected:
	void closeEvent(QCloseEvent* event);

private:
	SignalsModel* m_sourceModel = nullptr;
	CheckedoutSignalsModel* m_proxyModel = nullptr;

	std::vector<int> m_undoedSignalsIDs;
};

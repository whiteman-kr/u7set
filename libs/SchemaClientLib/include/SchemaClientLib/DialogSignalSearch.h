#pragma once

#include <QDialog>

#include "../AppSignalLib/IAppSignalManager.h"
#include <SchemaClientLib/DragDropHelper.h>

class QLineEdit;
class QTableView;
class QLabel;
class QMenu;
class SignalSearchItemModel;
class SignalSearchTableView;

namespace SchemaClientLib
{
	class DialogSignalSearch : public QDialog
	{
		Q_OBJECT

	public:
		explicit DialogSignalSearch(QWidget* parent, IAppSignalManager* appSignalManager);
		virtual ~DialogSignalSearch();

	public slots:
		void signalsUpdated(); // Should be called when new signals arrived from AppDataService

	signals:
		void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
		void signalInfo(QString appSignalId);

	private slots:
		void textEdited(const QString& arg1);
		void openClicked();
		void tableDoubleClicked(const QModelIndex& index);
		void prepareContextMenu(const QPoint& pos);

	private:
		void search();

	private:
		static QString m_signalId;

		QLineEdit* m_editSignalID = nullptr;
		SignalSearchTableView* m_tableView = nullptr;
		QLabel* m_labelFound = nullptr;

		IAppSignalManager* m_appSignalManager = nullptr;
		SignalSearchItemModel* m_model = nullptr;

		std::vector<AppSignalParam> m_signals;
	};
} // namespace SchemaClientLib
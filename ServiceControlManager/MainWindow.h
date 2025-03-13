#pragma once

#include <QMainWindow>
#include "ServiceTableModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
    ~MainWindow();

private:
	QVector<QWidget*> m_widgets;
    ServiceTableModel* m_serviceModel;
    QTableView* m_serviceTable;

//    void openConnectionInfo(QString text);

protected:
	void closeEvent(QCloseEvent *event);

public slots:
    void openEditor();
    void switchLanguage(QAction* selectedAction);
//    void connectionClicked(QAction* selectedAction);
    void scanNetwork();
    void removeHost();
	void aboutScm();
};

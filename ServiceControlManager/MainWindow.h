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
    ServiceTableModel* m_serviceModel;
    QTableView* m_serviceTable;

protected:
	void closeEvent(QCloseEvent *event);

public slots:
    void openEditor();
    void switchLanguage(QAction* selectedAction);
    void scanNetwork();
    void removeHost();
	void aboutScm();
};

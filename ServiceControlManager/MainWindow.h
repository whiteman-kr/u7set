#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "../lib/Tcp.h"

class ServiceTableModel;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(const Tcp::SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
    ~MainWindow();

private:
    QSystemTrayIcon *m_trayIcon;
    QVector<QWidget*> m_widgets;
    ServiceTableModel* m_serviceModel;
    QTableView* m_serviceTable;

    void openConnectionInfo(QString text);

protected:
	void closeEvent(QCloseEvent *event);

public slots:
    void openEditor();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void switchLanguage(QAction* selectedAction);
    void connectionClicked(QAction* selectedAction);
    void scanNetwork();
    void removeHost();
};

#endif // MAINWINDOW_H

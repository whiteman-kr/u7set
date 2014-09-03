#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class ServiceTableModel;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QSystemTrayIcon *trayIcon;
    QVector<QWidget*> widgets;
    ServiceTableModel* serviceModel;
    QTableView* serviceTable;

    void openConnectionInfo(QString text);
    void setServicesForCommand(int command);

public slots:
    void openEditor();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void switchLanguage(QAction* selectedAction);
    void connectionClicked(QAction* selectedAction);
    void scanNetwork();
    void startService();
    void stopService();
    void restartService();
    void removeHost();

signals:
    void commandPushed(int row, int col, int command);
};

#endif // MAINWINDOW_H

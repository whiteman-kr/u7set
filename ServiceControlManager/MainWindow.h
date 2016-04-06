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
    QSystemTrayIcon *m_trayIcon;
    QVector<QWidget*> m_widgets;
    ServiceTableModel* m_serviceModel;
    QTableView* m_serviceTable;

    void openConnectionInfo(QString text);

public slots:
    void openEditor();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void switchLanguage(QAction* selectedAction);
    void connectionClicked(QAction* selectedAction);
    void scanNetwork();
    void removeHost();
};

#endif // MAINWINDOW_H
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class ServiceTableModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    void openConnectionInfo(QString text);
    void checkAddress(QString connectionAddress);

    QVector<QWidget*> widgets;
    ServiceTableModel* serviceModel;

public slots:
    void openEditor();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void switchLanguage(QAction* selectedAction);
    void connectionClicked(QAction* selectedAction);
    void scanNetwork();
    void serviceFound();
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ClientSocket.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

signals:
    void clientSendRequest(quint32 requestID, char* requestData, quint32 requestDataSize);

private:
    Ui::MainWindow *ui;

    UdpSocketThread m_clientSocketThread;
};

#endif // MAINWINDOW_H

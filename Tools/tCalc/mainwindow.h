#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    void            createDialog();
    void            initDialog();

    QComboBox*      m_pTrList = nullptr;
    QRadioButton*   m_pTrDegreeRadio = nullptr;
    QLineEdit*      m_pTrDegreeEdit = nullptr;
    QRadioButton*   m_pTrElectricRadio = nullptr;
    QLineEdit*      m_pTrElectricEdit = nullptr;


    QComboBox*      m_pTcList = nullptr;
    QRadioButton*   m_pTcDegreeRadio = nullptr;
    QLineEdit*      m_pTcDegreeEdit = nullptr;
    QRadioButton*   m_pTcElectricRadio = nullptr;
    QLineEdit*      m_pTcElectricEdit = nullptr;

    QRadioButton*   m_pLinInRadio = nullptr;
    QLineEdit*      m_pLinInValEdit = nullptr;
    QRadioButton*   m_pLinOutRadio = nullptr;
    QLineEdit*      m_pLinOutValEdit = nullptr;
    QLineEdit*      m_pLinInLowEdit = nullptr;
    QLineEdit*      m_pLinInHighEdit = nullptr;
    QLineEdit*      m_pLinOutLowEdit = nullptr;
    QLineEdit*      m_pLinOutHighEdit = nullptr;

    void            conversionTr();
    void            conversionTc();
    void            conversionLin();

private slots:

    void            onTrSensorChanged(int)  { conversionTr(); }
    void            onTrRadio()             { conversionTr(); }
    void            onTrValue(QString)      { conversionTr(); }

    void            onTcSensorChanged(int)  { conversionTc(); }
    void            onTcRadio()             { conversionTc(); }
    void            onTcValue(QString)      { conversionTc(); }

    void            onLinRadio()            { conversionLin(); }
    void            onLinValue(QString)     { conversionLin(); }
};

#endif // MAINWINDOW_H

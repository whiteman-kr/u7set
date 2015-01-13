#pragma once

class DbProgress;

//
//
// ProgressDialog
//
//

class ProgressDialog : public QDialog
{
public:
    ProgressDialog(QWidget* parent, const QString& description, DbProgress* progress);

    static void ShowProgressDialog(QWidget* parent, const QString& description, DbProgress* progress);

protected:
    virtual void timerEvent(QTimerEvent*) override;

protected slots:
    void cancel();

private:
    QString m_description;
    QLabel* m_label;
    QProgressBar* m_progressBar;
    QPushButton* m_cancelButton;
    DbProgress* m_progress;
};

#pragma once

#include <QTextEdit>

class QCompleter;

class QTextEditCompleter : public QTextEdit
{
    Q_OBJECT

public:
	QTextEditCompleter(QWidget *parent = 0);
	~QTextEditCompleter();

	void setCompleter(QCompleter *m_completer);
    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
	void insertCompletion(const QString& completion);

private:
    QString textUnderCursor() const;

private:
	QCompleter* m_completer = nullptr;
};

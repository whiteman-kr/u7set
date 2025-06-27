#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QFormLayout>
#include <QHBoxLayout>

class CsvEditorDialog : public QDialog
{
	Q_OBJECT
public:
	explicit CsvEditorDialog(const QString& csvFile, QWidget* parent = nullptr);

signals:
	void csvSaved(const QString& path);

private slots:
	void onNewFile();
	void onSave();

private:
	QString m_csvFile;
	QTextEdit* m_textEdit;
	QHBoxLayout* buttonsLayout;
	QPushButton* m_saveBtn;
	QPushButton* m_newFileBtn;
};
#pragma once
#include <QDialog>

class QPushButton;
class QTextEdit;
class QHBoxLayout;

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
	QTextEdit* m_textEdit = nullptr;
	QHBoxLayout* m_buttonsLayout = nullptr;
	QPushButton* m_saveBtn = nullptr;
	QPushButton* m_newFileBtn = nullptr;
};
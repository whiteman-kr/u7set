#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;

class DialogImport : public QDialog
{
	Q_OBJECT
public:
	explicit DialogImport(const QString& comparatorsFile, const QString& appSignalsFile, QDialog* parent = nullptr);

	QString comparatorsFile() const;
	QString appSignalsFile() const;

private:
	void accept() override;

	QString m_comparatorsFile;
	QString m_appSignalsFile;

private:
	QLineEdit* m_fileComparatorsEdit = nullptr;
	QLineEdit* m_fileAppSignalsEdit = nullptr;
	QLabel* m_statusLabel = nullptr;
};

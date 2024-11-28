#ifndef DIALOGSCHEMASEXPORT_H
#define DIALOGSCHEMASEXPORT_H

#include "../Builder/SchemasReportGenerator.h"

namespace Ui {
	class DialogSchemasExport;
}

class DialogSchemasExport : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSchemasExport(const Builder::SchemasReportOptions& options, const QString& defaultPath, const QString& defaultFile, QWidget *parent = nullptr);
	~DialogSchemasExport();

	const Builder::SchemasReportOptions& options() const;

	bool isSingleFile() const;
	const QString& pathName() const;
	const QString& fileName() const;

private slots:
	void on_buttonPathBrowse_clicked();
	void on_buttonFileBrowse_clicked();

private:
	virtual void accept() override;

private:
	Ui::DialogSchemasExport *ui;

	Builder::SchemasReportOptions m_options;
	QString m_pathName;
	QString m_fileName;

	static inline int m_lastTab = 0;
};

#endif // DIALOGSCHEMASEXPORT_H

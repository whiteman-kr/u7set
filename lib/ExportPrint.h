#ifndef EXPORTPRINT_H
#define EXPORTPRINT_H


class ExportPrint : public QObject
{
public:
	ExportPrint(QWidget* parent);

	void printTable(QTableView* tableView);
	void exportTable(QTableView* tableView, QString fileName, QString extension);

protected:
	virtual void generateHeader(QTextCursor& cursor);



private:

	bool exportToTextDocument(QTableView* tableView, QTextDocument* doc, bool onlySelectedRows);

	bool saveArchiveWithDocWriter(QTableView* tableView, QString fileName, QString format);
	bool saveArchiveToCsv(QTableView* tableView, QString fileName);

private:
	QWidget* m_parent = nullptr;

	const int m_maxReportStates = 10000;
	const int m_maxReportStatesForCsv = 5000000;
};

#endif // EXPORTPRINT_H

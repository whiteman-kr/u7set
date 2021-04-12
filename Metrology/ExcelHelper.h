#ifndef EXCELHELPER_H
#define EXCELHELPER_H

#include <ActiveQt/qaxobject.h>
#include <ActiveQt/qaxbase.h>
#include <QString>

//Expected in .pro file: QT += axcontainer
//
class ExcelExportHelper
{
public:

	ExcelExportHelper(const ExcelExportHelper& other) = delete;
	explicit ExcelExportHelper(bool closeExcelOnExit = false);
	~ExcelExportHelper();

private:

	QAxObject*			m_pExcelApplication = nullptr;
	QAxObject*			m_pWorkbooks = nullptr;
	QAxObject*			m_pWorkbook = nullptr;
	QAxObject*			m_pSheets = nullptr;
	QAxObject*			m_pSheet = nullptr;

	bool				m_closeExcelOnExit = false;

public:

	bool				setCellValue(int lineIndex, int columnIndex, const QString& value);
	void				saveAs(const QString& fileName);

	ExcelExportHelper&	operator=(const ExcelExportHelper& other) = delete;
};

#endif // EXCELHELPER_H

//	example
//
//	try
//	{
//		const QString fileName = "c:\\test.xlsx";

//		ExcelExportHelper helper;

//		helper.SetCellValue(1,1,"Text-Cell_A1");
//		helper.SetCellValue(1,2,"Text-Cell_A2");

//		helper.SaveAs(fileName);
//	}
//	catch (const exception& e)
//	{
//		QMessageBox::critical(this, "Error - Demo", e.what());
//	}

#ifndef EXCELHELPER_H
#define EXCELHELPER_H

#ifdef Q_OS_WIN

	#include <ActiveQt/qaxobject.h>
	#include <ActiveQt/qaxbase.h>

#endif

#include <QString>

// ==============================================================================================

class ExcelExportHelper
{
public:

	ExcelExportHelper(const ExcelExportHelper& other) = delete;
	explicit ExcelExportHelper(bool closeExcelOnExit = false);
	~ExcelExportHelper();

private:

	#ifdef Q_OS_WIN

		QAxObject*			m_pExcelApplication = nullptr;
		QAxObject*			m_pWorkbooks = nullptr;
		QAxObject*			m_pWorkbook = nullptr;
		QAxObject*			m_pSheets = nullptr;
		QAxObject*			m_pSheet = nullptr;

		bool				m_closeExcelOnExit = false;

	#endif

public:

	bool				setCellValue(int lineIndex, int columnIndex, const QString& value);
	void				saveAs(const QString& fileName);

	ExcelExportHelper&	operator=(const ExcelExportHelper& other) = delete;
};

#endif // EXCELHELPER_H

// ==============================================================================================

//	example.cpp
//
//Expected in .pro file: QT += axcontainer
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

// ==============================================================================================

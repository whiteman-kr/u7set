#include <ActiveQt/qaxobject.h>
#include <ActiveQt/qaxbase.h>

#include <QString>
#include <QFile>
#include <stdexcept>

using namespace std;

#include "ExcelHelper.h"

ExcelExportHelper::ExcelExportHelper(bool closeExcelOnExit) :
	m_pExcelApplication (nullptr),
	m_pWorkbooks (nullptr),
	m_pWorkbook (nullptr),
	m_pSheets (nullptr),
	m_pSheet (nullptr),
	m_closeExcelOnExit (closeExcelOnExit)
{
	m_pExcelApplication = new QAxObject("Excel.Application", nullptr);//{00024500-0000-0000-C000-000000000046}
	if (m_pExcelApplication == nullptr)
	{
		throw invalid_argument("Failed to initialize interop with Excel (probably Excel is not installed)");
		return;
	}

	m_pExcelApplication->dynamicCall("SetVisible(bool)", false);	// hide excel
	m_pExcelApplication->setProperty("DisplayAlerts", 0);			// disable alerts

	m_pWorkbooks = m_pExcelApplication->querySubObject("Workbooks");
	if (m_pWorkbooks == nullptr)
	{
		return;
	}

	m_pWorkbook = m_pWorkbooks->querySubObject("Add");
	if (m_pWorkbook == nullptr)
	{
		return;
	}

	m_pSheets = m_pWorkbook->querySubObject("Worksheets");
	if (m_pSheets == nullptr)
	{
		return;
	}

	m_pSheet = m_pSheets->querySubObject("Add");
	if (m_pSheet == nullptr)
	{
		return;
	}
}

ExcelExportHelper::~ExcelExportHelper()
{
	if (m_pExcelApplication != nullptr)
	{
		if (m_closeExcelOnExit == false)
		{
			m_pExcelApplication->setProperty("DisplayAlerts", 1);
			m_pExcelApplication->dynamicCall("SetVisible(bool)", true );
		}

		if (m_pWorkbook != nullptr && m_closeExcelOnExit == true)
		{
			m_pWorkbook->dynamicCall("Close (Boolean)", true);
			m_pExcelApplication->dynamicCall("Quit (void)");
		}
	}

	if (m_pSheet != nullptr)
	{
		delete m_pSheet;
	}

	if (m_pSheets != nullptr)
	{
		delete m_pSheets;
	}

	if (m_pWorkbook != nullptr)
	{
		delete m_pWorkbook;
	}

	if (m_pWorkbooks != nullptr)
	{
		delete m_pWorkbooks;
	}

	if (m_pExcelApplication != nullptr)
	{
		delete m_pExcelApplication;
	}
}

bool ExcelExportHelper::setCellValue(int lineIndex, int columnIndex, const QString& value)
{
	if (m_pSheet == nullptr)
	{
		return false;
	}

	QAxObject *cell = m_pSheet->querySubObject("Cells(int,int)", lineIndex, columnIndex);
	if (cell == nullptr)
	{
		return false;
	}

    cell->setProperty("Value",value);

	delete cell;
	cell = nullptr;

	return true;
}

void ExcelExportHelper::saveAs(const QString& fileName)
{
	if (fileName.isEmpty() == true)
	{
		throw invalid_argument("'fileName' is empty!");
	}

	if (fileName.contains("/") == true)
	{
		throw invalid_argument("'/' character in 'fileName' is not supported by excel!");
	}

	if (QFile::exists(fileName) == true)
    {
		if (QFile::remove(fileName) == false)
        {
            throw new exception(QString("Failed to remove file '%1'").arg(fileName).toStdString().c_str());
        }
    }

	m_pWorkbook->dynamicCall("SaveAs (const QString&)", fileName);
}

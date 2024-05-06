#pragma once

#include <memory>

#include <QString>

class QWidget;
class QTableView;

namespace UiLib
{
	class ExportPrintPrivate;

	class IExportPrint
	{
	public:
		virtual void generateHeader(QTextCursor& cursor) = 0;
	};

	class ExportPrint : public IExportPrint
	{
	public:
		ExportPrint(QWidget* parent);
		virtual ~ExportPrint();

		void printTable(QTableView* tableView);
		void exportTable(QTableView* tableView, QString fileName, QString extension);

	protected:
		virtual void generateHeader(QTextCursor& cursor) override;

	private:
		std::unique_ptr<ExportPrintPrivate> m_impl;
	};
} // namespace UiLib

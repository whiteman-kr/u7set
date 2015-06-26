#include "Stable.h"
#include <QtWidgets/qwidget.h>
#include "Print.h"


namespace VFrame30
{
/*
	class QMyWidget : public QWidget
	{
	public:
		void Create(HWND hWnd)
		{
			create((WId)hWnd, false, false);
		}

	};*/

	CPrint::CPrint(void)
	{
	}

	CPrint::~CPrint(void)
	{
	}

	void CPrint::Add(std::pair<std::shared_ptr<Scheme>, bool> VFrameItem)
	{
		FrameList.push_back(VFrameItem);
	}

	/*
	void CPrint::Print(HWND hWnd, const std::string FileName)
	{

		//QWidget w;
		//w.Create((WId)hWnd);

		if (FrameList.empty() == true)
		{
			return;
		}
		
		QPrinter printer(QPrinter::HighResolution);
	
		// Установить поля
		//
		printer.setPageMargins(10, 10, 10, 10, QPrinter::Millimeter);

		// Установить ориентацию страницы по умолчанию по первому листу
		//
		pair<shared_ptr<VFrame30::Scheme>, bool> spFirstFrame  = FrameList.front();

		VFrame30::Scheme* pFirstFrame = spFirstFrame.first.get();
		if (pFirstFrame == nullptr)
		{
			ASSERT(pFirstFrame);
			return;
		}

		printer.setOrientation(pFirstFrame->GetWidth() < pFirstFrame->GetHeight() ? QPrinter::Portrait : QPrinter::Landscape);

		if (FileName.empty() == false)
		{
			// Установка свойств для экспорта в PDF
			//
			qreal l;
			qreal r;
			qreal t;
			qreal b;

			QPageSetupDialog dialog;
			::SetParent(dialog.winId(), hWnd);

			dialog.printer()->setOrientation(printer.orientation());
			printer.getPageMargins(&l, &t, &r, &b, QPrinter::Millimeter);
			dialog.printer()->setPageMargins(l, t, r, b, QPrinter::Millimeter);

			if (dialog.exec() != QDialog::Accepted)
				return;

			printer.setOutputFileName(QString::fromStdString(FileName));
			printer.setPaperSize(dialog.printer()->paperSize());
			printer.setOrientation(dialog.printer()->orientation());
			
			dialog.printer()->getPageMargins(&l, &t, &r, &b, QPrinter::Millimeter);
			printer.setPageMargins(l, t, r, b, QPrinter::Millimeter);

		}
		else
		{
			// Вызов диалога свойств принтера и установка параметров печати
			//
			int SelectedCount = 0;

			for (list <pair<shared_ptr<VFrame30::Scheme>, bool>>::iterator iter = FrameList.begin(); iter != FrameList.end(); ++iter)
			{
				if (iter->second == true)
				{
					SelectedCount++;
				}
			}

			if (SelectedCount > 0 && FrameList.size() > 1)
			{
				printer.setPrintRange(QPrinter::PrintRange::Selection);
			}

			QPrintDialog dialog(&printer, nullptr);
			::SetParent(dialog.winId(), hWnd);

			if (FrameList.size() == 1)
			{
				dialog.setOption(QAbstractPrintDialog::PrintPageRange, false);
			}
			else
			{
				dialog.setMinMax(0, FrameList.size() - 1);
				dialog.setFromTo(0, FrameList.size() - 1);
			}

			if (SelectedCount > 0 && FrameList.size() > 1)
			{
				dialog.setOption(QAbstractPrintDialog::PrintDialogOption::PrintSelection);
			}
			
			dialog.setWindowTitle("Print Document");
			if (dialog.exec() != QDialog::Accepted)
				return;

		}


		int dpi = printer.resolution();

		// Создание объекта рисования для печати
		//
		shared_ptr<VFrame30::CDrawParam> *pSPDrawParam = nullptr;
		try
		{
			pSPDrawParam = new shared_ptr<VFrame30::CDrawParam>(VFrame30::CDrawParam::Create(VFrame30::Direct2D));	
		}
		catch(bad_alloc)
		{
			if (pSPDrawParam != nullptr)
			{
				delete pSPDrawParam;
				pSPDrawParam = nullptr;
			}
			return;
		}

		QPainter *pPainter = (*pSPDrawParam)->GetPainter();
		if (pPainter == nullptr)
		{
			ASSERT (pPainter);
			if (pSPDrawParam != nullptr)
			{
				delete pSPDrawParam;
				pSPDrawParam = nullptr;
			}
			return;
		}

		// Подсчет полного количества печатаемых страниц для вывода NewPage()
		//
		int PageCount = 0;
		int PageCounter = 0;
		for (list <pair<shared_ptr<VFrame30::Scheme>, bool>>::iterator iter = FrameList.begin(); iter != FrameList.end(); iter++, PageCounter++)
		{
			if (printer.printRange() == QPrinter::Selection)
			{
				if (iter->second == false)
					continue;
			}

			if (printer.printRange() == QPrinter::PageRange)
			{
				if (PageCounter < printer.fromPage() || PageCounter > printer.toPage())
					continue;
			}
			
			PageCount++;
		}
		PageCount--;
			
		pPainter->begin(&printer);

		//Печать
		//
		int CurrentPage = 0;
		for (list <pair<shared_ptr<VFrame30::Scheme>, bool>>::iterator iter = FrameList.begin(); iter != FrameList.end(); iter++, CurrentPage++)
		{
			if (printer.printRange() == QPrinter::Selection)
			{
				if (iter->second == false)
					continue;
			}

			if (printer.printRange() == QPrinter::PageRange)
			{
				if (CurrentPage < printer.fromPage() || CurrentPage > printer.toPage())
					continue;
			}

			VFrame30::Scheme* pFrame = iter->first.get();
			if (pFrame == nullptr)
			{
				ASSERT(pFrame);
				continue;
			}

			double Width = pFrame->GetDocumentWidth(dpi, 100);
			double Height = pFrame->GetDocumentHeight(dpi, 100);

			QRectF pgRectPix = printer.pageRect(QPrinter::DevicePixel);

			double kX = pgRectPix.width() / Width;
			double kY = pgRectPix.height() / Height;

			double K = kX < kY ? kX : kY; // Выбираем меньший коэффициент расширения из двух (приводим размер рисунка к размеру листа по наиболее близкой по размеру стороне)

			// установка масштаба
			//
			(*pSPDrawParam)->Ajust(pFrame->GetUnit(), 0, 0, 100 * K, dpi, dpi);

			// печать кадра
			//
			QRectF fr(0, 0, (float)pgRectPix.width(), (float)pgRectPix.height());
			pFrame->Draw(pSPDrawParam->get(), fr);
			
			// Вывод рамки и названия кадра
			//
			pPainter->resetTransform();
			pPainter->drawRect(QRectF(0, 0, pgRectPix.width(), (int)pgRectPix.height()));

			QString StrID = QString((QChar*)pFrame->GetStrID().c_str());
			QString Caption = QString((QChar*)pFrame->GetCaption().c_str());
			pPainter->drawText(QRectF(0, 0, pgRectPix.width(), pgRectPix.height()), StrID + " - " + Caption);

			if (PageCount > 0)
			{
				printer.newPage();
				PageCount--;
			}
		}
	
		pPainter->end();

		//
		if (pSPDrawParam != nullptr)
		{
			delete pSPDrawParam;
			pSPDrawParam = nullptr;
		}
	}
	*/
}

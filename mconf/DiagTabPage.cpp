#include "Stable.h"
#include "DiagTabPage.h"

DiagTabPage::DiagTabPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

DiagTabPage::~DiagTabPage()
{

}

bool DiagTabPage::isFactoryNoValid() const
{
	if (ui.m_pFactoryNo == nullptr)
	{
		return false;
	}

	bool convertResult = false;
	ui.m_pFactoryNo->text().toUInt(&convertResult, 10);

	return convertResult;
}

uint32_t DiagTabPage::factoryNo() const
{
	if (ui.m_pFactoryNo == nullptr)
	{
		assert(ui.m_pFactoryNo);
		return 0;
	}

	bool convertResult = false;
	uint32_t value = ui.m_pFactoryNo->text().toUInt(&convertResult, 10);

	return convertResult ? value : 0;
}

void DiagTabPage::setFactoryNo(uint32_t value)
{
	if (ui.m_pFactoryNo == nullptr)
	{
		assert(ui.m_pFactoryNo);
		return;
	}

	ui.m_pFactoryNo->setText(QString().setNum(value, 10));
	return;
}

bool DiagTabPage::isManufactureDateValid() const
{
	if (ui.m_pManufactureDate == nullptr)
	{
		return false;
	}

	return true;
}

QDate DiagTabPage::manufactureDate() const
{
	if (ui.m_pManufactureDate == nullptr)
	{
		assert(ui.m_pManufactureDate);
		return QDate();
	}
	
	return ui.m_pManufactureDate->date();
}

void DiagTabPage::setManufactureDate(QDate value)
{
	if (ui.m_pManufactureDate == nullptr)
	{
		assert(ui.m_pManufactureDate);
		return;
	}

	ui.m_pManufactureDate->setDate(value);
	return;
}

bool DiagTabPage::isFirmwareCrcValid() const
{
    if (ui.m_pFirwareCrc == nullptr)
	{
		return false;
	}

    QString text = ui.m_pFirwareCrc->text();

	bool convertResult = false;
	text.toUInt(&convertResult, 16);

	return convertResult;
}

uint32_t DiagTabPage::firmwareCrc() const
{
    if (ui.m_pFirwareCrc == nullptr)
	{
        assert(ui.m_pFirwareCrc);
		return 0;
	}
		
    QString text = ui.m_pFirwareCrc->text();

	bool convertResult = false;
	uint32_t value = text.toUInt(&convertResult, 16);

	return convertResult ? value : 0;
}
void DiagTabPage::setFirmwareCrc(uint32_t value)
{
    if (ui.m_pFirwareCrc == nullptr)
	{
        assert(ui.m_pFirwareCrc);
		return;
	}

    ui.m_pFirwareCrc->setText(QString().setNum(value, 16));
	return;
}



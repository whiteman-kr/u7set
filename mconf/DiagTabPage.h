#ifndef DIAGTABPAGE_H
#define DIAGTABPAGE_H

#include <QWidget>
#include "ui_diagtabpage.h"

class DiagTabPage : public QWidget
{
	Q_OBJECT

public:
	DiagTabPage(QWidget *parent = 0);
	~DiagTabPage();

	// Get/set configuration data from/to controls
	//
public:
	bool isFactoryNoValid() const;
	uint32_t factoryNo() const;
	void setFactoryNo(uint32_t value);

	bool isManufactureDateValid() const;
	QDate manufactureDate() const;
	void setManufactureDate(QDate value);

    bool isFirmwareCrcValid() const;
    uint32_t firmwareCrc() const;
    void setFirmwareCrc(uint32_t value);


private:
	Ui::DiagTabPage ui;
};

#endif // DIAGTABPAGE_H

#ifndef AFBLSTORAGE_H
#define AFBLSTORAGE_H

#include "../include/OutputLog.h"
#include "../include/DbController.h"
#include "../VFrame30/Fbl.h"

using namespace Afbl;

class AfblSet
{
public:
	AfblSet();

	bool loadFromDatabase(DbController* db, OutputLog* log, QWidget *parent);

	std::vector<AfbElement> items;

private:


};

#endif // AFBLSTORAGE_H

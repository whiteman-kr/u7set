#pragma once

#include <QDialog>
#include "../../CommonLib/PropertyObject.h"
#include "../../DbLib/DbController.h"


class ProjectPropertiesForm
{
public:
	static bool show(QWidget* parent, DbController* db);
};


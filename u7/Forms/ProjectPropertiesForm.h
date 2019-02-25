#pragma once

#include <QDialog>
#include "../../lib/PropertyObject.h"
#include "../../lib/DbController.h"


class ProjectPropertiesForm
{
public:
	static bool show(QWidget* parent, DbController* db);
};


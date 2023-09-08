#pragma once

#include <QDialog>
#include "../../CommonLib/PropertyObject.h"


class ProjectPropertiesForm
{
public:
	static bool show(QWidget* parent, DbController* db);
};


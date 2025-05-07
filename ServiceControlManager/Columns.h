#pragma once

struct Column
{
	QString caption;
	int width = 0;
};

using Columns = std::vector<Column>;

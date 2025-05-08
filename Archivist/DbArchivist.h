#pragma once

#include "Archivist.h"

class DbArchivist : public Archivist
{
public:
	DbArchivist(const RequestParams& rp);

	bool copyArchive() override;
};

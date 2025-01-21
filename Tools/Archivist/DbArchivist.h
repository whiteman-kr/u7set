#pragma once

#include "Archivist.h"

class DbArchivist : public Archivist
{
public:
	DbArchivist(int argc, char* argv[]);

	void copyArchive() override;
};

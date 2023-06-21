#pragma once

#include "Archivist.h"

class FileArchivist : public Archivist
{
public:
	FileArchivist(int argc, char* argv[]);

	void copyArchive() override;
};

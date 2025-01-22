#pragma once

#include "Archivist.h"
#include <ArchSignal.pb.h>

class FileArchivist : public Archivist
{
public:
	FileArchivist(const RequestParams& rp);

	bool copyArchive() override;

private:
	bool readArchInfoProto();

private:
	Proto::ArchInfo m_archInfo;
};

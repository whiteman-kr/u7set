#include "DbArchivist.h"

DbArchivist::DbArchivist(const RequestParams &rp) :
	Archivist(rp)
{
}

bool DbArchivist::copyArchive()
{
	qDebug() << "Copy DB archive";
	return true;
}

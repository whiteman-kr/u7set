#include "DbArchivist.h"

DbArchivist::DbArchivist(int argc, char* argv[]) :
	Archivist(argc, argv)
{

}

void DbArchivist::copyArchive()
{
	qDebug() << "Copy DB archive";
}

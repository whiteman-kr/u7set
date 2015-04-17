#ifndef NCREPORTDATASOURCEFACTORY_H
#define NCREPORTDATASOURCEFACTORY_H

#include "ncreport_global.h"

class NCReportDataSource;

class NCREPORTSHARED_EXPORT NCReportDataSourceFactory
{
public:
    NCReportDataSourceFactory();
	virtual ~NCReportDataSourceFactory();
	virtual NCReportDataSource * createCustomDataSource( const QString& dsID ) = 0;
};

#endif // NCREPORTDATASOURCEFACTORY_H

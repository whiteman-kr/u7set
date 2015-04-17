#ifndef NCREPORTDUMMYDATASOURCEFACTORY_H
#define NCREPORTDUMMYDATASOURCEFACTORY_H

#include "ncreportdatasourcefactory.h"
#include "ncreport_global.h"

class NCREPORTSHARED_EXPORT NCReportDummyDataSourceFactory : public NCReportDataSourceFactory
{
public:
    NCReportDummyDataSourceFactory();
	virtual NCReportDataSource * createCustomDataSource( const QString& dsID );
};

#endif // NCREPORTDUMMYDATASOURCEFACTORY_H

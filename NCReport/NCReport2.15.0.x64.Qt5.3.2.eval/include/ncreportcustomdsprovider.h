/****************************************************************************
*
*  Copyright (C) 2002-2008 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: nszabo@helta.hu, info@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  nszabo@helta.hu if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTCUSTOMDSPROVIDER_H
#define NCREPORTCUSTOMDSPROVIDER_H

#include "ncreport_global.h"
#include "ncreportdatasource.h"

#include <QObject>

class NCReportDataSource;

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

/*!
 * Represents a custom data source provider, subclass to provide
 * the custom data sources for your application.
 */
class NCREPORTSHARED_EXPORT NCReportCustomDSProvider : public QObject
{
   Q_OBJECT

public:
   NCReportCustomDSProvider( QObject *parent=0);
   virtual ~NCReportCustomDSProvider();

   //! Return a custom data source for the given identifier.
   /*!
    * Return a custom data source for the given identifier.
    * This function returns NULL if there is no custom data source for
    * the identifier.
    * \param dsID Data source identifier.
    * \return The data source created or NULL.
    */
   virtual NCReportDataSource *createCustomDataSource( const QString& dsID, NCReportDataSource::DataSourceType dstype, QObject* parent ) = 0;

private:
};

#endif

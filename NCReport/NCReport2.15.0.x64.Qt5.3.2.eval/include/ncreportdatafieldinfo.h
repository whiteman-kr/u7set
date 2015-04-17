// -*- mode: C++ -*-
// Copyright(c) 2011 SENER Ingenieria y Sistemas.
// All rights reserved.

#ifndef __NCReportDataFieldInfo_H
#define __NCReportDataFieldInfo_H

#include <QString>

//! Represents the informatioin for a field in a data source.

/*!
 * Represents the informatioin for a field in a data source.
 */
class NCReportDataFieldInfo
{
 public:
   //! Default constructor (not implemented).
   NCReportDataFieldInfo();

   //! Constructor.
   NCReportDataFieldInfo( const QString& name,
                          const QString& briefDescription="",
                          const QString& longDescription="");

   //! Copy constructor.
   NCReportDataFieldInfo( const NCReportDataFieldInfo& fieldInfo);


   //! Destructor.
   virtual ~NCReportDataFieldInfo();

   //! Assignment operator.
   NCReportDataFieldInfo& operator = ( const NCReportDataFieldInfo& fieldInfo);

   //! Returns the fields name.
   QString name() const;

   //! Returns the fields brief description.
   QString briefDescription() const;

   //! Returns the fields long description.
   QString longDescription() const;

 protected:

 private:
   // ========================== Data Members =================================
   QString _name;
   QString _briefDescription;
   QString _longDescription;

   // ============================= Methods ===================================
};

#endif  //__NCReportDataFieldInfo_H

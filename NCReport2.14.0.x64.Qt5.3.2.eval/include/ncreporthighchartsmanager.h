/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport Report Generator System
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  norbert@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTHIGHCHARTSMANAGER_H
#define NCREPORTHIGHCHARTSMANAGER_H

#ifdef HIGHCHARTS_INTEGRATION

#include <QObject>
#include <QStringList>
#include <QLatin1String>
#include <QScriptValue>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QVariant>
#include <QPixmap>

QT_BEGIN_NAMESPACE
class QWidget;
class QLayout;
class QFormLayout;
class QTextEdit;
class QWebPage;
QT_END_NAMESPACE

class NCReportDef;
class NCReportDataSource;

class NCReportHighChartsManager : public QObject
{
	Q_OBJECT
public:
	explicit NCReportHighChartsManager(QObject *parent = 0);
	enum ProcessingMode { ReadMode=0, WriteMode };

	QString templateFile() const;
	void setTemplateFile( const QString& fileName );
	QString resultFile() const;
	void setResultFile( const QString& fileName );
	void setReportData( const QString& reportDataScript );
	void setReportDef( NCReportDef* rdef );

	bool loadTemplate(ProcessingMode mode, QString &reportData);
	bool parseTemplateMetaData(QScriptEngine &engine, QScriptValue &global );
	bool parseReportData(QScriptEngine &engine, QScriptValue &global, const QString &reportData );
	bool evaluateDataSource( QScriptValue& seriesList );
	bool evaluateDedicatedDataSource( const QString &dsID, QScriptEngine &engine, QScriptValue &seriesList, QScriptValue &newSeriesList );
	void setError( const QString& msg );
	//void runChartTest();
	bool load();
	bool write();

	QString objectToScript( const QScriptValue& value, int level=0, bool isLiteralArray=false, const QString& objectName=QString() ) const;
	QString slicedByUpperPos( const QString& str ) const;
	void startToLoadChart();
	QByteArray chartSvg() const;
	QPixmap& chartImage();

	QString errorMsg() const;
	bool error() const;
	//bool loadTemplateDOM();
	//void uniteTemplate(QString& templateData, const QString& reportChartData );
public slots:
	void timeoutError();

signals:
	void chartIsReady();
	
private slots:
	void finishChart(bool ok);

//	void startDownload(const QNetworkRequest & request);	//download SVG
//	void finishDownload();	//download SVG

private:
	bool m_error;
	QString m_errorMsg;
	QString m_templateFileName;
	QString m_outputFileName;
	QTextStream out;
	QWebPage *webPage;
	QByteArray m_chartSvg;
	QString m_reportDataScript;
	QList<NCReportDataSource*> m_dataSourcesToClose;
	QPixmap m_chartImage;
	NCReportDef *m_reportDef;
	//QNetworkAccessManager *nam;

	bool loadContentFromFile(const QString &fileName, QString& content) const;
	bool saveContentToFile(const QString &fileName, const QString& content) const;
	bool parseScript( const QString &script, QScriptEngine &engine, QScriptValue &global);
	void overViewScriptValue( const QScriptValue& value);

	void writeEndElement( const QXmlStreamReader &reader);
	bool loadData( int seriesIdx, const QString &dataRef, const QString &target, QScriptValue &seriesList, QVariant::Type type );
	void renderAsImage();

	bool loadPivotedData( NCReportDataSource *ds, QScriptEngine &engine, QScriptValue &newSeriesList);
	bool loadUnPivotedData( NCReportDataSource *ds, QScriptEngine &engine, QScriptValue &newSeriesList);
	bool loadUnPivotedDataWithCategoryKey(NCReportDataSource *ds, QScriptEngine &engine, QScriptValue &newSeriesList);

};

#endif // HIGHCHARTS_INTEGRATION
#endif // NCREPORTHIGHCHARTSMANAGER_H

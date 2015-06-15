#ifndef CONVERTOR_H
#define CONVERTOR_H

#include <QString>
#include <QTextStream>

class Convertor
{
public:
	static QString start(const QString& inputFilePath, const QString& parentFile);
	static QString convert(const QString& inputFilePath, const QString& parentFile, QString& query);
	static QString findFiles(const QString& dirName, const QString& parentFile, QString& out, QString& filePathes);
	static bool writeToFile(QString& outputFileName, QString query);
};

#endif // CONVERTOR_H

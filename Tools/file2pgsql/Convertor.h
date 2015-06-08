#ifndef CONVERTOR_H
#define CONVERTOR_H

#include <QString>
#include <QTextStream>

class Convertor
{
public:

	static bool start(const QString& inputFilePath, const QString& parentFile, QTextStream& out);

protected:
	static bool convert(const QString& inputFilePath, const QString& parentFile, QTextStream& out);
	static int findFiles(const QString& dirName, const QString& parentFile, QTextStream& out);
};

#endif // CONVERTOR_H

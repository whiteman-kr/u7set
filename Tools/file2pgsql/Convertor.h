#ifndef CONVERTOR_H
#define CONVERTOR_H

#include <QString>
#include <QTextStream>

struct FileQuery
{
	QString path;
	QString addQuery;
	QString deleteQuery;
};

enum class FileQueryType
{
	Add,
	Delete
};

class Convertor
{
public:
	bool createQueryFiles(const QString& inputFileName, const QString& parentFile, std::vector<FileQuery>& queries);

	bool writeToFile(const QString& outputFileName, const QString& header, const std::vector<FileQuery>& queries, FileQueryType queryType);

private:

	bool convertFiles(const QString& dirName, const QString& parentFile, std::vector<FileQuery>& queries);

	bool convert(const QString& inputFilePath, const QString& parentFile, std::vector<FileQuery>& queries);


private:


};

#endif // CONVERTOR_H

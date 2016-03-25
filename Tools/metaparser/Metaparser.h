#ifndef METAPARSER_H
#define METAPARSER_H

#include <QFile>
#include <QVector>
#include <QDebug>

class Metaparser
{
public:
	Metaparser();
	virtual ~Metaparser();

	struct BlockFromFile
    {
        QString issueCode;
        QString issueType;
        QString title;
        QString parameters;
        QString description;
        bool error = false;
        QString errorString;
    };

	void setInputFile(const QString fileName);
	void setOutputFile(const QString fileName);
	int searchBlocks();
	BlockFromFile processBlock(const QStringList& block);
	int writeToHtml();

private:
    QString m_inFileName;
    QString m_outFileName;
	QVector<BlockFromFile> m_commentBlocks;
};

#endif // METAPARSER_H

#include <QCoreApplication>
#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>

#define C_STR(qstring) qstring.toStdString().c_str()

// Also can be used standard Windows utilite - certutil.exe:
//
// certutil.exe -hashfile fileName MD5
//

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc == 1 ||  argc > 3)
	{
		qDebug() << "\nUse: md5.exe [options] fileName\n";
		qDebug() << "Options:\n\t-b\topen file in binary mode (default)";
		qDebug() << "\t-t\topen file in text mode";

		return 1;
	}

	bool binMode = true;

	int filenameIndex = 1;

	if (argc == 3)
	{
		filenameIndex = 1;
		QString option(argv[1]);

		option = option.trimmed().toLower();

		if (option == "-t")
		{
			binMode = false;
		}

		filenameIndex = 2;
	}

	QString fileName(argv[filenameIndex]);

	fileName = fileName.trimmed();

	QFile file(fileName);

	QIODevice::OpenMode flags = QIODevice::ReadOnly;

	if (binMode == false)
	{
		flags |= QIODevice::Text;
	}

	if (file.open(flags) == false)
	{
		qDebug() << C_STR(QString("\nFile open error: %1").arg(fileName));

		return 1;
	}

	QCryptographicHash md5Generator(QCryptographicHash::Md5);

	file.seek(0);

	md5Generator.addData(&file);

	QString md5 = QString(md5Generator.result().toHex());

	qDebug() << C_STR(QString("\nFile:\t%1").arg(fileName));
	qDebug() << C_STR(QString("Mode:\t%1").arg(binMode == true ? "binary" : "text"));
	qDebug() << C_STR(QString("MD5:\t%1").arg(md5));

	file.close();

	return 0;	//a.exec();
}


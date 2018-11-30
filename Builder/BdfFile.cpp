#include "BdfFile.h"


BdfFile::BdfFile()
{
	initCoords();

	m_stringList.append("(header \"graphic\" (version \"1.3\"))");
}


BdfFile::~BdfFile(void)
{
}


void BdfFile::addConnector1(const QString& connectorName, bool restart)
{
	addConnectorFunction(connectorName, 1, restart);
}


void BdfFile::addConnector16(const QString& connectorName, bool restart)
{
	addConnectorFunction(connectorName, 16, restart);
}


void BdfFile::addConnector32(const QString& connectorName, bool restart)
{
	addConnectorFunction(connectorName, 32, restart);
}


void BdfFile::addConnector(const QString& connectorName, int connectorWidth, bool restart)
{
	addConnectorFunction(connectorName, connectorWidth, restart);
}


void BdfFile::initCoords()
{
	m_y1 = 0;
	m_y2 = 12;
	m_y3 = 16;
}


void BdfFile::addConnectorFunction(QString connectorName, int connectorWidth, bool restart)
{
	connectorName = connectorName.replace(QLatin1String("#"), QLatin1String(""));

	const int x1 = 136;
	const int x2 = 337;
	const int x3 = 128;
	const int x4 = 280;

	if (restart == true)
	{
		initCoords();
	}

	QString str;

	str = "(connector\n\t(text ";

	if (connectorWidth > 1)
	{
		str += QString("\"%1[%2..0]\" ").arg(connectorName).arg(connectorWidth - 1);
	}
	else
	{
		str += QString("\"%1\" ").arg(connectorName);
	}

	str += QString("(rect %1 %2 %3 %4)(font \"Arial\" (font_size 8)))\n").arg(x1).arg(m_y1).arg(x2).arg(m_y2);
	str += QString("\t(pt %1 %2)\n").arg(x3).arg(m_y3);
	str += QString("\t(pt %1 %2)\n").arg(x4).arg(m_y3);

	if (connectorWidth > 1)
	{
		str += "\t(bus)\n)";
	}
	else
	{
		str += ")";
	}

	m_stringList.append(str);

	spacing();
}


void BdfFile::addText(const QString& textStr)
{
	int x1 = 136;
	int x2 = 337;

	QString str = QString("(text \"%1\" (rect %2 %3 %4 %5)(font \"Arial\" (font_size 8)))").
			arg(textStr).arg(x1).arg(m_y1).arg(x2).arg(m_y2);

	m_stringList.append(str);

	spacing();
}


void BdfFile::spacing()
{
	m_y1 += 16;
	m_y2 += 16;
	m_y3 += 16;
}

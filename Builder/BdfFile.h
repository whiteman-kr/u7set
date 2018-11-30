#pragma once

#include <QtCore>

class BdfFile
{

public:
	BdfFile();
	virtual ~BdfFile();

	void addConnector1(const QString& connectorName, bool restart = false);
	void addConnector16(const QString& connectorName, bool restart = false);
	void addConnector32(const QString& connectorName, bool restart = false);
	void addConnector(const QString& connectorName, int connectorWidth, bool restart = false);
	void addText(const QString& textStr);
	void spacing();

	const QStringList& stringList() const { return m_stringList; }

private:
	void initCoords();
	void addConnectorFunction(QString connectorName, int connectorWidth, bool restart);

private:
	QStringList m_stringList;

	int m_y1 = 0;
	int m_y2 = 0;
	int m_y3 = 0;
};

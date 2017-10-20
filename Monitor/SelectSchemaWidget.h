#ifndef SELECTSCHEMAWIDGET_H
#define SELECTSCHEMAWIDGET_H

#include <QWidget>

class SelectSchemaWidget : public QWidget
{
	Q_OBJECT

	struct SchemaItem
	{
		QString schemaId;
		QString caption;
	};

public:
	explicit SelectSchemaWidget(QWidget* parent = nullptr);

public:
	void clear();
	void addSchema(QString schemaId, QString caption);

	bool setCurrentSchema(QString schemaId);
	bool setCurrentSchema(int index);

	int currentIndex() const;
	QString currentSchemaId() const;

signals:

protected slots:

	// Data
	//
private:
	int m_currentIndex = -1;
	std::vector<SchemaItem> m_schemas;
};

#endif // SELECTSCHEMAWIDGET_H

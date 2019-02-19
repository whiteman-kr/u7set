#pragma once

#include <QDialog>
#include "../../lib/PropertyObject.h"
#include "../../lib/DbController.h"


class ProjectProperties : public PropertyObject
{
public:
	ProjectProperties();

	bool load(QWidget* parent, DbController* db);
	bool save(QWidget* parent, DbController* db);

public:
	QString description() const;
	void setDescription(const QString& value);

	std::vector<int> suppressWarnings() const;
	QString suppressWarningsAsString() const;
	void setSuppressWarnings(const QString& value);

	bool uppercaseAppSignalId() const;
	void setUppercaseAppSignalId(bool value);

private:
	QString m_description;
	std::vector<int> m_suppressWarnings;
	bool m_uppercaseAppSignalId = true;
};


class ProjectPropertiesForm
{
public:
	static bool show(QWidget* parent, DbController* db);
};


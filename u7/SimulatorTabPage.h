#pragma once
#include "MainTabPage.h"

#include <SimulatorUi/SimWidget.h>


// SimPropertyStorage - implementation of ISimPropertyStorage that uses DbController to store properties
//
class SimPropertyStorage : public SimUi::ISimPropertyStorage
{
public:
	explicit SimPropertyStorage(DbController& dbc, QWidget* parentWidget);

	virtual QStringList getPropertyNames() const override;
	virtual bool removeProperty(QStringView propertyName) const override;

	virtual void saveProperty(QStringView propertyName, QStringView value) override;
	virtual QString loadProperty(QStringView propertyName, QStringView defaultValue, bool* ok) override;

private:
	DbController& m_dbc;
	QWidget* m_parentWidget;
};


// DbProjectStateNotifier - notifies about project state changes (opened/closed)
//
class DbProjectStateNotifier : public SimUi::DbProjectStateNotifier
{
	Q_OBJECT

public:
	explicit DbProjectStateNotifier(DbController& dbc, QObject* parent = nullptr);

private:
	DbController& m_dbc;
};


//
//
// SimulatorTabPage
//
//
class SimulatorTabPage : public MainTabPage
{
	Q_OBJECT

public:
	explicit SimulatorTabPage(DbController* dbc, QWidget* parent);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

private:
	QString getProjectPathFunc(QWidget* parent);

	// Data
	//
private:
	SimPropertyStorage m_propertyStorage;
	DbProjectStateNotifier m_dbProjectStateNotifier;
	SimUi::SimWidget* m_simulatorWidget = nullptr;
};

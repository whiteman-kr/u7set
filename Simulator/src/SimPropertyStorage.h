#pragma once
#include <SimulatorUi/ISimPropertyStorage.h>

#include <QSettings>

// SimPropertyStorage - implementation of ISimPropertyStorage that uses QSettings to store properties
//
class SimPropertyStorage : public SimUi::ISimPropertyStorage
{
public:
	// SimUi::ISimPropertyStorage interface implementation
	//
	QStringList getPropertyNames() const override;
	bool removeProperty(QStringView propertyName) const override;
	void saveProperty(QStringView propertyName, QStringView value) override;
	QString loadProperty(QStringView propertyName, QStringView defaultValue = QStringView{}, bool* ok = nullptr) override;

private:
	QStringView m_prefix = u"SimPropertyStorage";
	mutable QSettings m_settings;
};
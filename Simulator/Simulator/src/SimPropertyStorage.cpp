#include "SimPropertyStorage.h"

QStringList SimPropertyStorage::getPropertyNames() const
{
	m_settings.beginGroup(m_prefix);
	auto keys = m_settings.childKeys();
	m_settings.endGroup();
	return keys;
}

bool SimPropertyStorage::removeProperty(QStringView propertyName) const
{
	m_settings.remove(m_prefix.toString() + "/" + propertyName.toString());
	return true;
}

void SimPropertyStorage::saveProperty(QStringView propertyName, QStringView value)
{
	m_settings.setValue(m_prefix.toString() + "/" + propertyName.toString(), value.toString());
	return;
}

QString SimPropertyStorage::loadProperty(QStringView propertyName, QStringView defaultValue /*= QStringView{}*/, bool* ok /*= nullptr*/)
{
	auto value = m_settings.value(m_prefix.toString() + "/" + propertyName.toString(), defaultValue.toString());
	if (ok != nullptr)
	{
		*ok = value.isValid();
	}

	return value.toString();
}

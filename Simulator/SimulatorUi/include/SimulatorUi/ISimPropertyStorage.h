#pragma once
#include <QStringView>

namespace SimUi
{
	class ISimPropertyStorage
	{
	public:
		virtual ~ISimPropertyStorage() = default;

		virtual QStringList getPropertyNames() const = 0;
		virtual bool removeProperty(QStringView propertyName) const = 0;

		virtual void saveProperty(QStringView propertyName, QStringView value) = 0;
		virtual QString loadProperty(QStringView propertyName, QStringView defaultValue = QStringView{}, bool* ok = nullptr) = 0;
	};
} // namespace SimUi
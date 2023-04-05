#include "LocatorProvider.h"

namespace Locator
{
	QString LocatorProvider::name() const
	{
		Q_ASSERT(false);
		return {};
	}

	void LocatorProvider::locateFor(const QString& /*text*/)
	{
		Q_ASSERT(false);
		return;
	}

	void LocatorProvider::stopSearching()
	{
		return;
	}

	void LocatorProvider::slot_locateFor(QString text)
	{
		return locateFor(text);
	}

	void LocatorProvider::slot_stopSearching()
	{
		return stopSearching();
	}
}

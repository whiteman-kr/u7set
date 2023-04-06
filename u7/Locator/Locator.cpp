#include "Locator.h"
#include "LocatorListWidget.h"

namespace Locator
{
	Locator::Locator(LocatorListWidget& listWidget) :
		QObject{},
		m_listWidget{listWidget}
	{
	}

	void Locator::addProvider(LocatorProvider& locatorProvider)
	{
		// Signals to provider
		//
		connect(this, &Locator::signal_locateFor, &locatorProvider, &LocatorProvider::slot_locateFor, Qt::QueuedConnection);
		connect(this, &Locator::signal_stopSearching, &locatorProvider, &LocatorProvider::slot_stopSearching, Qt::QueuedConnection);

		// Signals from provider
		//
		connect(&locatorProvider, &LocatorProvider::resultReady, this, &Locator::resultReady, Qt::QueuedConnection);

		return;
	}

	void Locator::setText(const QString& text)
	{
		if (m_text != text)
		{
			m_text = text;
			emit signal_locateFor(m_text);
		}

		return;
	}

	void Locator::stopSearching()
	{
		emit signal_stopSearching();
	}

	void Locator::resultReady(QString text, LocatorProvider* provider, const std::vector<LocatedItem>& items)
	{
//		qDebug() << "Locator::resultReady: provider name: " << provider->name() << ", text: " << text;
//		for (const auto& item : items)
//		{
//			qDebug() << "\t item: " << item.what;
//		}

		if (text != m_text)
		{
			// Providers can send data with a lag, filter outdated results.
			//
			return;
		}

		// --
		//
		m_listWidget.addData(text, provider->name(), items);
	}
}

#include "AppSignalLocatorProvider.h"
#include "../AppSignalSetProvider.h"

namespace Locator
{
	QString AppSignalLocatorProvider::name() const
	{
		return QStringLiteral("AppSignal");
	}

	void AppSignalLocatorProvider::locateFor(const QString& text)
	{
		std::vector<LocatedItem> result;

		if (m_signalProvider == nullptr || text.size() < 2)
		{
			emit resultReady(text, this, result);
			return;
		}

		result.reserve(64);

		// Blocking search.
		//
		const auto& allSignals = m_signalProvider->signalsVector();

		// Filter signals.
		//
		auto filteresSignals = std::views::filter(allSignals, [&text](const auto appSignal) {
			return appSignal->appSignalID().startsWith(text, Qt::CaseInsensitive);
		});

		// Compose result.
		//
		for (const AppSignal* appSignal : filteresSignals)
		{
			if (result.size() > 1023)
			{
				// Enough
				//
				result.emplace_back("...", tr("...more signals..."));
				break;
			}

			result.emplace_back(appSignal->appSignalID(), appSignal->caption());
		}

		// Emit results.
		//
		std::sort(result.begin(), result.end(), [](const auto& l, const auto& r) { return l.what < r.what; });
		emit resultReady(text, this, std::move(result));
		return;
	}

	void AppSignalLocatorProvider::stopSearching()
	{
		// This is blocking provider, stop searching is not applicable in this case.
		//
		return;
	}

	AppSignalSetProvider* AppSignalLocatorProvider::signalProvider() const
	{
		return m_signalProvider;
	}

	void AppSignalLocatorProvider::setSignalProvider(AppSignalSetProvider* value)
	{
		m_signalProvider = value;
	}
}


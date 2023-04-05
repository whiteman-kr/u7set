#pragma once
#include "LocatorProvider.h"

class AppSignalSetProvider;

namespace Locator
{
	class AppSignalLocatorProvider : public LocatorProvider
	{
		Q_OBJECT

	public:
		[[nodiscard]] virtual QString name() const override;

	protected:
		virtual void locateFor(const QString& text) override;
		virtual void stopSearching() override;

	public:
		AppSignalSetProvider* signalProvider() const;
		void setSignalProvider(AppSignalSetProvider* value);

	private:
		AppSignalSetProvider* m_signalProvider = nullptr;
	};
}

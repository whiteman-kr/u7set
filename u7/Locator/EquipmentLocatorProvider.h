#pragma once
#include "LocatorProvider.h"

class EquipmentTabPage;

namespace Locator
{
	class EquipmentLocatorProvider : public LocatorProvider
	{
		Q_OBJECT

	public:
		[[nodiscard]] virtual QString name() const override;

	protected:
		virtual void locateFor(const QString& text) override;
		virtual void stopSearching() override;

	public:
		const EquipmentTabPage* equipmentTabPage() const;
		void setEquipmentTabPage(const EquipmentTabPage* value);

	private:
		const EquipmentTabPage* m_equipmentTabPage = nullptr;
	};
}

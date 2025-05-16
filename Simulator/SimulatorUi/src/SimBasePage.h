#pragma once

#include <QWidget>

#include <list>

class QAction;

namespace SimUi
{
	class SimIdeSimulator;
	class SimLogicModulePage;
	class SimConnectionPage;
	class SimSelectSchemaPage;


	class SimBasePage : public QWidget
	{
		Q_OBJECT

	public:
		explicit SimBasePage(SimIdeSimulator* simulator, QWidget* parent = nullptr);
		virtual ~SimBasePage();

	public:
		static void deleteAllPages();

		static SimLogicModulePage* logicModulePage(QString lmEquipmnetId, QWidget* parent);
		static SimConnectionPage* connectionPage(QString connectionId, QWidget* parent);
		static SimSelectSchemaPage* selectSchemaPage(QWidget* parent);

	protected:
		QAction* m_closeAction = nullptr;

	private:
		static std::list<SimBasePage*> m_pages;

	protected:
		SimIdeSimulator* m_simulator = nullptr;
	};
} // namespace SimUi
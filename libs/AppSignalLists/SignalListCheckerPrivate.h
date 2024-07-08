#pragma once

namespace AppSignalLists
{
	//
	// DialogCheckAppSignalLists
	//
	class DialogCheckAppSignalLists : public QDialog
	{
		Q_OBJECT

	public:
		DialogCheckAppSignalLists(std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parent);
	};

	//
	// ActiveCheck
	//
	class ActiveCheck
	{
	public:
		ActiveCheck() { ActiveCheck::checkFunctionActive = true; }
		~ActiveCheck() { ActiveCheck::checkFunctionActive = false; }
		static bool active() { return checkFunctionActive; }

	private:
		inline static bool checkFunctionActive = false;
	};

} // namespace AppSignalLists

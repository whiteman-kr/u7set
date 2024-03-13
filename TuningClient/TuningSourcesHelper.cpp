#include <QCoreApplication>
#include <QMessageBox>
#include "TuningSourcesHelper.h"
#include <ClientLib/TuningConnection.h>

namespace ClientLib
{

	int TuningSourcesHelper::sourcesErrorsCount(const std::vector<ClientLib::TuningSource>& sourceStates)
	{
		int result = 0;

		for (const auto& ts : sourceStates)
		{
			for (int i = 0; i < ts.statesCount(); i++)
			{
				if (ts.state(i).isreply() == false && ts.state(i).controlisactive() == true)
				{
					// Control but not valid
					//
					result++;
				}
				else
				{
					result += ts.getErrorsCount(i);
				}
			}
		}

		return result;
	}

	int TuningSourcesHelper::sourcesSorCount(const std::vector<ClientLib::TuningSource>& sourceStates, bool* sorActive, bool* sorValid)
	{
		if (sorValid == nullptr || sorActive == nullptr)
		{
			assert(sorValid);
			assert(sorActive);
			return 0;
		}

		int result = 0;

		*sorActive = false;
		*sorValid = false;

		for (const auto& ts : sourceStates)
		{
			bool sorIsSet = false;

			for (int i = 0; i < ts.statesCount(); i++)
			{
				auto state = ts.state(i);

				if (state.controlisactive() == true)
				{
					*sorActive = true;

					if (state.isreply() == true)
					{
						*sorValid = true;

						if (state.setsor() == true)
						{
							sorIsSet = true;
						}
					}
				}
			}

			if (sorIsSet == true)
			{
				result++;
			}
		}

		return result;
	}

	int TuningSourcesHelper::sourceErrorsCount(const std::vector<ClientLib::TuningSource>& sourceStates, Hash sourceHash)
	{
		int result = 0;

		for (const ClientLib::TuningSource& ts : sourceStates)
		{
			if (::calcHash(ts.equipmentId()) != sourceHash)
			{
				continue;
			}

			for (int i = 0; i < ts.statesCount(); i++)
			{
				if (ts.state(i).isreply() == false && ts.state(i).controlisactive() == true)
				{
					result++;
				}
				else
				{
					result += ts.getErrorsCount(i);
				}
			}
		}

		return result;
	}

	int TuningSourcesHelper::sourceSorCount(const std::vector<ClientLib::TuningSource>& sourceStates, Hash sourceHash, bool* sorActive, bool* sorValid)
	{
		if (sorValid == nullptr || sorActive == nullptr)
		{
			assert(sorValid);
			assert(sorActive);
			return 0;
		}

		*sorActive = false;
		*sorValid = false;

		int result = 0;

		for (const ClientLib::TuningSource& ts : sourceStates)
		{
			if (::calcHash(ts.equipmentId()) != sourceHash)
			{
				continue;
			}

			for (int i = 0; i < ts.statesCount(); i++)
			{
				auto state = ts.state(i);

				if (state.controlisactive() == true)
				{
					*sorActive = true;

					if (state.isreply() == true)
					{
						*sorValid = true;

						if (state.setsor() == true)
						{
							result++;
						}
					}
				}
			}
		}

		return result;
	}

	void TuningSourcesHelper::activateTuningSource(TuningConnection& tuningConnection, const QString& sourceEquipmentId, bool activate, QWidget* parent)
	{
		int sourceStatesCount = tuningConnection.tuningSourceStatesCount(::calcHash(sourceEquipmentId));
		int activeStatesCount = tuningConnection.activatedTuningSourceStatesCount(::calcHash(sourceEquipmentId));

		if (activate == true)
		{
			if (activeStatesCount != 0 && activeStatesCount == sourceStatesCount)
			{
				return;	// All sources are already activated
			}
		}
		else
		{
			if (activeStatesCount == 0)
			{
				return;	// No sources are activated
			}
		}

		QString action = activate ? QObject::tr("activate") : QObject::tr("deactivate");

		if (QMessageBox::warning(parent, qAppName(),
								 QObject:: tr("Are you sure you want to %1 the source %2?")
								 .arg(action)
								 .arg(sourceEquipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}

		bool result = tuningConnection.activateTuningSource(::calcHash(sourceEquipmentId), activate);
		if (result == false)
		{
			QMessageBox::critical(parent, qAppName(), QObject::tr("Source activation/deactivation failed!"));
		}
	}
}

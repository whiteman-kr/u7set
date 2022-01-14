#include "TuningSourcesHelper.h"

#include "../lib/Tuning/TuningTcpClient.h"

void TuningSourcesHelper::isActivationActionsAvailable(std::vector<TuningTcpClient*> clients, const QString& sourceEquipmentId, bool* buttonEnableEnabled, bool* deactivateEnabled)
{
	// Count clients with single LM control mode that contain this source and count active/inactive states

	int clientsCount = 0;
	int activeCount = 0;
	int inactiveCount = 0;

	for (const TuningTcpClient* client : clients)
	{
		if (client->isConnected() == true &&
			client->singleLmControlMode() == true &&
			client->hasTuningSource(::calcHash(sourceEquipmentId)) == true)
		{
			clientsCount++;

			if (client->activeTuningSource() == sourceEquipmentId)
			{
				activeCount++;
			}
			else
			{
				inactiveCount++;
			}
		}
	}

	*buttonEnableEnabled = clientsCount != 0 &&  activeCount < clientsCount;
	*deactivateEnabled = clientsCount != 0 &&  inactiveCount < clientsCount;

	return;
}

void TuningSourcesHelper::activateTuningSourceControl(std::vector<TuningTcpClient*> clients, const QString& sourceEquipmentId, bool enable, QWidget* parent)
{
	// Count clients with single LM control mode that contain this source and count active/inactive states

	std::vector<TuningTcpClient*> activeClients;
	std::vector<TuningTcpClient*> inactiveClients;

	QStringList activeClientsIds;
	QStringList inactiveClientsIds;

	for (TuningTcpClient* client : clients)
	{
		if (client->isConnected() == true &&
			client->singleLmControlMode() == true &&
			client->hasTuningSource(::calcHash(sourceEquipmentId)) == true)
		{
			bool enabled = client->activeTuningSource() == sourceEquipmentId;

			if (enabled != enable)
			{
				if (client->clientIsActive() == true)
				{
					activeClients.push_back(client);
					activeClientsIds.push_back(client->tuningServiceId());
				}
				else
				{
					inactiveClients.push_back(client);
					inactiveClientsIds.push_back(client->tuningServiceId());
				}
			}
		}
	}

	if (activeClients.empty() == true && inactiveClients.empty() == true)
	{
		return;
	}

	QString action = enable ? QObject::tr("activate") : QObject::tr("deactivate");

	if (inactiveClients.empty() == false)
	{
		if (QMessageBox::warning(parent, qAppName(),
								 QObject::tr("Warning!\n\nCurrent clients is not selected as active now in services %1.\n\nAre you sure you want to take control and %2 the source %3?")
								 .arg(inactiveClientsIds.join(','))
								 .arg(action)
								 .arg(sourceEquipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}

		for (TuningTcpClient* client : inactiveClients)
		{
			client->activateTuningSourceControl(sourceEquipmentId, enable, true);
		}

		for (TuningTcpClient* client : activeClients)
		{
			client->activateTuningSourceControl(sourceEquipmentId, enable, false);
		}
	}
	else
	{
		if (activeClients.empty() == false)
		{
			if (QMessageBox::warning(parent, qAppName(),
									 QObject:: tr("Are you sure you want to %1 the source %2?")
									 .arg(action)
									 .arg(sourceEquipmentId),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}

			for (TuningTcpClient* client : activeClients)
			{
				client->activateTuningSourceControl(sourceEquipmentId, enable, false);
			}
		}
	}
}


bool TuningSourcesHelper::takeServicesControl(std::vector<TuningTcpClient*> clients, QWidget* parent)
{
	if (clients.empty() == true)
	{
		return true;
	}

	std::vector<TuningTcpClient*> clientsToTakeControl;

	for (TuningTcpClient* client : clients)
	{
		if (client->isConnected() == true &&
			client->activeTuningSourceCount() != 0 &&
			client->singleLmControlMode() == true &&
			client->clientIsActive() == false)
		{
			clientsToTakeControl.push_back(client);
		}
	}

	QStringList nonActiveClients;
	QStringList sourcesToActivate;

	for (const TuningTcpClient* client : clientsToTakeControl)
	{
		nonActiveClients.push_back(client->tuningServiceId());
		sourcesToActivate.push_back(client->activeTuningSource());
	}

	if (nonActiveClients.empty() == false && sourcesToActivate.empty() == false)
	{
		QString question;

		if (nonActiveClients.size() == 1 && sourcesToActivate.size() == 1)
		{
			question = QObject::tr("Warning!\n\nClient %1 is not selected as active now.\n\nAre you sure you want to take control and activate the source %2?")
					   .arg(nonActiveClients[0])
					.arg(sourcesToActivate[0]);
		}
		else
		{
			question = QObject::tr("Warning!\n\nClients %1 are not selected as active now.\n\nAre you sure you want to take control and activate sources %2?")
					   .arg(nonActiveClients.join(';'))
					   .arg(sourcesToActivate.join(';'));
		}

		if (QMessageBox::warning(parent, qAppName(),
								 question,
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}
	}

	for (TuningTcpClient* client : clientsToTakeControl)
	{
		client->activateTuningSourceControl(client->activeTuningSource(), true, true);
	}

	return true;
}

bool TuningSourcesHelper::clientsHaveSameActiveSource(std::vector<TuningTcpClient*> clients)
{
	bool firstActiveTuningSource = true;
	QString activeTuningSource;

	for (TuningTcpClient* client : clients)
	{
		if (client->isConnected() == true &&
			client->singleLmControlMode() == true)
		{
			// Check if active tuning source matches in all clients
			//
			if (firstActiveTuningSource == true)
			{
				firstActiveTuningSource = false;
				activeTuningSource = client->activeTuningSource();
			}
			else
			{
				if (activeTuningSource != client->activeTuningSource())
				{
					return false;
				}
			}
		}
	}

	return true;
}

#include "InstanceResolver.h"

bool InstanceResolver::init(QString instanceId, bool singleInstance)
{
	if (m_sm != nullptr)
	{
		Q_ASSERT(m_sm == nullptr);
		m_sm.reset();
	}

	// Create memory segment for check single Instance
	//
	m_sm = std::make_unique<QSharedMemory>();
	m_sm->setKey(qAppName() + instanceId);

	bool ok = m_sm->create(sizeof(Data));

	bool result = false;

	if (ok == true)
	{
		// If it was created then it must be initialized
		//
		m_sm->lock();

		Data sharedData{};	// default
		std::memcpy(m_sm->data(), &sharedData, sizeof(Data));

		m_sm->unlock();

		result = true;
	}
	else
	{
		if (m_sm->error() == QSharedMemory::SharedMemoryError::AlreadyExists &&
			singleInstance == true)
		{
			// Another instance is exist
			// 1. Request it to activate
			// 2. Wait for activation
			// 3. If not activated, then consider that process is dead and we became Instance
			//

			// 1. Request it to activate
			// In other way, if memory segment exists, write there value "1" and exit.
			//
			{
				bool aok = m_sm->attach();
				Q_ASSERT(aok);

				bool lok = m_sm->lock();
				Q_ASSERT(lok);

				Data data;
				std::memcpy(&data, m_sm->data(), sizeof(Data));

				data.state = State::ActivationRequested;

				std::memcpy(m_sm->data(), &data, sizeof(Data));

				bool uok = m_sm->unlock();
				Q_ASSERT(uok);
			}

			// 2. Wait for activation
			//
			bool activated = false;
			for (int i = 0; i < 50; i++)
			{
				QThread::currentThread()->msleep(100);

				m_sm->attach(QSharedMemory::ReadOnly);
				m_sm->lock();

				Data data;
				std::memcpy(&data, m_sm->data(), sizeof(Data));

				if (data.state == State::Idle)
				{
					activated = true;
					i = 1000;	// it means break;
				}

				m_sm->unlock();
			}

			if (activated == true)
			{
				// This instance must be closed, another instance has been activated
				//
				result = false;
			}
			else
			{
				// 3. If not activated, then consider that process is dead and we became Instance
				//

				// Allow this intance to be activated
				//
				result = true;
			}
		}
		else
		{
			// If other error occured - show it on debug console
			//
			//qDebug() << "Shared memory: " << m_sm->errorString();
			bool aok = m_sm->attach();
			Q_ASSERT(aok);

			result = true;	// Its ok, returning true allows to run this instace
		}
	}

	if (result == true)
	{
		m_timerId = startTimer(50);
	}
	else
	{
		m_sm.reset();
	}

	return result;
}

bool InstanceResolver::reinit(QString instanceId, bool singleInstance)
{
	killTimer(m_timerId);
	m_timerId = 0;

	m_sm.reset();
	return init(instanceId, singleInstance);
}

void InstanceResolver::timerEvent(QTimerEvent* /*event*/)
{
	if (m_sm == nullptr)
	{
		Q_ASSERT(m_sm);
		return;
	}

	Data data;
	bool ok = false;

	{
		Q_ASSERT(m_sm->isAttached());

		ok = m_sm->lock();
		Q_ASSERT(ok);

		std::memcpy(&data, m_sm->constData(), sizeof(Data));

		ok = m_sm->unlock();
		Q_ASSERT(ok);
	}

	if (data.state == State::ActivationRequested)
	{
		emit activate();

		m_sm->lock();
		Q_ASSERT(ok);

		data.state = State::Idle;
		std::memcpy(m_sm->data(), &data, sizeof(Data));

		ok = m_sm->unlock();
		Q_ASSERT(ok);
	}

	return;
}

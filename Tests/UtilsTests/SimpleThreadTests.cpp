// Functional tests for class SimpleThread
//

#include <QTimer>
#include <QRandomGenerator>
#include "../../UtilsLib/WUtils.h"
#include <ClientLib/ConfigController.h>
#include <CommonLib/HostAddressPort.h>
#include "../../OnlineLib/BuildInfo.h"
#include "../../OnlineLib/SoftwareInfo.h"

class ThreadWorker : public SimpleThreadWorker
{
public:
	ThreadWorker(const QString& workerName) :
		SimpleThreadWorker(workerName)
	{
	}

private:
	void onTimer()
	{
		QThread::msleep(QRandomGenerator::global()->bounded(10));

		//qDebug() << C_STR(QString("%1::onTimer").arg(workerName()));
	}

protected:
	virtual void onThreadStarted()
	{
		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &ThreadWorker::onTimer);

		int period = QRandomGenerator::global()->bounded(20);

		m_timer->start(period);

		qDebug() << C_STR(QString("%1 starts timer, period %2").arg(workerName()).arg(period));
	}

	virtual void onThreadFinished()
	{
		QThread::msleep(QRandomGenerator::global()->bounded(100));

		m_timer->stop();
		delete m_timer;
	}

private:
	QTimer* m_timer = nullptr;
};

/*
TEST(SimpleThreadTests, startStopThread)
{
	const int TEST_COUNT = 5;
	const int WORKERS_COUNT = 20;

	for(int r = 0; r < TEST_COUNT; r++)
	{
		SimpleThread st;

		for(int i = 0; i < WORKERS_COUNT; i++)
		{
			st.addWorker(new ThreadWorker(QString("ThreadWorker_%1").arg(i + 1)));
		}

		st.start();

		QThread::msleep(3000);

		bool finishedOk = st.quitAndWait(3000);

		EXPECT_EQ(finishedOk, true);
		EXPECT_EQ(st.finishedWorkersCount(), WORKERS_COUNT);
	}
}

TEST(SimpleThreadTests, startStopMultipleThreads)
{
	const int TEST_COUNT = 10;
	const int THREADS_COUNT = 5;
	const int WORKERS_COUNT = 20;

	for(int r = 0; r < TEST_COUNT; r++)
	{
		SimpleThread sts[THREADS_COUNT];

		for(int t = 0; t < THREADS_COUNT; t++)
		{
			SimpleThread& st = sts[t];

			for(int i = 0; i < WORKERS_COUNT; i++)
			{
				st.addWorker(new ThreadWorker(QString("ThreadWorker_%1").arg(i + 1)));
			}

			st.start();
		}

		QThread::msleep(3000);

		for(int t = 0; t < THREADS_COUNT; t++)
		{
			SimpleThread& st = sts[t];

			bool finishedOk = st.quitAndWait(3000);

			EXPECT_EQ(finishedOk, true);
			EXPECT_EQ(st.finishedWorkersCount(), WORKERS_COUNT);
		}
	}
}
*/

TEST(SimpleThreadTests, cfgLoaderTests)
{
	for(int i = 0; i < 10; i++)
	{
		SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

		HostAddressPort host1{"127.0.0.1", 13312};		// valid address, where cfgservice is expected to run.
		HostAddressPort host2{"192.168.99.103", 8888};	// some unreachable address
		ILogFileStub log;

		ClientLib::ConfigController* cfgController = new ClientLib::ConfigController(softwareInfo, host1, host2, &log);

		cfgController->start();

		delete cfgController;
	}
}

// Functional tests for class SimpleThread
//

#include <gtest/gtest.h>

#include <atomic>
#include <memory>

#include <QObject>
#include <QTimer>
#include <QThread>

#include "SimpleThread.h"

// Helper: loops through events and waits until cond() becomes true or timeout expires.
//
static bool waitUntil(const std::function<bool()>& cond, int timeoutMs = 3000, int pumpSliceMs = 5)
{
	QElapsedTimer t;
	t.start();

	while (!cond())
	{
		QCoreApplication::processEvents(QEventLoop::AllEvents, pumpSliceMs);

		if (t.elapsed() >= timeoutMs)
		{
			return false;
		}

		QThread::msleep(1);
	}

	return true;
}

class TestWorker : public SimpleThreadWorker
{
	Q_OBJECT
public:
	explicit TestWorker(const QString& name, bool periodicWork = true, QObject* parent = nullptr)
		: SimpleThreadWorker(name)
		, m_periodic(periodicWork)
	{
		Q_UNUSED(parent);
	}

signals:
	void startedSig();
	void finishedSig();

protected:
	void onThreadStarted() override
	{
		emit startedSig();

		if (m_periodic)
		{
			m_timer = new QTimer(this);
			m_timer->setInterval(1);
			connect(m_timer, &QTimer::timeout, this, [this]()
					{
						++m_ticks;
					});
			m_timer->start();
		}
	}

	void onThreadFinished() override
	{
		emit finishedSig();

		if (m_timer != nullptr)
		{
			m_timer->stop();
		}
	}

private:
	QTimer* m_timer { nullptr };
	bool m_periodic { true };
	int  m_ticks { 0 };
};

TEST(SimpleThreadTests, SingleWorker_Lifecycle)
{
	SimpleThread th;

	th.enableLog(true);

	auto* w = new TestWorker("W1");

	std::atomic<int> started { 0 };
	std::atomic<int> finished { 0 };
	std::atomic<int> destroyed { 0 };

	QObject::connect(w, &TestWorker::startedSig, [&started]()
					 {
						 ++started;
					 });
	QObject::connect(w, &TestWorker::finishedSig, [&finished]()
					 {
						 ++finished;
					 });
	QObject::connect(w, &QObject::destroyed, [&destroyed]()
					 {
						 ++destroyed;
					 });

	th.addWorker(w);
	th.start();

	ASSERT_TRUE(waitUntil([&started]() { return started.load() >= 1; }));

	ASSERT_TRUE(th.quitAndWait(3000));

	ASSERT_TRUE(waitUntil([&finished]() { return finished.load() >= 1; }));
	ASSERT_TRUE(waitUntil([&destroyed]() { return destroyed.load() >= 1; }));

	EXPECT_TRUE(th.isFinished());
	EXPECT_FALSE(th.isRunning());
}

TEST(SimpleThreadTests, MultipleWorkers_Lifecycle)
{
	SimpleThread th;

	th.enableLog(true);

	constexpr int N = 5;

	std::vector<TestWorker*> workers;
	workers.reserve(N);

	std::atomic<int> started { 0 };
	std::atomic<int> finished { 0 };
	std::atomic<int> destroyed { 0 };

	for (int i = 0; i < N; ++i)
	{
		auto* w = new TestWorker(QString("W%1").arg(i + 1));
		workers.push_back(w);
		th.addWorker(w);

		QObject::connect(w, &TestWorker::startedSig, [&started]()
						 {
							 ++started;
						 });
		QObject::connect(w, &TestWorker::finishedSig, [&finished]()
						 {
							 ++finished;
						 });
		QObject::connect(w, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });
	}

	th.start();

	ASSERT_TRUE(waitUntil([&started, N]() { return started.load() >= N; }));

	ASSERT_TRUE(th.quitAndWait(3000));

	ASSERT_TRUE(waitUntil([&finished, N]() { return finished.load() >= N; }));
	ASSERT_TRUE(waitUntil([&destroyed, N]() { return destroyed.load() >= N; }));

	EXPECT_TRUE(th.isFinished());
	EXPECT_FALSE(th.isRunning());
}

TEST(SimpleThreadTests, ImmediateQuit_AfterStart)
{
	std::atomic<int> destroyed { 0 };

	{
		SimpleThread th;

		th.enableLog(true);

		auto* w1 = new TestWorker("W1", false);
		auto* w2 = new TestWorker("W2", false);
		auto* w3 = new TestWorker("W3", false);
		auto* w4 = new TestWorker("W4", false);
		auto* w5 = new TestWorker("W5", false);

		QObject::connect(w1, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });

		QObject::connect(w2, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });

		QObject::connect(w3, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });

		QObject::connect(w4, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });

		QObject::connect(w5, &QObject::destroyed, [&destroyed]()
						 {
							 ++destroyed;
						 });

		th.addWorker(w1);
		th.addWorker(w2);
		th.addWorker(w3);
		th.addWorker(w4);
		th.addWorker(w5);

		th.start(2);

		ASSERT_TRUE(th.quitAndWait(3000));

		EXPECT_TRUE(th.isFinished());
		EXPECT_FALSE(th.isRunning());
	}

	QThread::msleep(50);

	ASSERT_TRUE(waitUntil([&destroyed]() { return destroyed.load() >= 5; }));
}

#include "SimpleThreadTests.moc"

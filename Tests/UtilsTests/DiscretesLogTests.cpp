#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QThread>

#include "../../AppDataService/DiscretesLog.h"
#include "../../AppSignalLib/DiscretesLogRecord.h"

#include "Common.h"

//
// Helpers
//

static std::vector<DiscretesLogRecord> readAllRows(const QString& dbPath)
{
	std::vector<DiscretesLogRecord> rows;

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "DLW_TEST_READ");
		db.setDatabaseName(dbPath);

		if (!db.open())
		{
			throw std::runtime_error(("open db failed: " + db.lastError().text()).toStdString());
		}

		{
			QSqlQuery q(db);
			if (!q.exec("SELECT id, recordTime, "
						"plantTime, systemTime, localTime, hash, value, flags, "
						"acknowledged, ackTime, ackSource, ackUser  "
						"FROM DiscretesLog ORDER BY id"))
			{
				db.close();
				QSqlDatabase::removeDatabase("DLW_TEST_READ");
				throw std::runtime_error(("query failed: " + q.lastError().text()).toStdString());
			}

			DiscretesLogRecord r;

			while (q.next())
			{
				DiscretesLog::readDiscretesLogRecord(q, r);

				rows.emplace_back(r);
			}
		}

		db.close();
	}

	QSqlDatabase::removeDatabase("DLW_TEST_READ");
	return rows;
}

static SimpleAppSignalState makeState(	Hash h,
										double v,
										uint32_t flagsAll,
										qint64 plantTime = 0,
										qint64 systemTime = 0,
										qint64 localTime = 0)
{
	SimpleAppSignalState s;

	s.hash = h;
	s.value = v;
	s.flags.all = flagsAll;

	s.time.plant = plantTime;
	s.time.system = systemTime;
	s.time.local = localTime;

	return s;
}

//
// Tests
//

TEST(DiscretesLog, hashToHex_Test)
{
	EXPECT_EQ(DiscretesLog::hashToHex(0x0000000000000000ULL), "0000000000000000");
	EXPECT_EQ(DiscretesLog::hashToHex(0xFFFFFFFFFFFFFFFFULL), "FFFFFFFFFFFFFFFF");
	EXPECT_EQ(DiscretesLog::hashToHex(0x8000000000000000ULL), "8000000000000000");
	EXPECT_EQ(DiscretesLog::hashToHex(0xFEDCBA9876543210ULL), "FEDCBA9876543210");
	EXPECT_EQ(DiscretesLog::hashToHex(0x0123456789ABCDEFULL), "0123456789ABCDEF");
	EXPECT_EQ(DiscretesLog::hashToHex(0xABCDEF0123456789ULL), "ABCDEF0123456789");
}

TEST(DiscretesLog, hexToHash)
{
	EXPECT_EQ(DiscretesLog::hexToHash("0000000000000000"), 0x0000000000000000ULL);
	EXPECT_EQ(DiscretesLog::hexToHash("FFFFFFFFFFFFFFFF"), 0xFFFFFFFFFFFFFFFFULL);
	EXPECT_EQ(DiscretesLog::hexToHash("8000000000000000"), 0x8000000000000000ULL);
	EXPECT_EQ(DiscretesLog::hexToHash("FEDCBA9876543210"), 0xFEDCBA9876543210ULL);
	EXPECT_EQ(DiscretesLog::hexToHash("0123456789ABCDEF"), 0x0123456789ABCDEFULL);
	EXPECT_EQ(DiscretesLog::hexToHash("6789ABCDEF012345"), 0x6789ABCDEF012345ULL);
}

TEST(DiscretesLogWriter, StartStop_CreatesDatabaseFile)
{
	DiscretesLogWriter writer;

	const QString project = "UTestProj";
	const QString equip   = "UTestEq";
	const int hours       = 2;

	writer.start(project, equip, hours, logger);

	const QString dbPath = DiscretesLogWriter::databaseName();

	QFileInfo fi(dbPath);

	EXPECT_TRUE(fi.exists());
	EXPECT_TRUE(fi.isFile());

	writer.stop();
}

TEST(DiscretesLogWriter, InsertBatch)
{
	DiscretesLogWriter writer;

	writer.start("UTestProjB", "UTestEqB", 2, logger);

	writer.clearLog();

	constexpr int CONST_STATES = 3;
	constexpr int VAR_STATES = 7000;

	std::vector<SimpleAppSignalState> states;

	states.reserve(CONST_STATES + VAR_STATES);

	states.push_back(makeState(0x0000000000000000ULL, 0.0, 0xAA55AA55));
	states.push_back(makeState(0x8000000000000000ULL, 1.0, 0x00000001));
	states.push_back(makeState(0xFFFFFFFFFFFFFFFFULL, 0.0, 0xFFFFFFFF));

	qint64 curTimeUTC = QDateTime::currentMSecsSinceEpoch();

	for(int i = 0; i < VAR_STATES;  i++)
	{
		SimpleAppSignalState s = makeState(randomUint64(), i % 2, randomUint32(),
										   curTimeUTC + 3600 * 1000 + i * 5,
										   curTimeUTC + i * 5,
										   curTimeUTC + 3600 * 1000 + i * 5 + i % 4);
		states.push_back(s);
	}

	writer.pushStates(states);

	QThread::msleep(500);

	const std::vector<DiscretesLogRecord> rows = readAllRows(DiscretesLogWriter::databaseName());

	EXPECT_EQ(rows.size(), CONST_STATES + VAR_STATES);
	ASSERT_EQ(rows.size(), states.size());

	for(size_t i = 0; i < rows.size(); i++)
	{
		const DiscretesLogRecord& r = rows[i];
		const SimpleAppSignalState& s = states[i];

		EXPECT_EQ(r.signalHash, s.hash);
		EXPECT_EQ(r.plantTime, s.plantTime());
		EXPECT_EQ(r.systemTime, s.systemTime());
		EXPECT_EQ(r.localTime, s.localTime());
		EXPECT_EQ(r.value, s.value);
		EXPECT_EQ(r.flags, s.flags.all);
	}

	writer.stop();
}

TEST(DiscretesLogWriter, AckLog)
{
	DiscretesLogWriter writer;

	writer.start("UTestProjC", "UTestEqC", 2, logger);

	writer.clearLog();

	std::vector<SimpleAppSignalState> v;

	for (int i = 0; i < 7; ++i)
	{
		v.push_back(makeState(randomUint64(), i % 2, randomUint32(), i * 10));
	}

	writer.pushStates(v);

	QThread::msleep(200);

	Network::AckDiscretesLogRequest ack;

	ack.set_ackuptoplanttime(20);
	writer.ackDiscretesLog(ack);			// delete plant time: 0, 10, 20

	QThread::msleep(200);

	auto rows = readAllRows(DiscretesLogWriter::databaseName());

	EXPECT_EQ(rows.size(), 4);				// left plant time is: 30, 40, 50, 60

	ack.set_ackuptoplanttime(100);
	writer.ackDiscretesLog(ack);			// delete all

	QThread::msleep(200);

	rows = readAllRows(DiscretesLogWriter::databaseName());

	EXPECT_EQ(rows.size(), 0);

	writer.stop();
}

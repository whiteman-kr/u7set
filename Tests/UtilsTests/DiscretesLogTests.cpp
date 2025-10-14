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

// ------------------------ DiscretesLogWriter tests ------------------------

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

static void makeStates(qint64 nowTime, int statesCount, std::vector<SimpleAppSignalState>& states)
{
	states.clear();
	states.reserve(statesCount);

	for(int i = 0; i < statesCount;  i++)
	{
		SimpleAppSignalState s = makeState(randomUint64(), i % 2, randomUint32(),
										   nowTime + 3600 * 1000 + i * 5,
										   nowTime + i * 5,
										   nowTime + 3600 * 1000 + i * 5 + i % 4);
		states.push_back(s);
	}
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

// ------------------------ DiscretesLogReader tests ------------------------

void insertRow(QSqlDatabase& db,
	qint64 recordTime,
	qint64 plantTime,
	qint64 systemTime,
	qint64 localTime,
	const QString& hashHex,
	int value,
	quint32 flags,
	bool acknowledged = false,
	qint64 ackTime = 0,
	const QString& ackSource = {},
	const QString& ackUser = {})
{
	QSqlQuery q(db);

	const char* sql =
		"INSERT INTO DiscretesLog "
		"(recordTime, plantTime, systemTime, localTime, hash, value, flags, "
		" acknowledged, ackTime, ackSource, ackUser) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	ASSERT_TRUE(q.prepare(sql))
		<< q.lastError().text().toStdString();

	q.bindValue(0, recordTime);
	q.bindValue(1, plantTime);
	q.bindValue(2, systemTime);
	q.bindValue(3, localTime);
	q.bindValue(4, hashHex);
	q.bindValue(5, value);
	q.bindValue(6, static_cast<qulonglong>(flags));
	q.bindValue(7, acknowledged ? 1 : 0);
	q.bindValue(8, ackTime == 0 ? QVariant() : QVariant(ackTime));
	q.bindValue(9, ackSource.isEmpty() ? QVariant() : QVariant(ackSource));
	q.bindValue(10, ackUser.isEmpty() ? QVariant() : QVariant(ackUser));

	ASSERT_TRUE(q.exec())
		<< q.lastError().text().toStdString();
}

qint64 countRows(QSqlDatabase& db)
{
	QSqlQuery q(db);

	EXPECT_TRUE(q.exec("SELECT COUNT(*) FROM DiscretesLog"))
		<< q.lastError().text().toStdString();

	EXPECT_TRUE(q.next());

	return q.value(0).toLongLong();
}

// ---------- fixture ----------

class DiscretesLogReaderFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// создаём валидную БД через writer (он создаёт файл и таблицы)

		writer.start("TProject", "TEq1", 3, logger);

		QThread::msleep(100);

		writer.clearLog();

		dbPath = DiscretesLogWriter::databaseName();

		ASSERT_FALSE(dbPath.isEmpty());

		// своё подключение для прямых вставок в тесте
		connName = "GTestConn_" + QUuid::createUuid().toString(QUuid::Id128);

		db = QSqlDatabase::addDatabase("QSQLITE", connName);
		db.setDatabaseName(dbPath);

		ASSERT_TRUE(db.open())
			<< db.lastError().text().toStdString();

		QSqlQuery q(db);

		ASSERT_TRUE(q.exec("DELETE FROM DiscretesLog"))
			<< q.lastError().text().toStdString();
	}

	void TearDown() override
	{
		writer.stop();

		if (db.isValid())
		{
			const auto cn = db.connectionName();

			if (db.isOpen())
			{
				db.close();
			}

			db = QSqlDatabase();

			QSqlDatabase::removeDatabase(cn);
		}

		writer.deleteDbFiles();
	}

	DiscretesLogWriter writer;
	QString dbPath;
	QString connName;
	QSqlDatabase db;
};

// ---------- tests ----------

TEST_F(DiscretesLogReaderFixture, initialFetch_batchesAndPending)
{
	std::vector<SimpleAppSignalState> states;

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	makeStates(now, 11000, states);

	writer.pushStates(states);

	writer.waitWhileLogQueueIsEmpty();

	EXPECT_EQ(countRows(db), 11000);

	auto reader = std::make_shared<DiscretesLogReader>(logger);

	writer.registerLogReader(reader);

	//

	Network::GetDiscretesLogReply r1;

	reader->getDiscretesLog(&r1);

	// first sample: maximum 5,000 + another 5,000 in the queue
	//
	EXPECT_EQ(r1.discreteslogrecord_size(), 5000);

	EXPECT_EQ(r1.pendingrecordscount(), 5000);

	EXPECT_TRUE(r1.has_logfirstrecordid());

	EXPECT_EQ(r1.logfirstrecordid(), 1);

	for (int i = 0; i < std::min(10, r1.discreteslogrecord_size()); ++i)
	{
		double v = r1.discreteslogrecord(i).value();

		EXPECT_TRUE(v == 0.0 || v == 1.0);
	}

	Network::GetDiscretesLogReply r2;

	reader->getDiscretesLog(&r2);

	// remove the remaining 5000 from the internal queue
	//
	EXPECT_EQ(r2.discreteslogrecord_size(), 5000);

	EXPECT_EQ(r2.pendingrecordscount(), 0);

	//

	now = QDateTime::currentMSecsSinceEpoch();

	makeStates(now, 6789, states);

	writer.pushStates(states);

	writer.waitWhileLogQueueIsEmpty();

	QThread::msleep(50);

	Network::GetDiscretesLogReply r3;

	reader->getDiscretesLog(&r3);

	EXPECT_EQ(r3.discreteslogrecord_size(), 5000);

	EXPECT_EQ(r3.pendingrecordscount(), 6789 - 5000);

	//

	Network::GetDiscretesLogReply r4;

	reader->getDiscretesLog(&r4);

	EXPECT_EQ(r4.discreteslogrecord_size(), 6789 - 5000);

	EXPECT_EQ(r4.pendingrecordscount(), 0);

	writer.unregisterLogReader(reader);
}

TEST_F(DiscretesLogReaderFixture, incrementalFetch_afterLastId)
{
	auto reader = std::make_shared<DiscretesLogReader>(logger);

	// прогреваем reader, чтобы он запросил "последние N"
	Network::GetDiscretesLogReply warmup;

	reader->getDiscretesLog(&warmup);

	EXPECT_TRUE(warmup.logisworkable());

	const qint64 t0 = QDateTime::currentMSecsSinceEpoch();

	for (int i = 0; i < 1200; ++i)
	{
		insertRow(
			db,
			t0,
			t0 + i,
			t0 + i,
			t0 + i,
			DiscretesLog::hashToHex(static_cast<Hash>(0x200000 + i)),
			(i % 2),
			static_cast<quint32>(i)
			);
	}

	// новых записей немного; уведомляем, что лог не менял границы (без truncate)
	reader->setLogChanged(false);

	Network::GetDiscretesLogReply r;

	reader->getDiscretesLog(&r);

	EXPECT_EQ(r.discreteslogrecord_size(), 1200);

	EXPECT_EQ(r.pendingrecordscount(), 0);
}

TEST_F(DiscretesLogReaderFixture, emptyTable_firstRecordIdFromSeq)
{
	qint64 seqBefore = 0;

	{
		QSqlQuery q(db);

		ASSERT_TRUE(q.exec("SELECT IFNULL((SELECT seq FROM sqlite_sequence WHERE name='DiscretesLog'), 0)"))
			<< q.lastError().text().toStdString();

		ASSERT_TRUE(q.next());

		seqBefore = q.value(0).toLongLong();
	}

	auto reader = std::make_shared<DiscretesLogReader>(logger);

	Network::GetDiscretesLogReply r;

	reader->getDiscretesLog(&r);

	EXPECT_EQ(r.discreteslogrecord_size(), 0);

	EXPECT_TRUE(r.has_logfirstrecordid());

	EXPECT_EQ(r.logfirstrecordid(), seqBefore + 1);
}

TEST_F(DiscretesLogReaderFixture, truncate_recalculateFirstId)
{
	const qint64 now = QDateTime::currentMSecsSinceEpoch();

	for (int i = 0; i < 10; ++i)
	{
		insertRow(
			db,
			now,
			now + i,
			now + i,
			now + i,
			DiscretesLog::hashToHex(static_cast<Hash>(0x3000 + i)),
			(i % 2),
			static_cast<quint32>(i)
			);
	}

	EXPECT_EQ(countRows(db), 10);

	auto reader = std::make_shared<DiscretesLogReader>(logger);

	Network::GetDiscretesLogReply r0;

	reader->getDiscretesLog(&r0);

	EXPECT_GT(r0.discreteslogrecord_size(), 0);

	{
		QSqlQuery q(db);

		ASSERT_TRUE(q.prepare("DELETE FROM DiscretesLog WHERE plantTime <= ?"))
			<< q.lastError().text().toStdString();

		q.bindValue(0, now + 4);

		ASSERT_TRUE(q.exec())
			<< q.lastError().text().toStdString();
	}

	// сообщаем, что лог "усох" (truncate)
	reader->setLogChanged(true);

	Network::GetDiscretesLogReply r1;

	reader->getDiscretesLog(&r1);

	EXPECT_TRUE(r1.has_logfirstrecordid());

	// после частичного удаления таблица не пустая
	EXPECT_GT(countRows(db), 0);
}

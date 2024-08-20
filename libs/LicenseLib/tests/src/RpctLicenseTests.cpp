#include <LicenseLib/RpctLicense.h>

#include <QTemporaryFile>

// using ::testing::_;
// using ::testing::AtLeast;
// using ::testing::An;

TEST(RpctLicenseTests, toFromRawData)
{
	using namespace LicenseLib;

	RpctLicense license;
	license.setUuid(QUuid::createUuid());
	license.setOrganization("Radiy");
	license.setFirstName("John");
	license.setLastName("Doe");
	license.setContactInfo("john.doe@radiy.com");
	license.setStartDate(QDate::currentDate());
	license.setEndDate(QDate::fromJulianDay(license.startDate().toJulianDay() + 12));
	license.setNotes("Some notes");

	QFile filePrivateKey{":/license/TestPrivateKey.pem"};
	ASSERT_TRUE(filePrivateKey.open(QIODevice::ReadOnly));
	auto privateKeyData = filePrivateKey.readAll();

	QTemporaryFile privateKeyFile{};
	ASSERT_TRUE(privateKeyFile.open());
	privateKeyFile.write(privateKeyData);
	privateKeyFile.flush();

	QString errorMessage;
	QByteArray data = license.toRawData(privateKeyFile.fileName(), "radiy", &errorMessage);

	ASSERT_FALSE(data.isEmpty());
	ASSERT_EQ(errorMessage, "");

	errorMessage.clear();
	RpctLicense license2 = RpctLicense::fromRawData(data, QString{":/license/TestPublicKey.pem"}, &errorMessage);
	ASSERT_EQ(errorMessage, "");

	EXPECT_EQ(license2.uuid(), license.uuid());
	EXPECT_EQ(license2.organization(), "Radiy");
	EXPECT_EQ(license2.firstName(), "John");
	EXPECT_EQ(license2.lastName(), "Doe");
	EXPECT_EQ(license2.contactInfo(), "john.doe@radiy.com");
	EXPECT_EQ(license2.startDate(), license.startDate());
	EXPECT_EQ(license2.endDate(), license.endDate());
	EXPECT_EQ(license2.notes(), "Some notes");

	return;
}

TEST(RpctLicenseTests, toFromRawDataCurruptedData)
{
	using namespace LicenseLib;

	RpctLicense license;
	license.setUuid(QUuid::createUuid());
	license.setOrganization("Radiy");
	license.setFirstName("John");
	license.setLastName("Doe");
	license.setContactInfo("john.doe@radiy.com");
	license.setStartDate(QDate::currentDate());
	license.setEndDate(QDate::fromJulianDay(license.startDate().toJulianDay() + 12));
	license.setNotes("Some notes");

	QFile filePrivateKey{":/license/TestPrivateKey.pem"};
	ASSERT_TRUE(filePrivateKey.open(QIODevice::ReadOnly));
	auto privateKeyData = filePrivateKey.readAll();

	QTemporaryFile privateKeyFile{};
	ASSERT_TRUE(privateKeyFile.open());
	privateKeyFile.write(privateKeyData);
	privateKeyFile.flush();

	QString errorMessage;
	QByteArray data = license.toRawData(privateKeyFile.fileName(), "radiy", &errorMessage);

	ASSERT_FALSE(data.isEmpty());
	ASSERT_EQ(errorMessage, "");

	// Corrupt data
	//
	data[100] = data[100] ^ 55;

	errorMessage.clear();
	RpctLicense license2 = RpctLicense::fromRawData(data, QString{":/license/TestPublicKey.pem"}, &errorMessage);
	ASSERT_FALSE(errorMessage.isEmpty());

	EXPECT_EQ(license2.uuid(), QUuid{});

	return;
}

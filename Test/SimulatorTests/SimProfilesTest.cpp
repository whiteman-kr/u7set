#include "SimProfilesTest.h"
#include <QtTest>

#include "../../HardwareLib/DeviceObject.h"

SimProfilesTest::SimProfilesTest()
{
	m_profiles = std::make_unique<Sim::Profiles>();
	return;
}

void SimProfilesTest::initTestCase()
{
	// Load Profiles

	QFile simProfilesFile(m_fileName);
	if (simProfilesFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QFAIL(simProfilesFile.errorString().toStdString().data());
	}

	QString errorMessage;
	bool ok = m_profiles->load(simProfilesFile.readAll(), &errorMessage);
	if (ok == false)
	{
		QFAIL(errorMessage.toStdString().data());
	}

	QVERIFY(m_profiles->isEmpty() == false);

	return;
}

void SimProfilesTest::cleanupTestCase()
{
	// Delete all profiles

	m_profiles->clear();
	QVERIFY(m_profiles->isEmpty() == false);

	// Check that Default profile still exists

	QVERIFY(m_profiles->hasProfile("Default") == true);

	return;
}

void SimProfilesTest::SimProfilesTest::init()
{
}

void SimProfilesTest::SimProfilesTest::cleanup()
{
}

// Profile Storage Test

void SimProfilesTest::profileAccessTest()
{
	QStringList profilesList = m_profiles->profiles();
	QCOMPARE(profilesList.size(), 2);

	QVERIFY(m_profiles->hasProfile("Default") == true);
	QVERIFY(m_profiles->hasProfile("TestProfile") == true);

	// Get profile by name

	const Sim::Profile& profile = m_profiles->profile("TestProfile");
	QCOMPARE(profile.profileName, "TestProfile");

	// Wrong profile name

	const Sim::Profile& wrongProfile = m_profiles->profile("WrongProfile");
	QVERIFY(wrongProfile.profileName.isEmpty() == true);

	return;
}

// Profile Contents Test

void SimProfilesTest::profileContentsTest()
{
	const Sim::Profile& profile = m_profiles->profile("TestProfile");
	QCOMPARE(profile.profileName, "TestProfile");

	// Check profiles EquipmentID

	QStringList equipment = profile.equipment();
	QCOMPARE(equipment.size(), 1);
	QCOMPARE(equipment.at(0), "SYSTEMID_WS00_ARCHS");

	// Check profile properties

	const Sim::ProfileProperties& properties = profile.properties("SYSTEMID_WS00_ARCHS");
	QCOMPARE(properties.properties.size(), 3);

	QVERIFY(properties.properties.find("AppDataReceivingIP") != properties.properties.end());
	QVERIFY(properties.properties.find("AppDataReceivingPort") != properties.properties.end());
	QVERIFY(properties.properties.find("ArchiveLocation") != properties.properties.end());

	return;
}

// Apply/Restore Properties Test

void SimProfilesTest::applyRestoreTest()
{
	Sim::Profile profile = m_profiles->profile("TestProfile");
	QCOMPARE(profile.profileName, "TestProfile");

	const Sim::ProfileProperties& properties = profile.properties("SYSTEMID_WS00_ARCHS");
	QCOMPARE(properties.properties.size(), 3);

	// Check for applying to wrong EquipmentID

	QString errorMessage;

	std::shared_ptr<Hardware::Software> unknownObject = std::make_shared<Hardware::Software>();

	bool ok = profile.applyToObject("SYSTEMID_WS00_UNKNOWN", unknownObject, &errorMessage);
	QVERIFY(ok == false);

	// Check for applying to object with non-existing properties

	std::shared_ptr<Hardware::Software> tunsObject = std::make_shared<Hardware::Software>();
	tunsObject->setEquipmentIdTemplate("SYSTEMID_WS00_TUNS");
	tunsObject->addProperty("TuningServiceIP", "Connection", true, "127.0.0.1");
	tunsObject->addProperty("TuningServicePort", "Connection", true, 13343);

	ok = profile.applyToObject("SYSTEMID_WS00_TUNS", tunsObject, &errorMessage);
	QVERIFY(ok == false);

	// Apply properties to an object

	std::shared_ptr<Hardware::Software> archsObject = std::make_shared<Hardware::Software>();
	archsObject->setEquipmentIdTemplate("SYSTEMID_WS00_ARCHS");
	archsObject->addProperty("AppDataReceivingIP", "Connection", true, "127.0.0.1");
	archsObject->addProperty("AppDataReceivingPort", "Connection", true, 13342);
	archsObject->addProperty("ArchiveLocation", "ArchiveSettings", true, QString());

	ok = profile.applyToObject("SYSTEMID_WS00_ARCHS", archsObject, &errorMessage);
	if (ok == false)
	{
		QFAIL(errorMessage.toStdString().data());
	}

	// Check that properties have overridden values

	QCOMPARE(archsObject->propertyValue("AppDataReceivingIP").toString(), "192.168.12.254");
	QCOMPARE(archsObject->propertyValue("AppDataReceivingPort").toInt(), 2500);
	QCOMPARE(archsObject->propertyValue("ArchiveLocation").toString(), "d:\\arch2");

	// Restore original values

	ok = profile.restoreObjects();
	if (ok == false)
	{
		QFAIL("");
	}

	QCOMPARE(archsObject->propertyValue("AppDataReceivingIP").toString(), "127.0.0.1");
	QCOMPARE(archsObject->propertyValue("AppDataReceivingPort").toInt(), 13342);
	QCOMPARE(archsObject->propertyValue("ArchiveLocation").toString(), QString());

	return;
}

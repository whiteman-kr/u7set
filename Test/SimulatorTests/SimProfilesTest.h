#pragma once

#include <QObject>
#include "../../Simulator/SimProfiles.h"

class SimProfilesTest : public QObject
{
	Q_OBJECT
public:
	SimProfilesTest();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void init();
	void cleanup();

	void profileAccessTest();
	void profileContentsTest();
	void applyRestoreTest();

private:
	const QString m_fileName = ":/SimProfiles.txt";

	std::unique_ptr<Sim::Profiles> m_profiles;

};


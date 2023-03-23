#include "../ClientLib/TuningUserManager.h"

class TuningUserManagerTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{

		return;
	}

	virtual void TearDown()
	{
	}

};

class TuningUserManagerTest : public ClientLib::TuningUserManager
{
public:
	TuningUserManagerTest() = default;

	bool askForPassword(QString* userName, QString* password, QWidget* parent) override
	{
		Q_UNUSED(parent);
		*userName = "user";
		*password = "P2ssw0rd";
		return true;
	}

	bool checkPassword(const QString& userName, const QString& password) override
	{
#ifdef Q_OS_LINUX
		int todo_check_password_on_linux;
		Q_UNUSED(userName);
		Q_UNUSED(password);
		return password == "P2ssw0rd";
#else
		return TuningUserManager::checkPassword(userName, password);
#endif
	}
};


TEST_F(TuningUserManagerTests, loginTests)
{
	TuningUserManagerTest tum{};

	bool tuningLogin = true;
	QStringList tuningUserAccounts{"user"};
	int tuningSessionTimeout = 10;

	tum.setConfiguration(tuningLogin, tuningUserAccounts, false/*loginPerOperation*/, tuningSessionTimeout);

	// Check properties

	EXPECT_EQ(tum.tuningLogin(), tuningLogin);
	EXPECT_EQ(tum.tuningUserAccounts(), tuningUserAccounts);
	EXPECT_FALSE(tum.loginPerOperation());
	EXPECT_EQ(tum.tuningSessionTimeout(), tuningSessionTimeout);

	// Login in saving login status mode

	EXPECT_TRUE(tum.login(nullptr));
	EXPECT_TRUE(tum.isLoggedIn());
	EXPECT_TRUE(tum.loggedInUser() == "user");

	EXPECT_TRUE(tum.logoutPendingSeconds() == tuningSessionTimeout);

	QThread::msleep(1000);

	EXPECT_TRUE(tum.logoutPendingSeconds() < tuningSessionTimeout);

	tum.reLogin(nullptr);

	EXPECT_TRUE(tum.logoutPendingSeconds() == tuningSessionTimeout);

	tum.logout();

	EXPECT_FALSE(tum.isLoggedIn());

	// Login in login-per-operation mode

	tum.setConfiguration(tuningLogin, tuningUserAccounts, true/*loginPerOperation*/, tuningSessionTimeout);

	EXPECT_TRUE(tum.loginPerOperation());

	EXPECT_TRUE(tum.login(nullptr));
	EXPECT_FALSE(tum.isLoggedIn());
	tum.reLogin(nullptr);
	EXPECT_FALSE(tum.isLoggedIn());
	tum.logout();


	return;
}

TEST_F(TuningUserManagerTests, checkPasswordsTest)
{
	TuningUserManagerTest tum{};

	EXPECT_TRUE(tum.checkPassword("user", "P2ssw0rd"));
	EXPECT_FALSE(tum.checkPassword("user", "wrongpassword"));

	return;
}

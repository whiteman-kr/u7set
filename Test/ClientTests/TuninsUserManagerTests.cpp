#include <ClientLib/TuningUserManager.h>

// ----------------
//
//	Warning! To complete these tests successfully, add user account with "user" username and "P2ssw0rd" password!
//
// ---------------

class TuningUserManagerTests : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{

		std::cout << "Warning: To complete TuningUserManagerTests successfully, add an account with \"user\" username and \"P2ssw0rd\" password.\n";
	}

	virtual void SetUp()
	{
		return;
	}

	virtual void TearDown()
	{
	}

};

TEST_F(TuningUserManagerTests, loginTests)
{
	ClientLib::TuningUserManager tum{};

	bool tuningLogin = true;
	QStringList tuningUserAccounts{"user"};
	int tuningSessionTimeout = 10;

	tum.setConfiguration(tuningLogin, tuningUserAccounts, false /*loginPerOperation*/, tuningSessionTimeout, {});

	// Check properties

	EXPECT_EQ(tum.enabled(), tuningLogin);
	EXPECT_EQ(tum.tuningUserAccounts(), tuningUserAccounts);
	EXPECT_FALSE(tum.loginPerOperation());
	EXPECT_EQ(tum.tuningSessionTimeout(), tuningSessionTimeout);

	// Login in saving login status mode

	EXPECT_TRUE(tum.login("user", "P2ssw0rd"));
	EXPECT_TRUE(tum.isLoggedIn());
	EXPECT_TRUE(tum.userName() == "user");

	EXPECT_TRUE(tum.logoutPendingSeconds() == tuningSessionTimeout);

	QThread::msleep(1000);

	EXPECT_TRUE(tum.logoutPendingSeconds() < tuningSessionTimeout);

	tum.reLogin(nullptr);

	EXPECT_TRUE(tum.logoutPendingSeconds() == tuningSessionTimeout);

	tum.logout();

	EXPECT_FALSE(tum.isLoggedIn());

	// Login in login-per-operation mode

	tum.setConfiguration(tuningLogin, tuningUserAccounts, true /*loginPerOperation*/, tuningSessionTimeout, {});

	EXPECT_TRUE(tum.loginPerOperation());

	EXPECT_TRUE(tum.login("user", "P2ssw0rd"));
	EXPECT_FALSE(tum.isLoggedIn());
	tum.reLogin(nullptr);
	EXPECT_FALSE(tum.isLoggedIn());
	tum.logout();


	return;
}

TEST_F(TuningUserManagerTests, checkPasswordsTest)
{
	EXPECT_TRUE(ClientLib::TuningUserManager::checkPassword("user", "P2ssw0rd"));
	EXPECT_FALSE(ClientLib::TuningUserManager::checkPassword("user", "wrongpassword"));

	return;
}

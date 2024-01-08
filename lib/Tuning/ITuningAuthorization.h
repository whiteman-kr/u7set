#pragma once

class ITuningAuthorization
{
public:
	virtual ~ITuningAuthorization() = default;

	virtual bool tuningLogin() const = 0;
	virtual bool login(QWidget* parent) = 0;

	virtual bool isLoggedIn() const = 0;
	virtual QString loggedInUser() const = 0;
};

class TuningAuthorizationStub : public ITuningAuthorization
{
	bool tuningLogin() const override { return false; }
	virtual bool login(QWidget* /*parent*/) override { return true; }

	virtual bool isLoggedIn() const override { return true; }
	virtual QString loggedInUser() const override { return {}; }
};



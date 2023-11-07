#pragma once

class ITuningAuthorization
{
public:
	virtual ~ITuningAuthorization() = default;

	virtual bool isLoggedIn() const = 0;
	virtual bool checkTuningAccess(QWidget* parent) = 0;
};

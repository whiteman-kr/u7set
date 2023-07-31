#pragma once

class ITuningAuthorization
{
public:
	virtual ~ITuningAuthorization() = default;

	virtual bool checkTuningAccess(QWidget* parent) = 0;
};

#pragma once

class ITuningAuthorization
{
public:
	virtual ~ITuningAuthorization() = default;

	virtual bool enabled() const = 0;
	
	virtual bool login(QWidget* parent) = 0;
	virtual bool isLoggedIn() const = 0;
	
	virtual QString userName() const = 0;
	virtual QStringList userTags() const = 0;
};

class TuningAuthorizationStub : public ITuningAuthorization
{
	bool enabled() const  override { return false; }

	bool login(QWidget* /*parent*/) override { return true; }
	bool isLoggedIn() const override { return true; }

	QString userName() const override { return {}; }
	QStringList userTags() const override { return {}; }
};



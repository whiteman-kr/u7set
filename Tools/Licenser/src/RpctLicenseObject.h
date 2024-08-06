#pragma once

#include <CommonLib/PropertyObject.h>
#include <LicenseLib/RpctLicense.h>

class RpctLicenseObject : public PropertyObject, public LicenseLib::RpctLicense
{
	Q_OBJECT

public:
	explicit RpctLicenseObject(QObject* parent = nullptr);

public:
	void setLicense(const LicenseLib::RpctLicense& license);
};
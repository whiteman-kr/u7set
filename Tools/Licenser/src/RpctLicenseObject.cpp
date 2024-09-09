#include "RpctLicenseObject.h"

RpctLicenseObject::RpctLicenseObject(QObject* parent) :
	PropertyObject{parent},
	LicenseLib::RpctLicense{}
{
	Property* p = ADD_PROPERTY_GETTER(QUuid, "Uuid", true, LicenseLib::RpctLicense::uuid);

#if 0
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, LicenseLib::RpctLicense::caption, LicenseLib::RpctLicense::setCaption);
#endif

	p = ADD_PROPERTY_GETTER_SETTER(QString, "Organization", true, LicenseLib::RpctLicense::organization, LicenseLib::RpctLicense::setOrganization);
	p->setCategory("Customer");
	p->setViewOrder(100);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "First Name", true, LicenseLib::RpctLicense::firstName, LicenseLib::RpctLicense::setFirstName);
	p->setCategory("Customer");
	p->setViewOrder(101);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "Last Name", true, LicenseLib::RpctLicense::lastName, LicenseLib::RpctLicense::setLastName);
	p->setCategory("Customer");
	p->setViewOrder(102);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "Contact Info", true, LicenseLib::RpctLicense::contactInfo, LicenseLib::RpctLicense::setContactInfo);
	p->setCategory("Customer");
	p->setViewOrder(103);

	p = ADD_PROPERTY_GETTER_SETTER(QDate, "Start Date", true, LicenseLib::RpctLicense::startDate, LicenseLib::RpctLicense::setStartDate);
	p->setCategory("License");
	p->setViewOrder(200);

	p = ADD_PROPERTY_GETTER_SETTER(QDate, "End Date", true, LicenseLib::RpctLicense::endDate, LicenseLib::RpctLicense::setEndDate);
	p->setCategory("License");
	p->setViewOrder(201);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "Notes", true, LicenseLib::RpctLicense::notes, LicenseLib::RpctLicense::setNotes);
	p->setCategory("Other");
	p->setViewOrder(300);

	p = ADD_PROPERTY_GETTER(QString, "LicenserVersion", true, LicenseLib::RpctLicense::licenserVersion);
	p->setCategory("Other");
	p->setViewOrder(301);

	p = ADD_PROPERTY_GETTER(QDate, "IssueDate", true, LicenseLib::RpctLicense::issueDate);
	p->setCategory("Other");
	p->setViewOrder(302);

	// WorkplaceLicense 
	//
#if 0
	p = ADD_PROPERTY_GETTER_SETTER(LicenseLib::WorkplaceCheckType, "WorkplaceCheckType", true, LicenseLib::RpctLicense::workplaceCheckType, LicenseLib::RpctLicense::setWorkplaceCheckType);
	p->setCategory("Workplace");
	p->setViewOrder(400);
	p->setEssential(true);
#endif

	p = ADD_PROPERTY_GETTER_SETTER(QString, "WorkplaceId", true, LicenseLib::RpctLicense::workplaceId, LicenseLib::RpctLicense::setWorkplaceId);
	p->setCategory("Workplace");
	p->setViewOrder(401);
	p->setEssential(true);

	// ModuleConfiguratorLicense
	//
	p = ADD_PROPERTY_GETTER_SETTER(bool, "AllowedModuleConfigurator", true, LicenseLib::RpctLicense::allowedModuleConfigurator, LicenseLib::RpctLicense::setAllowedModuleConfigurator);
	p->setCategory("Module Configurator (mconf/u7)");
	p->setViewOrder(500);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(bool, "McLimitModuleUartUuids", true, LicenseLib::RpctLicense::mcLimitModuleUartUuids, LicenseLib::RpctLicense::setMcLimitModuleUartUuids);
	p->setCategory("Module Configurator (mconf/u7)");
	p->setDescription("Limit the modules that can be configured. The limitation is enforced via UartUuid. The module has three configuration UARTs (configuration, applogic, tuning). All three UartUuids must be added to McAllowedModuleUartUuids.");
	p->setViewOrder(501);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "McAllowedModuleUartUuids", true, LicenseLib::RpctLicense::mcAllowedModuleUartUuids, LicenseLib::RpctLicense::setMcAllowedModuleUartUuids);
	p->setCategory("Module Configurator (mconf/u7)");
	p->setDescription("If McLimitModuleUartUuids is true, then this property is a list of modules allowed to be configured. Note that all three UartUuids must be added to this list. Data should be separated by spaces or new lines.\nUuids with wrong format will be ignored, this fact allows to add kind of comments.");
	p->setViewOrder(502);

	return;
}

void RpctLicenseObject::setLicense(const LicenseLib::RpctLicense& license)
{
	LicenseLib::RpctLicense::operator=(license);
	return;
}

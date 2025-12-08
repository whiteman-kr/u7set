#include "RpctLicenseObject.h"

#include <LicenseLib/RpctValidator.h>

RpctLicenseObject::RpctLicenseObject(QObject* parent) :
	PropertyObject{parent},
	LicenseLib::RpctLicense{}
{
	QString categoryCommon = "1 Common";
	QString categoryCustomer = "2 Customer";
	QString categoryLicense = "3 License";
	QString categoryU7 = "4 u7";
	QString categorySimulator = "5 Simulator";
	QString categoryModuleConfigurator = "6 Module Configurator (mconf/u7)";
	QString categoryOther = "9 Other";

	// --
	Property* p = ADD_PROPERTY_GETTER_SETTER(bool, "Revoked", true, LicenseLib::RpctLicense::revoked, LicenseLib::RpctLicense::setRevoked);
	p->setCategory(categoryCommon);
	p->setViewOrder(1);

	p = ADD_PROPERTY_GETTER(QUuid, "Uuid", true, LicenseLib::RpctLicense::uuid);
	p->setCategory(categoryCommon);
	p->setViewOrder(2);

#if 0
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, LicenseLib::RpctLicense::caption, LicenseLib::RpctLicense::setCaption);
#endif

	p = ADD_PROPERTY_GETTER_SETTER(QString,
								   "Organization",
								   true,
								   LicenseLib::RpctLicense::organization,
								   LicenseLib::RpctLicense::setOrganization);
	p->setCategory(categoryCustomer);
	p->setViewOrder(100);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "First Name", true, LicenseLib::RpctLicense::firstName, LicenseLib::RpctLicense::setFirstName);
	p->setCategory(categoryCustomer);
	p->setViewOrder(101);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString, "Last Name", true, LicenseLib::RpctLicense::lastName, LicenseLib::RpctLicense::setLastName);
	p->setCategory(categoryCustomer);
	p->setViewOrder(102);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(QString,
								   "Contact Info",
								   true,
								   LicenseLib::RpctLicense::contactInfo,
								   LicenseLib::RpctLicense::setContactInfo);
	p->setCategory(categoryCustomer);
	p->setViewOrder(103);

	// Workplace
	//
#if 0
	p = ADD_PROPERTY_GETTER_SETTER(LicenseLib::WorkplaceCheckType, "WorkplaceCheckType", true, LicenseLib::RpctLicense::workplaceCheckType, LicenseLib::RpctLicense::setWorkplaceCheckType);
	p->setCategory(categoryCustomer);
	p->setViewOrder(198);
	p->setEssential(true);
#endif

	p = ADD_PROPERTY_GETTER_SETTER(QString,
								   "WorkplaceId",
								   true,
								   LicenseLib::RpctLicense::workplaceId,
								   LicenseLib::RpctLicense::setWorkplaceId);
	p->setCategory(categoryCustomer);
	p->setViewOrder(199);
	p->setEssential(true);

	// --
	//
	p = ADD_PROPERTY_GETTER_SETTER(QDate, "Start Date", true, LicenseLib::RpctLicense::startDate, LicenseLib::RpctLicense::setStartDate);
	p->setCategory(categoryLicense);
	p->setViewOrder(200);

	p = ADD_PROPERTY_GETTER_SETTER(QDate, "End Date", true, LicenseLib::RpctLicense::endDate, LicenseLib::RpctLicense::setEndDate);
	p->setCategory(categoryLicense);
	p->setViewOrder(201);
	p->setEssential(true);

	// u7
	//
	p = ADD_PROPERTY_GETTER_SETTER(bool, "u7 Allowed", true, LicenseLib::RpctLicense::allowedU7, LicenseLib::RpctLicense::setAllowedU7);
	p->setCategory(categoryU7);
	p->setDescription("Allow the u7 (RPCT) application.");
	p->setViewOrder(401);
	p->setEssential(true);

	// Simulator application
	//
	p = ADD_PROPERTY_GETTER_SETTER(bool,
								   "Simulator App Allowed",
								   true,
								   LicenseLib::RpctLicense::allowedSimulatorApp,
								   LicenseLib::RpctLicense::setSimulatorApp);

	p->setCategory(categorySimulator);
	p->setDescription("Allow the simulator application.");
	p->setViewOrder(501);
	p->setEssential(true);

	// ModuleConfiguratorLicense
	//
	p = ADD_PROPERTY_GETTER_SETTER(bool,
								   "AllowedModuleConfigurator",
								   true,
								   LicenseLib::RpctLicense::allowedModuleConfigurator,
								   LicenseLib::RpctLicense::setAllowedModuleConfigurator);
	p->setCategory(categoryModuleConfigurator);
	p->setViewOrder(600);
	p->setEssential(true);

	p = ADD_PROPERTY_GETTER_SETTER(bool,
								   "WriteServiceEeprom",
								   true,
								   LicenseLib::RpctLicense::mcWriteServiceEeprom,
								   LicenseLib::RpctLicense::setMcWriteServiceEeprom);
	p->setCategory(categoryModuleConfigurator);
	p->setViewOrder(601);
	p->setEssential(false);

	p = ADD_PROPERTY_GETTER_SETTER(bool,
								   "McLimitModuleUartUuids",
								   true,
								   LicenseLib::RpctLicense::mcLimitModuleUartUuids,
								   LicenseLib::RpctLicense::setMcLimitModuleUartUuids);
	p->setCategory(categoryModuleConfigurator);
	p->setDescription(
		"Limit the modules that can be configured. The limitation is enforced via UartUuid. The module has three configuration UARTs "
		"(configuration, applogic, tuning). All three UartUuids must be added to McAllowedModuleUartUuids.");
	p->setViewOrder(602);

	p = ADD_PROPERTY_GETTER_SETTER(QString,
								   "McAllowedModuleUartUuids",
								   true,
								   LicenseLib::RpctLicense::mcAllowedModuleUartUuids,
								   LicenseLib::RpctLicense::setMcAllowedModuleUartUuids);
	p->setCategory(categoryModuleConfigurator);
	p->setDescription("If McLimitModuleUartUuids is true, then this property is a list of modules allowed to be configured. Note that all "
					  "three UartUuids must be added to this list. Data should be separated by spaces or new lines.\nUuids with wrong "
					  "format will be ignored, this fact allows to add kind of comments.");
	p->setViewOrder(603);

	// Other
	//
	p = ADD_PROPERTY_GETTER_SETTER(QString, "Notes", true, LicenseLib::RpctLicense::notes, LicenseLib::RpctLicense::setNotes);
	p->setCategory(categoryOther);
	p->setViewOrder(300);

	p = ADD_PROPERTY_GETTER(QString, "LicenserVersion", true, LicenseLib::RpctLicense::licenserVersion);
	p->setCategory(categoryOther);
	p->setViewOrder(301);

	p = ADD_PROPERTY_GETTER(QDate, "IssueDate", true, LicenseLib::RpctLicense::issueDate);
	p->setCategory(categoryOther);
	p->setViewOrder(302);

	p = ADD_PROPERTY_GETTER(QString,
							"Dump",
							true,
							[this]()
							{
								return LicenseLib::RpctValidator::dumpWorkplaceId(workplaceId());
							});
	p->setCategory(categoryOther);
	p->setViewOrder(303);

	return;
}

void RpctLicenseObject::setLicense(const LicenseLib::RpctLicense& license)
{
	LicenseLib::RpctLicense::operator=(license);
	return;
}

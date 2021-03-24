#include "DeviceObjectTests.h"
#include <QtTest>

DeviceObjectTests::DeviceObjectTests(QObject* parent) :
	QObject(parent)
{
}

void DeviceObjectTests::initTestCase()
{
	//	Hardware::DeviceRoot
	//	   USB1 Hardware::DeviceSystem
	//		   USB1_RACK01 Hardware::DeviceRack
	//			   USB1_RACK01_CH01 Hardware::DeviceChassis
	//			   USB1_RACK01_CH02 Hardware::DeviceChassis
	//				   USB1_RACK01_CH02_MD00 Hardware::DeviceModule
	//				   USB1_RACK01_CH02_MD00_CTRL01 Hardware::DeviceController
	//					   USB1_RACK01_CH02_MD00_IN22 Hardware::DeviceAppSignal
	//				   USB1_RACK01_CH02_MD06 Hardware::DeviceModule
	//		   USB1_RACK02 Hardware::DeviceRack
	//

	// Root
	//
	m_root = std::make_shared<Hardware::DeviceRoot>();
	m_root->setUuid(QUuid::createUuid());
	m_root->setCaption(QStringLiteral("Root"));

	{
		// System
		//
		auto system = std::make_shared<Hardware::DeviceSystem>();
		system->setUuid(QUuid::createUuid());
		system->setCaption(QStringLiteral("USB-1"));
		system->setEquipmentIdTemplate(QStringLiteral("USB1"));
		m_root->addChild(system);

		// Rack - Rack1
		//
		{
			auto rack1 = std::make_shared<Hardware::DeviceRack>();
			rack1->setUuid(QUuid::createUuid());
			rack1->setPlace(1);
			rack1->setCaption(QStringLiteral("Rack $(Place)"));
			rack1->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_RACK$(Place)"));
			system->addChild(rack1);

			// Chassis 1
			//
			{
				auto chassis = std::make_shared<Hardware::DeviceChassis>();
				chassis->setUuid(QUuid::createUuid());
				chassis->setPlace(1);
				chassis->setCaption(QStringLiteral("Chassis $(Place)"));
				chassis->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_CH$(Place)"));
				rack1->addChild(chassis);
			}

			// Chassis 2
			//
			{
				auto chassis = std::make_shared<Hardware::DeviceChassis>();
				chassis->setUuid(QUuid::createUuid());
				chassis->setPlace(2);
				chassis->setCaption(QStringLiteral("Chassis $(Place)"));
				chassis->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_CH$(Place)"));
				rack1->addChild(chassis);

				// Module LM
				//
				{
					auto module = std::make_shared<Hardware::DeviceModule>();
					module->setUuid(QUuid::createUuid());
					module->setPlace(0);
					module->setCaption(QStringLiteral("LM"));
					module->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_MD$(Place)"));
					module->setModuleFamily(Hardware::DeviceModule::LM);
					module->setModuleVersion(1);
					chassis->addChild(module);

					{
						auto controller = std::make_shared<Hardware::DeviceController>();
						controller->setUuid(QUuid::createUuid());
						controller->setPlace(1);
						controller->setCaption(QStringLiteral("Controller"));
						controller->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_CTRL$(Place)"));
						module->addChild(controller);

						// Module AppSignal
						//
						{
							auto appSignal = std::make_shared<Hardware::DeviceAppSignal>();
							appSignal->setUuid(QUuid::createUuid());
							appSignal->setPlace(22);
							appSignal->setCaption(QStringLiteral("AppSignal $(Place)"));
							appSignal->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_IN$(Place)"));
							controller->addChild(appSignal);
						}
					}
				}

				// Module IO
				//
				{
					auto module = std::make_shared<Hardware::DeviceModule>();
					module->setUuid(QUuid::createUuid());
					module->setPlace(6);
					module->setCaption(QStringLiteral("Chassis $(Place)"));
					module->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_MD$(Place)"));
					module->setModuleFamily(Hardware::DeviceModule::AIM);
					module->setModuleVersion(1);
					chassis->addChild(module);
				}
			}
		}

		// Rack - Rack2
		//
		{
			auto rack2 = std::make_shared<Hardware::DeviceRack>();
			rack2->setUuid(QUuid::createUuid());
			rack2->setPlace(2);
			rack2->setCaption(QStringLiteral("Rack $(Place)"));
			rack2->setEquipmentIdTemplate(QStringLiteral("$(PARENT)_RACK$(Place)"));
			system->addChild(rack2);
		}
	}

//	QString s;
//	m_root->dump(true, true, &s);
//	qDebug().noquote() << s;

	return;
}

void DeviceObjectTests::cleanupTestCase()
{
	m_root.reset();
}

void DeviceObjectTests::testSharedPtr()
{
	auto d = std::make_shared<Hardware::DeviceAppSignal>();

	auto ncp = d->sharedPtr();
	const auto sp = d->sharedPtr();

	QVERIFY(ncp != nullptr);
	QVERIFY(sp != nullptr);

	return;
}

void DeviceObjectTests::testSaveLoad()
{
	// Create object, save it, restore it, check restored object
	//
	auto deviceToSave = std::make_shared<Hardware::DeviceAppSignal>();

	deviceToSave->setUuid(QUuid::createUuid());
	deviceToSave->setEquipmentIdTemplate(QStringLiteral("SOME_EQUID"));
	deviceToSave->setCaption(QStringLiteral("SignalCaption"));
	deviceToSave->setPlace(49);

	deviceToSave->setSignalType(E::SignalType::Analog);
	deviceToSave->setFunction(E::SignalFunction::Output);
	deviceToSave->setSize(48);
	deviceToSave->setValueOffset(77);
	deviceToSave->setValueBit(13);

	// Save
	//
	QByteArray buffer;
	bool ok = deviceToSave->saveToByteArray(&buffer);
	QVERIFY(ok);

	// Restore
	//
	std::shared_ptr<Hardware::DeviceObject> restoredDevice = Hardware::DeviceObject::Create(buffer);
	QVERIFY(restoredDevice);

	// Check restored
	//
	std::vector<std::shared_ptr<Property>> src = deviceToSave->properties();
	std::vector<std::shared_ptr<Property>> dst = restoredDevice->properties();

	QVERIFY(src.empty() == false);
	QVERIFY(src.size() == dst.size());

	for (const auto& sp : src)
	{
		auto dit = std::find_if(dst.begin(), dst.end(),
								[sp](const auto dp)
								{
									return sp->caption() == dp->caption();
								});

		QVERIFY(dit != dst.end());

		auto dp = *dit;
		QVERIFY(dp->value() == sp->value());
	}

	return;
}

void DeviceObjectTests::testSaveLoadTree()
{
	::Proto::Envelope e;

	bool ok = m_root->SaveObjectTree(&e);
	QVERIFY(ok);

	auto restoreRoot = Hardware::DeviceObject::Create(e);
	QVERIFY(restoreRoot);

	QString s1, s2;

	m_root->dump(true, true, &s1);
	restoreRoot->dump(true, true, &s2);

	QVERIFY(s1.isEmpty() == false);
	QVERIFY(s1 == s2);

	return;
}

void DeviceObjectTests::testExpandEquipmentId()
{
	m_root->expandEquipmentId();

	auto devices = Hardware::EquipmentSet{m_root}.devices();

	for (const auto& d : devices)
	{
		QVERIFY(d->equipmentIdTemplate() == d->equipmentId());
		QVERIFY(d->equipmentIdTemplate().contains(QLatin1String("$("), Qt::CaseInsensitive) == false);
	}

	return;
}

void DeviceObjectTests::testGetAllAppSignals()
{
	auto appSignalDevices = m_root->getAllAppSignals();
	QVERIFY(appSignalDevices.size() == 1);			// in initTestCase created 1 signal (USB1_RACK01_CH02_MD00_IN22)

	for (const auto& d : appSignalDevices)
	{
		QVERIFY(d->isAppSignal());
	}

	return;
}

void DeviceObjectTests::testGetParentController()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentController() == nullptr);

	auto d = appSignalDevice->getParentController();
	QVERIFY(d != nullptr);
	QVERIFY(d->isController() == true);

	return;
}

void DeviceObjectTests::testGetParentModule()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentModule() == nullptr);

	auto d = appSignalDevice->getParentModule();
	QVERIFY(d != nullptr);
	QVERIFY(d->isModule() == true);

	return;
}

void DeviceObjectTests::testGetParentChassis()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentChassis() == nullptr);

	auto d = appSignalDevice->getParentChassis();
	QVERIFY(d != nullptr);
	QVERIFY(d->isChassis() == true);

	return;
}

void DeviceObjectTests::testGetParentRack()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentRack() == nullptr);

	auto d = appSignalDevice->getParentRack();
	QVERIFY(d != nullptr);
	QVERIFY(d->isRack() == true);

	return;
}

void DeviceObjectTests::testGetParentSystem()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentSystem() == nullptr);

	auto d = appSignalDevice->getParentSystem();
	QVERIFY(d != nullptr);
	QVERIFY(d->isSystem() == true);

	return;
}

void DeviceObjectTests::testGetParentRoot()
{
	auto appSignalDevice = m_root->getAllAppSignals().front();

	QVERIFY(m_root->getParentRoot() == nullptr);

	auto d = appSignalDevice->getParentRoot();
	QVERIFY(d != nullptr);
	QVERIFY(d->isRoot() == true);

	return;
}

void DeviceObjectTests::testCanAddChildRoot()
{
	auto root = std::make_shared<Hardware::DeviceRoot>();

	QVERIFY(root->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(root->canAddChild(Hardware::DeviceType::System) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Rack) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Chassis) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Module) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Controller) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Workstation) == true);
	QVERIFY(root->canAddChild(Hardware::DeviceType::Software) == true);

	return;
}

void DeviceObjectTests::testCanAddChildSystem()
{
	auto system = std::make_shared<Hardware::DeviceSystem>();

	QVERIFY(system->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(system->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Rack) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Chassis) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Module) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Controller) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Workstation) == true);
	QVERIFY(system->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildRack()
{
	auto rack = std::make_shared<Hardware::DeviceRack>();

	QVERIFY(rack->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Chassis) == true);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Module) == true);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Controller) == true);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Workstation) == true);
	QVERIFY(rack->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildChassis()
{
	auto chassis = std::make_shared<Hardware::DeviceChassis>();

	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Module) == true);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Controller) == true);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Workstation) == true);
	QVERIFY(chassis->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildModule()
{
	auto module = std::make_shared<Hardware::DeviceModule>();

	QVERIFY(module->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Module) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Controller) == true);
	QVERIFY(module->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Workstation) == false);
	QVERIFY(module->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildController()
{
	auto controller = std::make_shared<Hardware::DeviceController>();

	QVERIFY(controller->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Module) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Controller) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::AppSignal) == true);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Workstation) == false);
	QVERIFY(controller->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildAppSignal()
{
	auto devAppSignal = std::make_shared<Hardware::DeviceAppSignal>();

	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Module) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Controller) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::AppSignal) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Workstation) == false);
	QVERIFY(devAppSignal->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testCanAddChildWorkstation()
{
	auto workstation = std::make_shared<Hardware::Workstation>();

	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Module) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Controller) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::AppSignal) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Workstation) == false);
	QVERIFY(workstation->canAddChild(Hardware::DeviceType::Software) == true);

	return;
}

void DeviceObjectTests::testCanAddChildSoftware()
{
	auto software = std::make_shared<Hardware::Software>();

	QVERIFY(software->canAddChild(Hardware::DeviceType::Root) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::System) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Rack) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Chassis) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Module) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Controller) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::AppSignal) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Workstation) == false);
	QVERIFY(software->canAddChild(Hardware::DeviceType::Software) == false);

	return;
}

void DeviceObjectTests::testAddChild()
{
	auto module = std::make_shared<Hardware::DeviceModule>();
	auto controller = std::make_shared<Hardware::DeviceController>();

	module->addChild(controller);

	QVERIFY(module->child(0) == controller);
	QVERIFY(controller->parent() == module);

	return;
}

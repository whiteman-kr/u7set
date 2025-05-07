#pragma once

namespace Hardware
{
	class DeviceRoot;
}

class DeviceObjectTests : public QObject
{
	Q_OBJECT

public:
	DeviceObjectTests(QObject* parent = nullptr);

private slots:
	void initTestCase();
	void cleanupTestCase();

	void testSharedPtr();
	void testSaveLoad();
	void testSaveLoadTree();
	void testExpandEquipmentId();
	void testGetAllAppSignals();

	void testGetParentController();
	void testGetParentModule();
	void testGetParentChassis();
	void testGetParentRack();
	void testGetParentSystem();
	void testGetParentRoot();

	void testCanAddChildRoot();
	void testCanAddChildSystem();
	void testCanAddChildRack();
	void testCanAddChildChassis();
	void testCanAddChildModule();
	void testCanAddChildController();
	void testCanAddChildAppSignal();
	void testCanAddChildDiagSignal();
	void testCanAddChildWorkstation();
	void testCanAddChildSoftware();

	void testAddChild();

private:
	std::shared_ptr<Hardware::DeviceRoot> m_root;
};

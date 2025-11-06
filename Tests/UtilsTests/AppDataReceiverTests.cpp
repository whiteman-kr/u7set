#include "RupFrameProducer.h"
#include "Common.h"
#include "../../AppSignalLib/AppSignal.h"
#include "../../AppDataService/AppDataReceiver.h"

/*
TEST(AppDataReceiverTests, ReceivePackets)
{
	AppDataSource* src = appDataSources.getSourceByEquipmentID("SYSTEMID_RACK01_FSCC01_MD00");

	EXPECT_NE(src, nullptr);

	Rup::Data data;
	Network::AppDataReceiveState adrs;

	AppDataReceiver receiver(appDataSrvSettings.appDataReceivingIP,
							appDataSources,
							appSignalStates,
							4,
							E::SoftwareRunMode::Normal,
							logger);

	RupFrameProducer rfp(appDataSrvSettings.appDataReceivingIP,
						 HostAddressPort("127.0.0.1", 30001));

	receiver.start();
	rfp.start();

	//

	rfp.pushRupFrame(data);

	QThread::msleep(2000);

	receiver.fillAppDataReceiveState(&adrs);

	EXPECT_EQ(adrs.rupframescount(), 1);

	//

	rfp.stop();
	receiver.quitAndWait();
} */

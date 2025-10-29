#include "RupFrameProducer.h"
#include "Common.h"
#include "../../AppSignalLib/AppSignal.h"
#include "../../AppDataService/AppDataReceiver.h"

TEST(AppDataReceiverTests, ReceivePackets)
{
	AppDataReceiver receiver(appDataSrvSettings.appDataReceivingIP,
							appDataSources,
							appSignalStates,
							4,
							E::SoftwareRunMode::Normal,
							logger);
	receiver.start();

	receiver.quitAndWait();
}

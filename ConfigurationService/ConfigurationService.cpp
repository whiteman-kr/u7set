#include <QXmlStreamReader>
#include <QMetaProperty>

#include "ConfigurationService.h"


// ConfigurationService class implementation
//

ConfigurationService::ConfigurationService(int argc, char ** argv) :
	BaseService(argc, argv, "Configuration Service", SERVICE_CONFIGURATION, new ConfigurationServiceMainFunctionWorker())
{
}


ConfigurationService::~ConfigurationService()
{
}


// DataServiceMainFunctionWorker class implementation
//

void ConfigurationServiceMainFunctionWorker::initialize()
{
	// Service Main Function initialization
	//

	m_cfgServerThread = new Tcp::ServerThread(HostAddressPort("127.0.0.1", PORT_CONFIGURATION_SERVICE_REQUEST),
											  new CfgServer(""));

	m_cfgServerThread->start();

	qDebug() << "ConfigurationServiceMainFunctionWorker initialized";
}


void ConfigurationServiceMainFunctionWorker::shutdown()
{
	// Service Main Function deinitialization
	//

	m_cfgServerThread->quit();

	delete m_cfgServerThread;

	qDebug() << "ConfigurationServiceMainFunctionWorker stoped";
}


void ConfigurationServiceMainFunctionWorker::onInformationRequest(UdpRequest request)
{
	switch(request.ID())
	{
	case RQID_GET_CONFIGURATION_SERVICE_SETTINGS:
		onGetSettings(request);
		break;

	default:
		assert(false);
	}
}


void ConfigurationServiceMainFunctionWorker::onGetSettings(UdpRequest& request)
{
	/*quint32 count = request.readDword();

	QVector<DataSourceInfo> dsInfo;

	for(quint32 i = 0; i < count; i++)
	{
		quint32 sourceID = request.readDword();

		if (m_dataSources.contains(sourceID))
		{
			DataSourceInfo dsi;

			DataSource ds = m_dataSources.value(sourceID);

			ds.getInfo(dsi);

			dsInfo.append(dsi);
		}
	}

	UdpRequest ack;

	ack.initAck(request);

	count = static_cast<quint32>(dsInfo.count());

	ack.writeDword(count);

	for(quint32 i = 0; i < count; i++)
	{
		ack.writeStruct(&dsInfo[i]);
	}

	ackInformationRequest(ack);*/
}



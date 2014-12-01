#include <QCoreApplication>

#include "../include/BaseService.h"


#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visula Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif

// DataAquisitionService class
//
/*
class DataAquisitionService : public QtService<QCoreApplication>
{
private:
	BaseServiceController* m_baseServiceController = nullptr;
	int m_serviceType = 0;

public:
	DataAquisitionService(int argc, char ** argv, const QString & name, int serviceType);
	virtual ~DataAquisitionService();

	void sendFile(QHostAddress address, quint16 port, QString fileName);

protected:
	void start() override;
	void stop() override;
};


// BaseService class implementation
//


DataAquisitionService::DataAquisitionService(int argc, char ** argv, const QString & name, int serviceType):
	QtService(argc, argv, name),
	m_serviceType(serviceType)
{
	if ( !(m_serviceType >= 0 && m_serviceType < SERVICE_TYPE_COUNT))
	{
		assert(m_serviceType >= 0 && m_serviceType);

		m_serviceType = STP_BASE;
	}
}


BaseService::~BaseService()
{
}


void BaseService::start()
{
	m_baseServiceController = new BaseServiceController(m_serviceType);
}


void BaseService::stop()
{
	delete m_baseServiceController;
}


void BaseService::sendFile(QHostAddress address, quint16 port, QString fileName)
{
	assert(m_baseServiceController != nullptr);

	emit m_baseServiceController->sendFile(address, port, fileName);
}

*/
int main(int argc, char *argv[])
{
	//BaseService service(argc, argv, "RPCT Base Service", STP_BASE);

	return 1; //service.exec();
}

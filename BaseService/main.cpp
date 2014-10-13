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

// BaseService class
//

class BaseService : public QtService<QCoreApplication>
{
private:
	BaseServiceController* m_baseServiceController;
	int m_serviceType;

public:
	BaseService(int argc, char ** argv, const QString & name, int serviceType);
	virtual ~BaseService();

	void sendFile(QHostAddress address, quint16 port, QString fileName);

protected:
	void start() override;
	void stop() override;
};


// BaseService class implementation
//


BaseService::BaseService(int argc, char ** argv, const QString & name, int serviceType):
	QtService(argc, argv, name),
	m_baseServiceController(nullptr),
	m_serviceType(serviceType)
{
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


int main(int argc, char *argv[])
{
	BaseService service(argc, argv, "RPCT Base Service", STP_BASE);

    return service.exec();
}

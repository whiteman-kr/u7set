#include "ArchivingService.h"
#include "../lib/WUtils.h"


struct Data
{
	int v1 = 0;
	quint64 v2 = 0;
};

void testQueue()
{
	FastQueue<Data>	q(7);

	for(int i = 0; i < 9; i++)
	{
		Data d;

		d.v1 = i + 1;

		q.push(d);
	}

/*	Data qq;

	q.pop(&qq);*/

	Data buffer[6];

	int copied = 0;

	bool res = q.copyToBuffer(buffer, sizeof(buffer) / sizeof(Data), &copied);

	for(int i = 0; i < copied; i++)
	{
		qDebug() << buffer[i].v1;
	}
}

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

/*	testQueue();

	return 1;*/

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::ArchiveService, "", 1, 0);

	ArchivingServiceWorker archServiceWorker(si, "RPCT Archiving Service", argc, argv, logger);

	ServiceStarter serviceStarter(app, archServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}

#include <QCoreApplication>

#include "ConfigurationService.h"


#if defined(Q_OS_WIN) && defined(_MSC_VER)
    #include <vld.h>		// Enable Visual Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif


int main(int argc, char *argv[])
{
    QString buildFolder = "d:/temp/build";

    for(int i = 0; i < argc; i++)
    {
        QString arg = argv[i];

        if (arg.mid(0, 3) == "-b=")
        {
            buildFolder = arg.mid(3);
            break;
        }
    }

    ServiceStarter serviceStarter(argc, argv, "RPCT Configuration Service", new ConfigurationServiceWorker(buildFolder));

	return serviceStarter.exec();
}

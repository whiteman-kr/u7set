#include <QCoreApplication>
#include "BaseService.h"

int main(int argc, char *argv[])
{
    BaseService service(argc, argv, "RPCT Base Service");

    return service.exec();
}

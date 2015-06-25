#include "Stable.h"
#include "VFrame30Library.h"
#include "Scheme.h"
#include "DiagScheme.h"
#include "LogicScheme.h"
#include "WorkflowScheme.h"
#include "WiringScheme.h"
#include "SchemeLayer.h"
#include "SchemeItemLine.h"
#include "SchemeItemRect.h"
#include "SchemeItemPath.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "SchemeItemLink.h"
#include "SchemeItemAfb.h"
#include "SchemeItemSignal.h"
#include "SchemeItemConst.h"
#include "../include/Types.h"

namespace VFrame30
{
	VFrame30Library::VFrame30Library()
	{
		qDebug() << Q_FUNC_INFO;
	}

	bool VFrame30Library::Init()
	{
		qDebug() << Q_FUNC_INFO;

		// Registering Schemes
		//
		SchemeFactory.Register<DiagScheme>();
		SchemeFactory.Register<LogicScheme>();
		SchemeFactory.Register<WorkflowScheme>();
		SchemeFactory.Register<WiringScheme>();

		// Registering VideoLayers
		//
		VideoLayerFactory.Register<SchemeLayer>();

		// Registering SchemeItems
		//
		SchemeItemFactory.Register<SchemeItemLine>();
		SchemeItemFactory.Register<SchemeItemRect>();
		SchemeItemFactory.Register<SchemeItemPath>();
		SchemeItemFactory.Register<FblItemLine>();
		SchemeItemFactory.Register<FblItemRect>();
		SchemeItemFactory.Register<SchemeItemLink>();
		SchemeItemFactory.Register<SchemeItemAfb>();
		SchemeItemFactory.Register<SchemeItemInput>();
		SchemeItemFactory.Register<SchemeItemOutput>();
		SchemeItemFactory.Register<SchemeItemConst>();

		QMetaType::registerConverter<int, VFrame30::SchemeItemConst::ConstType>(IntToEnum<VFrame30::SchemeItemConst::ConstType>);

		return true;
	}

	bool VFrame30Library::Shutdown()
	{
		qDebug() << Q_FUNC_INFO;

		google::protobuf::ShutdownProtobufLibrary();
		return true;
	}
}

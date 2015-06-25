#include "Stable.h"
#include "VFrame30Library.h"
#include "Scheme.h"
#include "DiagScheme.h"
#include "LogicScheme.h"
#include "WorkflowScheme.h"
#include "WiringScheme.h"
#include "SchemeLayer.h"
#include "VideoItemLine.h"
#include "VideoItemRect.h"
#include "VideoItemConnectionLine.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "VideoItemLink.h"
#include "VideoItemFblElement.h"
#include "VideoItemSignal.h"
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

		// Registering VideoFrames
		//
		SchemeFactory.Register<DiagScheme>();
		SchemeFactory.Register<LogicScheme>();
		SchemeFactory.Register<WorkflowScheme>();
		SchemeFactory.Register<WiringScheme>();

		// Registering VideoLayers
		//
		VideoLayerFactory.Register<SchemeLayer>();

		// Registering VideoItems
		//
		VideoItemFactory.Register<VideoItemLine>();
		VideoItemFactory.Register<VideoItemRect>();
		VideoItemFactory.Register<VideoItemConnectionLine>();
		VideoItemFactory.Register<FblItemLine>();
		VideoItemFactory.Register<FblItemRect>();
		VideoItemFactory.Register<VideoItemLink>();
		VideoItemFactory.Register<VideoItemFblElement>();
		VideoItemFactory.Register<VideoItemInputSignal>();
		VideoItemFactory.Register<VideoItemOutputSignal>();
		VideoItemFactory.Register<SchemeItemConst>();

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

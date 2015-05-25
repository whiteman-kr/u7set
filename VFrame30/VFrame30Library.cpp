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

namespace VFrame30
{
	VFrame30Library::VFrame30Library()
	{

	}

	bool VFrame30Library::Init()
	{
		// Registering VideoFrames
		//
		VideoFrameFactory.Register<DiagScheme>();
		VideoFrameFactory.Register<LogicScheme>();
		VideoFrameFactory.Register<WorkflowScheme>();
		VideoFrameFactory.Register<WiringScheme>();

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

		return true;
	}

	bool VFrame30Library::Shutdown()
	{
		google::protobuf::ShutdownProtobufLibrary();
		return true;
	}
}

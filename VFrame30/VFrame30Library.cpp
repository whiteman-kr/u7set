#include "Stable.h"
#include "VFrame30Library.h"
#include "Scheme.h"
#include "VideoFrameDiag.h"
#include "VideoFrameLogic.h"
#include "VideoFrameTech.h"
#include "VideoFrameWiring.h"
#include "SchemeLayer.h"
#include "VideoItemLine.h"
#include "VideoItemRect.h"
#include "VideoItemConnectionLine.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "VideoItemLink.h"
#include "VideoItemFblElement.h"
#include "VideoItemSignal.h"

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

namespace VFrame30
{
	VFrame30Library::VFrame30Library()
	{

	}

	bool VFrame30Library::Init()
	{
		// Registering VideoFrames
		//
		VideoFrameFactory.Register<CVideoFrameDiag>();
		VideoFrameFactory.Register<CVideoFrameLogic>();
		VideoFrameFactory.Register<CVideoFrameTech>();
		VideoFrameFactory.Register<CVideoFrameWiring>();

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

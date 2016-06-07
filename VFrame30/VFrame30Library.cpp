#include "Stable.h"
#include "VFrame30Library.h"
#include "Schema.h"
#include "DiagSchema.h"
#include "LogicSchema.h"
#include "MonitorSchema.h"
#include "WiringSchema.h"
#include "SchemaLayer.h"
#include "SchemaItemLine.h"
#include "SchemaItemRect.h"
#include "SchemaItemPath.h"
#include "FblItemLine.h"
#include "FblItemRect.h"
#include "SchemaItemLink.h"
#include "SchemaItemAfb.h"
#include "SchemaItemSignal.h"
#include "SchemaItemConst.h"
#include "../lib/Types.h"

namespace VFrame30
{
	VFrame30Library::VFrame30Library()
	{
		qDebug() << Q_FUNC_INFO;
	}

	bool VFrame30Library::Init()
	{
		qDebug() << Q_FUNC_INFO;

		// Registering Schemas
		//
		SchemaFactory.Register<DiagSchema>();
		SchemaFactory.Register<LogicSchema>();
		SchemaFactory.Register<MonitorSchema>();
		SchemaFactory.Register<WiringSchema>();

		// Registering VideoLayers
		//
		VideoLayerFactory.Register<SchemaLayer>();

		// Registering SchemaItems
		//
		SchemaItemFactory.Register<SchemaItemLine>();
		SchemaItemFactory.Register<SchemaItemRect>();
		SchemaItemFactory.Register<SchemaItemPath>();
		SchemaItemFactory.Register<FblItemLine>();
		SchemaItemFactory.Register<FblItemRect>();
		SchemaItemFactory.Register<SchemaItemLink>();
		SchemaItemFactory.Register<SchemaItemAfb>();
		SchemaItemFactory.Register<SchemaItemInput>();
		SchemaItemFactory.Register<SchemaItemOutput>();
		SchemaItemFactory.Register<SchemaItemConst>();

		QMetaType::registerConverter<int, VFrame30::SchemaItemConst::ConstType>(IntToEnum<VFrame30::SchemaItemConst::ConstType>);

		return true;
	}

	bool VFrame30Library::Shutdown()
	{
		qDebug() << Q_FUNC_INFO;

		SchemaItem::PrintRefCounter("SchemaItem");

		google::protobuf::ShutdownProtobufLibrary();
		return true;
	}
}

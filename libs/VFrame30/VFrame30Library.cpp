#include <VFrame30/VFrame30Library.h>

#include <VFrame30/DiagSchema.h>
#include <VFrame30/FblItemLine.h>
#include <VFrame30/FblItemRect.h>
#include <VFrame30/LogicSchema.h>
#include <VFrame30/MonitorSchema.h>
#include <VFrame30/Schema.h>
#include <VFrame30/SchemaItemActuator.h>
#include <VFrame30/SchemaItemAfb.h>
#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaItemConnection.h>
#include <VFrame30/SchemaItemConst.h>
#include <VFrame30/SchemaItemDiagValue.h>
#include <VFrame30/SchemaItemFrame.h>
#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemIndicator.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/SchemaItemLink.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemPushButton.h>
#include <VFrame30/SchemaItemRect.h>
#include <VFrame30/SchemaItemSignal.h>
#include <VFrame30/SchemaItemSlider.h>
#include <VFrame30/SchemaItemTerminator.h>
#include <VFrame30/SchemaItemUfb.h>
#include <VFrame30/SchemaItemValue.h>
#include <VFrame30/SchemaItemVduImage.h>
#include <VFrame30/SchemaItemVduImageValue.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduTrend.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaLayer.h>
#include <VFrame30/TuningSchema.h>
#include <VFrame30/UfbSchema.h>
#include <VFrame30/VduSchema.h>
#include <VFrame30/WiringSchema.h>


namespace VFrame30
{
	bool init()
	{
		qDebug() << Q_FUNC_INFO;

		// Registering Schemas
		//
		SchemaFactory.Register<DiagSchema>();
		SchemaFactory.Register<LogicSchema>();
		SchemaFactory.Register<UfbSchema>();
		SchemaFactory.Register<MonitorSchema>();
		SchemaFactory.Register<TuningSchema>();
		SchemaFactory.Register<WiringSchema>();
		SchemaFactory.Register<VduSchema>();

		// Registering VideoLayers
		//
		VideoLayerFactory.Register<SchemaLayer>();

		// Registering SchemaItems
		//
		SchemaItemFactory.Register<FblItemLine>();
		SchemaItemFactory.Register<FblItemRect>();
		SchemaItemFactory.Register<SchemaItemActuator>();
		SchemaItemFactory.Register<SchemaItemAfb>();
		SchemaItemFactory.Register<SchemaItemBus>();
		SchemaItemFactory.Register<SchemaItemBusComposer>();
		SchemaItemFactory.Register<SchemaItemBusExtractor>();
		SchemaItemFactory.Register<SchemaItemConst>();
		SchemaItemFactory.Register<SchemaItemDiagValue>();
		SchemaItemFactory.Register<SchemaItemImage>();
		SchemaItemFactory.Register<SchemaItemImageValue>();
		SchemaItemFactory.Register<SchemaItemIndicator>();
		SchemaItemFactory.Register<SchemaItemInOut>();
		SchemaItemFactory.Register<SchemaItemInput>();
		SchemaItemFactory.Register<SchemaItemLine>();
		SchemaItemFactory.Register<SchemaItemLineEdit>();
		SchemaItemFactory.Register<SchemaItemLink>();
		SchemaItemFactory.Register<SchemaItemLoopback>();
		SchemaItemFactory.Register<SchemaItemLoopbackSource>();
		SchemaItemFactory.Register<SchemaItemLoopbackTarget>();
		SchemaItemFactory.Register<SchemaItemOutput>();
		SchemaItemFactory.Register<SchemaItemPath>();
		SchemaItemFactory.Register<SchemaItemPushButton>();
		SchemaItemFactory.Register<SchemaItemReceiver>();
		SchemaItemFactory.Register<SchemaItemRect>();
		SchemaItemFactory.Register<SchemaItemSlider>();
		SchemaItemFactory.Register<SchemaItemTerminator>();
		SchemaItemFactory.Register<SchemaItemTransmitter>();
		SchemaItemFactory.Register<SchemaItemUfb>();
		SchemaItemFactory.Register<SchemaItemValue>();

		SchemaItemFactory.Register<SchemaItemVduImage>();
		SchemaItemFactory.Register<SchemaItemVduImageValue>();
		SchemaItemFactory.Register<SchemaItemVduLine>();
		SchemaItemFactory.Register<SchemaItemVduRect>();
		SchemaItemFactory.Register<SchemaItemVduTrend>();
		SchemaItemFactory.Register<SchemaItemVduValue>();

		QMetaType::registerConverter<int, VFrame30::SchemaItemConst::ConstType>(IntToEnum<VFrame30::SchemaItemConst::ConstType>);

		qRegisterMetaType<E::ColumnData>();

		return true;
	}

	bool shutdown()
	{
		qDebug() << Q_FUNC_INFO;

		SchemaItem::PrintRefCounter();

		return true;
	}
} // namespace VFrame30

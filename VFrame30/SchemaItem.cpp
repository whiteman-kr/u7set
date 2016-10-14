#include "Stable.h"
#include "SchemaItem.h"
#include "SchemaItemRect.h"
#include "SchemaItemAfb.h"

namespace VFrame30
{
	Factory<VFrame30::SchemaItem> SchemaItemFactory;

	const QColor SchemaItem::errorColor(0xE0, 0x33, 0x33, 0xFF);
	const QColor SchemaItem::warningColor(0xF8, 0x72, 0x17, 0xFF);
	const QColor SchemaItem::selectionColor(0x33, 0x99, 0xFF, 0x80);
	const QColor SchemaItem::lockedSelectionColor(0xF0, 0x80, 0x80, 0xB0);
	const QColor SchemaItem::commentedColor(0xE0, 0xE0, 0xEF, 0xC0);

	// SchemaItem
	//
	SchemaItem::SchemaItem() :
		m_itemUnit(SchemaUnit::Display)
	{	
		m_guid = QUuid::createUuid();

		auto acceptClickProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::acceptClick, true, SchemaItem::acceptClick, SchemaItem::setAcceptClick);
		acceptClickProp->setCategory(PropertyNames::behaviourCategory);

		auto commentedProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::commented, true, SchemaItem::commented, SchemaItem::setCommented);
		commentedProp->setCategory(PropertyNames::functionalCategory);

		auto lockedProp = ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::locked, true, SchemaItem::isLocked, SchemaItem::setLocked);
		lockedProp->setCategory(PropertyNames::appearanceCategory);

		auto clickScriptProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::clickScript, true, SchemaItem::clickScript, SchemaItem::setClickScript);
		clickScriptProp->setCategory(PropertyNames::behaviourCategory);
	}

	SchemaItem::~SchemaItem()
	{
	}
	
	// Serialization
	//

	bool SchemaItem::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		Proto::SchemaItem* schemaItem = message->mutable_schemaitem();

		Proto::Write(schemaItem->mutable_uuid(), m_guid);
		schemaItem->set_isstatic(m_static);
		schemaItem->set_islocked(m_locked);
		schemaItem->set_iscommented(m_commented);
		schemaItem->set_itemunit(static_cast<Proto::SchemaUnit>(m_itemUnit));

		schemaItem->set_acceptclick(m_acceptClick);

		if (m_clickScript.isEmpty() == false)
		{
			schemaItem->set_clickscript(m_clickScript.toStdString());
		}

		return true;
	}

	bool SchemaItem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		const Proto::SchemaItem& schemaitem = message.schemaitem();

		m_guid = Proto::Read(schemaitem.uuid());
		m_static = schemaitem.isstatic();
		m_locked = schemaitem.islocked();
		m_commented = schemaitem.iscommented();
		m_itemUnit = static_cast<SchemaUnit>(schemaitem.itemunit());

		m_acceptClick = schemaitem.acceptclick();

		if (schemaitem.has_clickscript() == true)
		{
			m_clickScript = QString::fromStdString(schemaitem.clickscript());
		}
		else
		{
			m_clickScript.clear();
		}

		return true;
	}

	std::shared_ptr<SchemaItem> SchemaItem::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<SchemaItem> schemaItem = SchemaItemFactory.Create(classNameHash);

		if (schemaItem == nullptr)
		{
			assert(schemaItem);
			return nullptr;
		}
		
		schemaItem->LoadData(message);

		return schemaItem;
	}

	// Action Functions
	//

	void SchemaItem::MoveItem(double /*horzOffsetDocPt*/, double /*vertOffsetDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::snapToGrid(double /*gridSize*/)
	{
		assert(false);
	}

	double SchemaItem::GetWidthInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	double SchemaItem::GetHeightInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	void SchemaItem::SetWidthInDocPt(double /*widthInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::SetHeightInDocPt(double /*heightInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void SchemaItem::debug(std::shared_ptr<SchemaItem> item)
	{
		QString str = QString("SchemaItem %1, [ptr: %2, counter: %3], uuid: %4")
					  .arg(item->metaObject()->className())
					  .arg(reinterpret_cast<qulonglong>(item.get()))
					  .arg(item.use_count())
					  .arg(item->guid().toString());

		qDebug() << str;
	}

	void SchemaItem::debug() const
	{
		qDebug() << "Item: " << metaObject()->className();
		qDebug() << "\tguid:" << guid();
	}

	bool SchemaItem::searchText(const QString& /*text*/) const
	{
		return false;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItem::Draw(CDrawParam*, const Schema*, const SchemaLayer*) const
	{
	}

	// Рисование элемента при его создании изменении
	//
	void SchemaItem::DrawOutline(CDrawParam* ) const
	{
	}

	void SchemaItem::DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items)
	{
		if (pDrawParam == nullptr)
		{
			assert(pDrawParam != nullptr);
			return;
		}

		for (auto vi = items.cbegin(); vi != items.cend(); ++vi)				
		{
			vi->get()->DrawOutline(pDrawParam);
		}
	}

	void SchemaItem::DrawIssue(CDrawParam* /*drawParam*/, OutputMessageLevel /*issue*/) const
	{
		assert(false);
	}

	void SchemaItem::DrawDebugInfo(CDrawParam* /*drawParam*/, const QString& /*runOrderIndex*/) const
	{
	}

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void SchemaItem::DrawSelection(CDrawParam*, bool) const
	{
	}

	void SchemaItem::DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items, bool drawSizeBar)
	{
		if (pDrawParam == nullptr)
		{
			assert(pDrawParam != nullptr);
			return;
		}

		for (auto vi = items.cbegin(); vi != items.cend(); ++vi)				
		{
			vi->get()->DrawSelection(pDrawParam, drawSizeBar);
		}
	}

	void SchemaItem::drawCommentDim(CDrawParam* /*drawParam*/) const
	{

	}

	// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
	// 
	bool SchemaItem::IsIntersectPoint(double x, double y) const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return IsIntersectRect(x, y, 1, 1);
		}
		else
		{
			// Из точки делается прямоугольник 0.5мм на 0.5мм (0.02 in )
			//
			return IsIntersectRect(x - 0.01, y - 0.01, 0.02, 0.02);
		}
	}

	// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
	// координаты и размер прямоугольника заданы в дюймах или пикселях
	// 
	bool SchemaItem::IsIntersectRect(double x, double y, double width, double height) const
	{ 
		x = x; y = y; width = width; height = height;		// убираю unreferenced warning
		assert(false);
		return false;
	};

	double SchemaItem::penDeviceWidth(const QPaintDevice* device, double penWidth)
	{
		if (dynamic_cast<const QPdfWriter*>(device) != nullptr)
		{
			const QPdfWriter* pdfDevice = dynamic_cast<const QPdfWriter*>(device);

			if (pdfDevice->resolution() >= 600)
			{
				if (penWidth == 0.0)
				{
					return 1.0 / 256.0;
				}
			}
		}

		return penWidth;
	}

	// ISchemaItemPropertiesPos interface implementation
	//
	double SchemaItem::left() const
	{
		assert(true);
		return 0;
	}
	void SchemaItem::setLeft(double)
	{
		assert(true);
	}

	double SchemaItem::top() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setTop(double)
	{
		assert(false);
	}

	double SchemaItem::width() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setWidth(double)
	{
		assert(false);
	}

	double SchemaItem::height() const
	{
		assert(false);
		return 0;
	}
	void SchemaItem::setHeight(double)
	{
		assert(false);
	}

	std::vector<SchemaPoint> SchemaItem::getPointList() const
	{
		Q_ASSERT(false);
		return std::vector<SchemaPoint>();
	}

	void SchemaItem::setPointList(const std::vector<SchemaPoint>& /*points*/)
	{
		Q_ASSERT(false);
	}

	// Properties and Data
	//

	bool SchemaItem::IsStatic() const
	{
		return m_static;
	}

	bool SchemaItem::IsDynamic() const
	{
		return !m_static;
	}

	bool SchemaItem::isFblItemRect() const
	{
		return dynamic_cast<const FblItemRect*>(this) != nullptr;
	}

	FblItemRect* SchemaItem::toFblItemRect()
	{
		return dynamic_cast<FblItemRect*>(this);
	}

	const FblItemRect* SchemaItem::toFblItemRect() const
	{
		return dynamic_cast<const FblItemRect*>(this);
	}

	bool SchemaItem::isFblItem() const
	{
		return dynamic_cast<const FblItem*>(this) != nullptr;
	}

	bool SchemaItem::isSchemaItemAfb() const
	{
		return dynamic_cast<const SchemaItemAfb*>(this) != nullptr;
	}

	SchemaItemAfb* SchemaItem::toSchemaItemAfb()
	{
		return dynamic_cast<SchemaItemAfb*>(this);
	}

	const SchemaItemAfb* SchemaItem::toSchemaItemAfb() const
	{
		return dynamic_cast<const SchemaItemAfb*>(this);
	}

	bool SchemaItem::isLocked() const
	{
		return m_locked;
	}

	void SchemaItem::setLocked(bool lock)
	{
		m_locked = lock;
		return;
	}

	bool SchemaItem::isCommented() const
	{
		return m_commented;
	}

	bool SchemaItem::commented() const
	{
		return m_commented;
	}

	void SchemaItem::setCommented(bool value)
	{
		m_commented = value;
	}

	const QUuid& SchemaItem::guid() const
	{
		return m_guid;
	}

	void SchemaItem::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	void SchemaItem::setNewGuid()
	{
		QUuid uuid = QUuid::createUuid();
		setGuid(uuid);

		return;
	}

	// Единицы измерения, в которых хранятся координаты (может быть только дюймы или точки)
	//
	SchemaUnit SchemaItem::itemUnit() const
	{
		return m_itemUnit;
	}

	void SchemaItem::setItemUnit(SchemaUnit value)
	{
		assert(value == SchemaUnit::Display || value == SchemaUnit::Inch);
		m_itemUnit = value;
	}

	// AcceptClick property
	//
	bool SchemaItem::acceptClick() const
	{
		return m_acceptClick;
	}

	void SchemaItem::setAcceptClick(bool value)
	{
		m_acceptClick = value;
	}

	// ClickScript property
	//
	const QString& SchemaItem::clickScript() const
	{
		return m_clickScript;
	}

	void SchemaItem::setClickScript(const QString& value)
	{
		m_clickScript = value;
	}

	QRectF SchemaItem::boundingRectInDocPt() const
	{
		assert(false);		// Must be implemented in child classes
		return QRectF();
	}

}

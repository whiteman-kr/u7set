#include "Stable.h"
#include "VideoItem.h"
#include "../include/VFrameUtils.h"
#include "VideoItemRect.h"

namespace VFrame30
{
	Factory<VFrame30::CVideoItem> VideoItemFactory;

	// CVideoItem

	CVideoItem::CVideoItem() :
		m_static(true),
		m_locked(false),
		m_itemUnit(Display),
		m_acceptClick(false)
	{	
		m_guid = QUuid::createUuid();
	}

	CVideoItem::~CVideoItem()
	{
	}
	
	// Serialization
	//

	bool CVideoItem::SaveData(VFrame30::Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CVFrameUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		VFrame30::Proto::VideoItem* pMutableVideoItem = message->mutable_videoitem();

		VFrame30::Proto::Write(pMutableVideoItem->mutable_guid(), m_guid);
		pMutableVideoItem->set_isstatic(m_static);
		pMutableVideoItem->set_islocked(m_locked);
		pMutableVideoItem->set_itemunit(static_cast<VFrame30::Proto::SchemeUnit>(m_itemUnit));

		pMutableVideoItem->set_acceptclick(m_acceptClick);

		if (m_clickScript.isEmpty() == false)
		{
			VFrame30::Proto::Write(pMutableVideoItem->mutable_clickscript(), m_clickScript);
		}

		return true;
	}

	bool CVideoItem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		const VFrame30::Proto::VideoItem& videoitem = message.videoitem();

		m_guid = VFrame30::Proto::Read(videoitem.guid());
		m_static = videoitem.isstatic();
		m_locked = videoitem.islocked();
		m_itemUnit = static_cast<SchemeUnit>(videoitem.itemunit());

		m_acceptClick = videoitem.acceptclick();

		if (videoitem.has_clickscript() == true)
		{
			m_clickScript = Proto::Read(videoitem.clickscript());
		}
		else
		{
			m_clickScript.clear();
		}

		return true;
	}

	CVideoItem* CVideoItem::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		CVideoItem* pVideoItem = VideoItemFactory.Create(classNameHash);

		if (pVideoItem == nullptr)
		{
			assert(pVideoItem);
			return nullptr;
		}
		
		pVideoItem->LoadData(message);

		return pVideoItem;
	}

	// Action Functions
	//

	void CVideoItem::MoveItem(double /*horzOffsetDocPt*/, double /*vertOffsetDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	double CVideoItem::GetWidthInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	double CVideoItem::GetHeightInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	void CVideoItem::SetWidthInDocPt(double /*widthInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void CVideoItem::SetHeightInDocPt(double /*heightInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void CVideoItem::Draw(CDrawParam*, const CVideoFrame*, const CVideoLayer*) const
	{
	}

	// Рисование элемента при его создании изменении
	//
	void CVideoItem::DrawOutline(CDrawParam* ) const
	{
	}

	void CVideoItem::DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<CVideoItem>>& items)
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

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void CVideoItem::DrawSelection(CDrawParam*, bool) const
	{
	}

	void CVideoItem::DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<CVideoItem>>& items, bool drawSizeBar)
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

	// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
	// 
	bool CVideoItem::IsIntersectPoint(double x, double y) const
	{
		if (itemUnit() == SchemeUnit::Display)
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
	bool CVideoItem::IsIntersectRect(double x, double y, double width, double height) const
	{ 
		x = x; y = y; width = width; height = height;		// убираю unreferenced warning
		assert(false);
		return false;
	};

	// IVideoItemPropertiesPos interface implementation
	//
	double CVideoItem::left() const
	{
		assert(true);
		return 0;
	}
	void CVideoItem::setLeft(double)
	{
		assert(true);
	}

	double CVideoItem::top() const
	{
		assert(false);
		return 0;
	}
	void CVideoItem::setTop(double)
	{
		assert(false);
	}

	double CVideoItem::width() const
	{
		assert(false);
		return 0;
	}
	void CVideoItem::setWidth(double)
	{
		assert(false);
	}

	double CVideoItem::height() const
	{
		assert(false);
		return 0;
	}
	void CVideoItem::setHeight(double)
	{
		assert(false);
	}

	std::vector<VideoItemPoint> CVideoItem::getPointList() const
	{
		Q_ASSERT(false);
		return std::vector<VideoItemPoint>();
	}

	void CVideoItem::setPointList(const std::vector<VideoItemPoint>& /*points*/)
	{
		Q_ASSERT(false);
	}

	// Properties and Data
	//

	bool CVideoItem::IsStatic() const
	{
		return m_static;
	}

	bool CVideoItem::IsDynamic() const
	{
		return !m_static;
	}

	bool CVideoItem::IsFblItem() const
	{
		return false;
	}

	bool CVideoItem::IsLocked() const
	{
		return m_locked;
	}

	void CVideoItem::setLocked(bool lock)
	{
		m_locked = lock;
		return;
	}

	const QUuid& CVideoItem::guid() const
	{
		return m_guid;
	}

	void CVideoItem::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// Единицы измерения, в которых хранятся координаты (может быть только дюймы или точки)
	//
	SchemeUnit CVideoItem::itemUnit() const
	{
		return m_itemUnit;
	}

	void CVideoItem::setItemUnit(SchemeUnit value)
	{
		assert(value == Display || value == Inch);
		m_itemUnit = value;
	}

	// AcceptClick property
	//
	bool CVideoItem::acceptClick() const
	{
		return m_acceptClick;
	}

	void CVideoItem::setAcceptClick(bool value)
	{
		m_acceptClick = value;
	}

	// ClickScript property
	//
	const QString& CVideoItem::clickScript() const
	{
		return m_clickScript;
	}

	void CVideoItem::setClickScript(const QString value)
	{
		m_clickScript = value;
	}

	QRectF CVideoItem::boundingRectInDocPt() const
	{
		assert(false);		// Must be implemented in child classes
		return QRectF();
	}

}

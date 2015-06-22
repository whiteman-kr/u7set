#include "Stable.h"
#include "VideoItem.h"
#include "VideoItemRect.h"

namespace VFrame30
{
	Factory<VFrame30::VideoItem> VideoItemFactory;

	// CVideoItem

	VideoItem::VideoItem() :
		m_static(true),
		m_locked(false),
		m_itemUnit(SchemeUnit::Display),
		m_acceptClick(false)
	{	
		m_guid = QUuid::createUuid();
	}

	VideoItem::~VideoItem()
	{
	}
	
	// Serialization
	//

	bool VideoItem::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// ������������ ����, �� ����� ������, �� ���� ����������������� �����.

		Proto::VideoItem* pMutableVideoItem = message->mutable_videoitem();

		Proto::Write(pMutableVideoItem->mutable_uuid(), m_guid);
		pMutableVideoItem->set_isstatic(m_static);
		pMutableVideoItem->set_islocked(m_locked);
		pMutableVideoItem->set_itemunit(static_cast<Proto::SchemeUnit>(m_itemUnit));

		pMutableVideoItem->set_acceptclick(m_acceptClick);

		if (m_clickScript.isEmpty() == false)
		{
			Proto::Write(pMutableVideoItem->mutable_clickscript(), m_clickScript);
		}

		return true;
	}

	bool VideoItem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		const Proto::VideoItem& videoitem = message.videoitem();

		m_guid = Proto::Read(videoitem.uuid());
		m_static = videoitem.isstatic();
		m_locked = videoitem.islocked();
		m_itemUnit = static_cast<SchemeUnit>(videoitem.itemunit());

		m_acceptClick = videoitem.acceptclick();

		if (videoitem.has_clickscript() == true)
		{
			Proto::Read(videoitem.clickscript(), &m_clickScript);
		}
		else
		{
			m_clickScript.clear();
		}

		return true;
	}

	VideoItem* VideoItem::CreateObject(const Proto::Envelope& message)
	{
		// ��� ������� ����� ��������� ������ ���� ���������
		//
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		VideoItem* pVideoItem = VideoItemFactory.Create(classNameHash);

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

	void VideoItem::MoveItem(double /*horzOffsetDocPt*/, double /*vertOffsetDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void VideoItem::snapToGrid(double /*gridSize*/)
	{
		assert(false);
	}

	double VideoItem::GetWidthInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	double VideoItem::GetHeightInDocPt() const
	{
		assert(false);	// Implement in child classes
		return 0;
	}

	void VideoItem::SetWidthInDocPt(double /*widthInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	void VideoItem::SetHeightInDocPt(double /*heightInDocPt*/)
	{
		assert(false);	// Implement in child classes
	}

	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
	//
	void VideoItem::Draw(CDrawParam*, const Scheme*, const SchemeLayer*) const
	{
	}

	// ��������� �������� ��� ��� �������� ���������
	//
	void VideoItem::DrawOutline(CDrawParam* ) const
	{
	}

	void VideoItem::DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<VideoItem>>& items)
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

	// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
	//
	void VideoItem::DrawSelection(CDrawParam*, bool) const
	{
	}

	void VideoItem::DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<VideoItem>>& items, bool drawSizeBar)
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

	// �����������, ������ �� ����� � �������, x � y � ������ ��� � ��������
	// 
	bool VideoItem::IsIntersectPoint(double x, double y) const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return IsIntersectRect(x, y, 1, 1);
		}
		else
		{
			// �� ����� �������� ������������� 0.5�� �� 0.5�� (0.02 in )
			//
			return IsIntersectRect(x - 0.01, y - 0.01, 0.02, 0.02);
		}
	}

	// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
	// ���������� � ������ �������������� ������ � ������ ��� ��������
	// 
	bool VideoItem::IsIntersectRect(double x, double y, double width, double height) const
	{ 
		x = x; y = y; width = width; height = height;		// ������ unreferenced warning
		assert(false);
		return false;
	};

	// IVideoItemPropertiesPos interface implementation
	//
	double VideoItem::left() const
	{
		assert(true);
		return 0;
	}
	void VideoItem::setLeft(double)
	{
		assert(true);
	}

	double VideoItem::top() const
	{
		assert(false);
		return 0;
	}
	void VideoItem::setTop(double)
	{
		assert(false);
	}

	double VideoItem::width() const
	{
		assert(false);
		return 0;
	}
	void VideoItem::setWidth(double)
	{
		assert(false);
	}

	double VideoItem::height() const
	{
		assert(false);
		return 0;
	}
	void VideoItem::setHeight(double)
	{
		assert(false);
	}

	std::vector<VideoItemPoint> VideoItem::getPointList() const
	{
		Q_ASSERT(false);
		return std::vector<VideoItemPoint>();
	}

	void VideoItem::setPointList(const std::vector<VideoItemPoint>& /*points*/)
	{
		Q_ASSERT(false);
	}

	// Properties and Data
	//

	bool VideoItem::IsStatic() const
	{
		return m_static;
	}

	bool VideoItem::IsDynamic() const
	{
		return !m_static;
	}

	bool VideoItem::IsFblItem() const
	{
		return false;
	}

	bool VideoItem::IsLocked() const
	{
		return m_locked;
	}

	void VideoItem::setLocked(bool lock)
	{
		m_locked = lock;
		return;
	}

	const QUuid& VideoItem::guid() const
	{
		return m_guid;
	}

	void VideoItem::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	void VideoItem::setNewGuid()
	{
		QUuid uuid = QUuid::createUuid();
		setGuid(uuid);

		return;
	}

	// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)
	//
	SchemeUnit VideoItem::itemUnit() const
	{
		return m_itemUnit;
	}

	void VideoItem::setItemUnit(SchemeUnit value)
	{
		assert(value == SchemeUnit::Display || value == SchemeUnit::Inch);
		m_itemUnit = value;
	}

	// AcceptClick property
	//
	bool VideoItem::acceptClick() const
	{
		return m_acceptClick;
	}

	void VideoItem::setAcceptClick(bool value)
	{
		m_acceptClick = value;
	}

	// ClickScript property
	//
	const QString& VideoItem::clickScript() const
	{
		return m_clickScript;
	}

	void VideoItem::setClickScript(const QString& value)
	{
		m_clickScript = value;
	}

	QRectF VideoItem::boundingRectInDocPt() const
	{
		assert(false);		// Must be implemented in child classes
		return QRectF();
	}

}

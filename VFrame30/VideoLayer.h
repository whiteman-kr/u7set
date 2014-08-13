#pragma once
#include "VideoItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CVideoLayer : 
		public QObject,
		public Proto::CVFrameObjectSerialization<CVideoLayer>,
		public DebugInstCounter<CVideoLayer>
	{
		Q_OBJECT

	public:
		CVideoLayer(void);
		CVideoLayer(const QString& name, bool compile);
		virtual ~CVideoLayer(void);

	private:
		void Init(const QString& name, bool compile);

		// Serialization
		//
		friend Proto::CVFrameObjectSerialization<CVideoLayer>;

	private:
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		static CVideoLayer* CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:

		// ���� � connectionMap ���� pinPos, �� ���������������� ��������, ����� �������� ����� ������ �� ���������� 1
		//
		void ConnectionMapPosInc(VideoItemPoint pinPos);
		int GetPinPosConnectinCount(VideoItemPoint pinPos) const;

		std::shared_ptr<CVideoItem> getItemUnderPoint(QPointF point) const;
		std::list<std::shared_ptr<CVideoItem>> getItemListInRectangle(const QRectF& rect) const;

		// Properties
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString name() const;
		void setName(const QString& value);

		bool compile() const;
		void setCompile(bool value);

		bool show() const;
		void setShow(bool value);

		bool print() const;
		void setPrint(bool value);

		// Data
		//
	public:
		// �������� ���� 
		//
		std::list<std::shared_ptr<CVideoItem>> Items;

		// ������� ��������� ���� �����, ��������� �������� ���������� ��������� ������� �� �����
		//
		std::map<VideoItemPoint, int> connectionMap;		// ���� - ���������� ����, �������� - ���������� ����� � ���������

	private:
		QUuid m_guid;
		QString m_name;
		bool m_compile;
		bool m_show;
		bool m_print;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::CVideoLayer> VideoLayerFactory;
#endif

}

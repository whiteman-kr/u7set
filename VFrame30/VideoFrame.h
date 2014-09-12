#pragma once

#include "VideoLayer.h"
#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class CDrawParam;
	class VideoFrameWidgetAgent;
	
	class VFRAME30LIBSHARED_EXPORT CVideoFrame : 
		public QObject,
		public Proto::ObjectSerialization<CVideoFrame>,
		public DebugInstCounter<CVideoFrame>
	{
		Q_OBJECT
	
	protected:
		CVideoFrame(void);
	
	public:
		virtual ~CVideoFrame(void);

		void Init(void);

		// Serialization
		//
		friend Proto::ObjectSerialization<CVideoFrame>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		//
		static CVideoFrame* CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect) const;
		void Print();

		virtual void MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const;
		void RunClickScript(const std::shared_ptr<CVideoItem>& videoItem, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const;

		// �������� ������ ��������� � �����
		//
		int GetDocumentWidth(double DpiX, double zoom) const;
		int GetDocumentHeight(double DpiY, double zoom) const;

		int GetLayerCount() const;
		
		// ��������� connectionMap ������� �������� � ������ ����.
		// std::map<VideoItemPoint, int> connectionMap		���� - ���������� ����, �������� - ���������� ����������� � ����
		//
		void BuildFblConnectionMap() const;

		// Properties and Datas
		//
	public:
		QUuid guid() const;
		void setGuid(const QUuid& guid);

		QString strID() const;
		void setStrID(const QString& strID);

		QString caption() const;
		void setCaption(const QString& caption);

		double docWidth() const;
		void setDocWidth(double width);

		double docHeight() const;
		void setDocHeight(double height);

		SchemeUnit unit() const;
		void setUnit(SchemeUnit value);
		
	public:
		std::vector<std::shared_ptr<CVideoLayer>> Layers;

	private:
		QUuid m_guid;
		QString m_strID;
		QString m_caption;

		double m_width;				// pixels or inches, depends on m_unit
		double m_height;			// pixels or inches, depends on m_unit

		SchemeUnit m_unit;			// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)
	};


	class VFRAME30LIBSHARED_EXPORT VideoFrameSharedPtr
	{
	public:
		VideoFrameSharedPtr(const std::shared_ptr<CVideoFrame>& sp) : m_sp(sp)
		{
		}

		std::shared_ptr<CVideoFrame> get()
		{
			return m_sp;
		}
		
	private:
		std::shared_ptr<CVideoFrame> m_sp;
	};



#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::CVideoFrame> VideoFrameFactory;
#endif
}



#ifndef FBLITEM_H
#define FBLITEM_H

#include "VideoItem.h"
#include "Fbl.h"

class QPainter;

namespace VFrame30
{
	enum ConnectionDirrection
	{
		Input,
		Output
	};

	// CFblConnectionPoint
	//
	class CFblConnectionPoint
	{
	private:
		CFblConnectionPoint();

	public:
		CFblConnectionPoint(double x, double y, ConnectionDirrection dirrection, const QUuid& guid);
		CFblConnectionPoint(const Proto::FblConnectionPoint& cpm);

		// Other
		//

		// Serialization
		//
		bool SaveData(Proto::FblConnectionPoint* cpm) const;
		bool LoadData(const Proto::FblConnectionPoint& cpm);

		// Properties
		//
	public:
		const VideoItemPoint& point() const
		{
			return m_point;
		}
		void setPoint(const VideoItemPoint& value)
		{
			m_point = value;
		}

		double x() const
		{
			return m_point.X;
		}
		void setX(double val)
		{
			m_point.X = val;
		}

		double y() const
		{
			return m_point.Y;
		}
		void setY(double val)
		{
			m_point.Y = val;
		}

		ConnectionDirrection dirrection() const
		{
			return m_dirrection;
		}
		bool IsInput() const
		{
			return m_dirrection == ConnectionDirrection::Input;
		}
		bool IsOutput() const
		{
			return m_dirrection == ConnectionDirrection::Output;
		}

		const QUuid& guid() const
		{
			return m_guid;
		}
		void setGuid(const QUuid& guid)
		{
			m_guid = guid;
		}

		const std::list<QUuid>& associatedIOs() const
		{
			return m_associatedIOs;
		}
		void ClearAssociattdIOs()
		{
			m_associatedIOs.clear();
		}
		void AddAssociattedIOs(const QUuid& guid)
		{
			m_associatedIOs.push_back(guid);
		}

		bool HasConnection() const
		{
			assert(!(IsInput() && m_associatedIOs.size() > 1));
			return !m_associatedIOs.empty();
		}

		const QUuid& signalGuid() const
		{
			return m_signalGuid;
		}
		void setSignalGuid(const QUuid& guid)
		{
			m_signalGuid = guid;
		}

		const QString& signalStrID() const
		{
			return m_signalStrID;
		}
		void setSignalStrID(const QString& strid)
		{
			m_signalStrID = strid;
		}

		const QString& signalCaption() const
		{
			return m_signalCaption;
		}
		void setSignalCaption(const QString& caption)
		{
			m_signalCaption = caption;
		}
			
		// Data
		//
	private:
		QUuid m_guid;
		VideoItemPoint m_point;
		ConnectionDirrection m_dirrection;
		std::list<QUuid> m_associatedIOs;	// if connection is an output, the list contains GUID associated inputs
		
		QUuid m_signalGuid;					// Guid сигнала ассоциаированного с данной точкой, может быть GUID_NULL
		QString m_signalStrID;				// Строковый ИД сигнала ассоциаированного с данной точкой,
											// может быть пустой строкой, используется для кэширования, 
											// не использовать для логики, только для пунктов меню, отрисовки и т.п.
		QString m_signalCaption;			// Наименование сигнала ассоциаированного с данной точкой,
											// может быть пустой строкой, используется для кэширования, 
											// не использовать для логики, только для пунктов меню, отрисовки и т.п.
	};

	// CFblItem
	//
	class VFRAME30LIBSHARED_EXPORT CFblItem
	{
	protected:
		CFblItem(void);

	public:
		virtual ~CFblItem(void);
		
	public:
		// Serialization
		//
	protected:
		virtual bool SaveData(VFrame30::Proto::Envelope* message) const;
		virtual bool LoadData(const VFrame30::Proto::Envelope& message);

		// Drawing stuff
		//
	protected:
		void DrawPinCross(QPainter* p, double x, double y, double pinWidth) const;
		void DrawPinJoint(QPainter* p, double x, double y, double pinWidth) const;

		double GetPinWidth(SchemeUnit unit, int dpi) const;

		// Connections
		//
	public:
		const std::list<CFblConnectionPoint>& inputs() const;
		const std::list<CFblConnectionPoint>& outputs() const;

		std::list<CFblConnectionPoint>* mutableInputs();
		std::list<CFblConnectionPoint>* mutableOutputs();

		bool GetConnectionPoint(const QUuid& guid, CFblConnectionPoint* pResult) const;

		int inputsCount() const;
		int outputsCount() const;

	protected:
		void AddInput();
		void AddOutput();

		// Вычислить координаты точки
		//
	public:
		void ClearAssociatedConnections();
		virtual void SetConnectionsPos();
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const;

		// Properties
		//
	private:
		std::list<CFblConnectionPoint> m_inputPoints;
		std::list<CFblConnectionPoint> m_outputPoints;
	};
}

#endif

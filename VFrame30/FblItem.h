#pragma once

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
	class VFRAME30LIBSHARED_EXPORT CFblConnectionPoint
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
		const VideoItemPoint& point() const;
		void setPoint(const VideoItemPoint& value);

		double x() const;
		void setX(double val);

		double y() const;
		void setY(double val);

		ConnectionDirrection dirrection() const;
		bool IsInput() const;
		bool IsOutput() const;

		const QUuid& guid() const;
		void setGuid(const QUuid& guid);

		const std::list<QUuid>& associatedIOs() const;
		void ClearAssociattdIOs();
		void AddAssociattedIOs(const QUuid& guid);

		bool HasConnection() const;

//		const QUuid& signalGuid() const;
//		void setSignalGuid(const QUuid& guid);

//		const QString& signalStrID() const;
//		void setSignalStrID(const QString& strid);

//		const QString& signalCaption() const;
//		void setSignalCaption(const QString& caption);

		// Data
		//
	private:
		QUuid m_guid;
		VideoItemPoint m_point;
		ConnectionDirrection m_dirrection;
		std::list<QUuid> m_associatedIOs;	// if connection is an output, the list contains GUID associated inputs
		
		//QUuid m_signalGuid;				// Guid сигнала ассоциаированного с данной точкой, может быть GUID_NULL
		//QString m_signalStrID;			// Строковый ИД сигнала ассоциаированного с данной точкой,
											// может быть пустой строкой, используется для кэширования, 
											// не использовать для логики, только для пунктов меню, отрисовки и т.п.
		//QString m_signalCaption;			// Наименование сигнала ассоциаированного с данной точкой,
											// может быть пустой строкой, используется для кэширования, 
											// не использовать для логики, только для пунктов меню, отрисовки и т.п.
	};

	// CFblItem
	//
	class VFRAME30LIBSHARED_EXPORT FblItem
	{
	protected:
		FblItem(void);

	public:
		virtual ~FblItem(void);
		
	public:
		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const;
		virtual bool LoadData(const Proto::Envelope& message);

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



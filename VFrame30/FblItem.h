#pragma once

#include "SchemeItem.h"
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
	public:
		CFblConnectionPoint();

		CFblConnectionPoint(ConnectionDirrection dirrection,
							const QUuid& guid,
							int operandIndex,
							QString caption);

		CFblConnectionPoint(ConnectionDirrection dirrection, const QUuid& guid, const Afbl::AfbElementSignal& afbSignal);

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
		const SchemePoint& point() const;
		void setPoint(const SchemePoint& value);

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

		int afbOperandIndex() const;
		void setAfbOperandIndex(int value);

		QString caption() const;
		void setCaption(QString caption);

		// Data
		//
	private:
		QUuid m_guid;
		SchemePoint m_point;
		ConnectionDirrection m_dirrection = ConnectionDirrection::Input;
		int m_afbOperandIndex = 0;

		std::list<QUuid> m_associatedIOs;	// if connection is an output, the list contains GUID associated inputs
		
		QString m_caption;					// Pin caption
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

		void addInput();
		void addInput(const Afbl::AfbElementSignal& s);
		void addInput(int opIndex, QString caption);

		void addOutput();
		void addOutput(const Afbl::AfbElementSignal& s);
		void addOutput(int opIndex, QString caption);

		void removeAllInputs();
		void removeAllOutputs();

		void ClearAssociatedConnections();
		virtual void SetConnectionsPos(double gridSize, int pinGridStep);
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemePoint* pResult, double gridSize, int pinGridStep) const;

		// Public methods
		//
	public:
		virtual QString buildName() const;

		// Properties
		//
	private:
		std::list<CFblConnectionPoint> m_inputPoints;
		std::list<CFblConnectionPoint> m_outputPoints;
	};
}



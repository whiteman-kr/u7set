#pragma once

#include "SchemaItem.h"
#include "Afb.h"

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
	class VFRAME30LIBSHARED_EXPORT AfbPin
	{
	public:
		AfbPin();

		AfbPin(ConnectionDirrection dirrection,
			   const QUuid& guid,
			   int operandIndex,
			   QString caption);

		AfbPin(ConnectionDirrection dirrection, const QUuid& guid, const Afb::AfbSignal& afbSignal);

		AfbPin(const Proto::FblConnectionPoint& cpm);

		// Other
		//

		// Serialization
		//
		bool SaveData(Proto::FblConnectionPoint* cpm) const;
		bool LoadData(const Proto::FblConnectionPoint& cpm);

		// Properties
		//
	public:
		const SchemaPoint& point() const;
		void setPoint(const SchemaPoint& value);

		double x() const;
		void setX(double val);

		double y() const;
		void setY(double val);

		ConnectionDirrection dirrection() const;
		bool IsInput() const;
		bool IsOutput() const;

		const QUuid& guid() const;
		void setGuid(const QUuid& guid);

		const std::vector<QUuid>& associatedIOs() const;
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
		SchemaPoint m_point;				// Don't remove position!!!
		ConnectionDirrection m_dirrection = ConnectionDirrection::Input;
		int m_afbOperandIndex = 0;

		std::vector<QUuid> m_associatedIOs;	// if connection is an output, the list contains GUID associated inputs
		
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

		double GetPinWidth(SchemaUnit unit, int dpi) const;
		double GetPinWidth(SchemaUnit unit, QPaintDevice* device) const;

		// Connections
		//
	public:
		const std::vector<VFrame30::AfbPin>& inputs() const;
		const std::vector<VFrame30::AfbPin>& outputs() const;

		std::vector<AfbPin>* mutableInputs();
		std::vector<AfbPin>* mutableOutputs();

		bool GetConnectionPoint(const QUuid& guid, VFrame30::AfbPin* pResult) const;

		int inputsCount() const;
		int outputsCount() const;

		void addInput();
		void addInput(const Afb::AfbSignal& s);
		void addInput(int opIndex, QString caption);

		void addOutput();
		void addOutput(const Afb::AfbSignal& s);
		void addOutput(int opIndex, QString caption);

		void removeAllInputs();
		void removeAllOutputs();

		void ClearAssociatedConnections();
		virtual void SetConnectionsPos(double gridSize, int pinGridStep);
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const;

		// Public methods
		//
	public:
		virtual QString buildName() const;

		// Properties
		//
	private:
		std::vector<VFrame30::AfbPin> m_inputPoints;
		std::vector<VFrame30::AfbPin> m_outputPoints;
	};
}



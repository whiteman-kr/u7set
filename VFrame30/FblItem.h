#pragma once

#include "SchemaPoint.h"
#include "../lib/Types.h"

class QPainter;

namespace Afb
{
	class AfbPin;
	class AfbSignal;
}

namespace VFrame30
{
	struct SchemaPoint;

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
			   E::SignalType signalType,
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
		std::vector<QUuid>& associatedIOs();
		void ClearAssociattdIOs();
		void AddAssociattedIOs(const QUuid& guid);

		bool removeFromAssociatedIo(const QUuid& uuid);

		bool hasAssociatedIo(const QUuid& guid) const;

		bool HasConnection() const;

		int afbOperandIndex() const;
		void setAfbOperandIndex(int value);

		E::SignalType signalType() const;			// Here we care ONLY about is it BUS or its regular signal, ceep in mind that a lot of code does not care about analog/discrete pin
		void setSignalType(E::SignalType value);

		QString caption() const;
		void setCaption(QString caption);

		// Data
		//
	private:
		QUuid m_guid;
		SchemaPoint m_point;				// Don't remove position!!!
		ConnectionDirrection m_dirrection = ConnectionDirrection::Input;
		int m_afbOperandIndex = 0;
		E::SignalType m_signalType;			// Here we care ONLY about is it BUS or its regular signal, ceep in mind that a lot of code does not care about analog/discrete pin

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
		std::vector<VFrame30::AfbPin>& inputs();

		const std::vector<VFrame30::AfbPin>& outputs() const;
		std::vector<VFrame30::AfbPin>& outputs();

		std::vector<AfbPin>* mutableInputs();
		std::vector<AfbPin>* mutableOutputs();

		bool GetConnectionPoint(const QUuid& guid, VFrame30::AfbPin* pResult) const;

		int inputsCount() const;
		int outputsCount() const;

		void addInput();
		void addInput(const Afb::AfbSignal& s);
		void addInput(int opIndex, E::SignalType signalType, QString caption);

		void addOutput();
		void addOutput(const Afb::AfbSignal& s);
		void addOutput(int opIndex, E::SignalType signalType, QString caption);

		void removeAllInputs();
		void removeAllOutputs();

		void ClearAssociatedConnections();
		virtual void SetConnectionsPos(double gridSize, int pinGridStep);
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const;

		bool hasInput(const QUuid& guid) const;
		bool hasOutput(const QUuid& guid) const;

		void setNewGuid();					// FblItem is not derived from SchemaIte, so this func cannot be virtual

		bool searchText(const QString& text) const;

		const VFrame30::AfbPin& input(const QUuid& guid) const;
		VFrame30::AfbPin& input(const QUuid& guid);

		const VFrame30::AfbPin& output(const QUuid& guid) const;
		VFrame30::AfbPin& output(const QUuid& guid);

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



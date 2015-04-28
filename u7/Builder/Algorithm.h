#pragma once


#include <QObject>
#include "../VFrame30/Fbl.h"
#include "../include/Types.h"

namespace Builder
{
	class AlgItem;


	class Pin
	{
	private:
		AlgItem* m_parentAlgItem = nullptr;

		// properties from AfbElementSignal
		//
		QString m_caption;
		SignalType m_signalType = SignalType::Discrete;		// like Afbl::AfbSignalType
		int m_index = -1;
		int m_size = 0;

	public:
		Pin() {}
		Pin(AlgItem* parentAlgItem, int index, SignalType signalType, int size);

		Pin& operator = (const Pin& pin);

		virtual bool isOutPin() const = 0;
		virtual bool isInPin() const = 0;

		virtual bool isConnected() const = 0;
		bool isCompatible(const Pin* pin) const;

		void setSignalType(SignalType signalType) { m_signalType = signalType; }
		SignalType signalType() const { return m_signalType; }

		void setSize(int size) { m_size = size; }
		int size() const { return m_size; }

		void setIndex(int index) { m_index = index; }
		int index() const { return m_index; }

		AlgItem* parent() const { return m_parentAlgItem; }
	};


	class InPin;

	typedef const InPin* ConstInPinPtr;
	typedef QVector<ConstInPinPtr> ConstInPinPtrArray;


	class OutPin : public Pin
	{
	private:
		ConstInPinPtrArray m_inPins;

	public:
		OutPin() {}
		OutPin(AlgItem *parentAlgItem, int index, SignalType signalType, int size);

		OutPin& operator = (const OutPin& outPin);

		bool isOutPin() const override { return  true; }
		bool isInPin() const override { return  false; }

		bool isConnected() const override { return m_inPins.count() != 0; }

		void connect(const InPin* inPin);
		void disconnect(const InPin* inPin);
	};


	typedef const OutPin* ConstOutPinPtr;


	class InPin : public Pin
	{
	private:
		ConstOutPinPtr m_outPin = nullptr;

	public:
		InPin() {}
		InPin(AlgItem* parentAlgItem, int index, SignalType signalType, int size);

		InPin& operator = (const InPin& inPin);

		bool isOutPin() const override { return  false; }
		bool isInPin() const override { return  true; }

		bool isConnected() const override { return m_outPin != nullptr; }

		void connect(const OutPin* outPin);
		void disconnect(const OutPin* outPin);
	};


	class AlgItem
	{
	protected:
		QVector<InPin> m_inPins;
		QVector<OutPin> m_outPins;

		// Scheme Item Pointer m_schemeItem;
	};


	class AlgSignal : public AlgItem
	{
	private:
		SignalType m_signalType = SignalType::Discrete;
		int m_size = 0;

	public:
		AlgSignal(SignalType signalType, int size);
	};


	class AlgInSignal : public AlgSignal
	{
	public:
		AlgInSignal(SignalType signalType, int size);
	};


	class AlgOutSignal : public AlgSignal
	{
	public:
		AlgOutSignal(SignalType signalType, int size);
	};


	class AlgFbParam
	{
	private:
		bool m_initialized = false;
	};


	typedef QVector<AlgFbParam> AlgFbParamArray;


	class AlgFb : public AlgItem
	{
	private:
		AlgFbParamArray m_params;
		AlgFbParamArray m_constParam;

	public:
		AlgFb(const Afbl::AfbElement &afbElement);
	};


	typedef QVector<AlgItem*> AlgItemPtrArray;

	class Algorithm : public QObject
	{
		Q_OBJECT
	private:
		AlgItemPtrArray m_algItems;

	public:
		explicit Algorithm(QObject *parent = 0);

		static bool connect(OutPin* outPin, InPin* inPin);
	};
}


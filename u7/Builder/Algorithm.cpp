#include "Algorithm.h"

namespace Builder
{

	// ---------------------------------------------------------------------------
	//
	// Pin class implementation
	//

	/*Pin::Pin(const Pin& pin)
	{
		m_parentAlgItem = pin.m_parentAlgItem;
		m_index = pin.m_index;
		m_signalType = pin.m_signalType;
		m_size = pin.m_size;
		m_caption = pin.m_caption;
	}*/


	Pin::Pin(AlgItem *parentAlgItem, int index, SignalType signalType, int size) :
		m_parentAlgItem(parentAlgItem),
		m_index(index),
		m_signalType(signalType),
		m_size(size)
	{
	}


	Pin& Pin::operator = (const Pin& pin)
	{
		m_parentAlgItem = pin.m_parentAlgItem;
		m_caption = pin.m_caption;
		m_signalType = pin.m_signalType;
		m_index = pin.m_index;
		m_size = pin.m_size;

		return *this;
	}


	bool Pin::isCompatible(const Pin* pin) const
	{
		if (pin == nullptr)
		{
			assert(false);
			return false;
		}

		if (size() == pin->size() && signalType() == pin->signalType())
		{
			return true;
		}

		return false;
	}



	// ---------------------------------------------------------------------------
	//
	// OutPin class implementation
	//

	OutPin::OutPin(AlgItem* parentAlgItem, int index, SignalType signalType, int size) :
		Pin(parentAlgItem, index, signalType, size)
	{
	}


	OutPin& OutPin::operator = (const OutPin& outPin)
	{
		Pin::operator =(outPin);
		m_inPins = outPin.m_inPins;

		return *this;
	}


	void OutPin::connect(const InPin* inPin)
	{
		if (inPin == nullptr)
		{
			assert(false);
			return;
		}

		if (!isCompatible(inPin))
		{
			assert(false);
			return;
		}

		for(ConstInPinPtr inPinPtr : m_inPins)
		{
			if (inPinPtr == inPin)
			{
				assert(false);		// assert for warning, NOT error! repeated connect
				return;
			}
		}

		m_inPins.append(inPin);
	}


	void OutPin::disconnect(const InPin* inPin)
	{
		if (inPin == nullptr)
		{
			assert(false);
			return;
		}

		int count = m_inPins.count();

		for(int i = 0; i < count; i++)
		{
			if (m_inPins[i] == inPin)
			{
				m_inPins.removeAt(i);
				return;
			}
		}

		assert(false);		// assert for warning, NOT error! pins is not connected
	}


	// ---------------------------------------------------------------------------
	//
	// InPin class implementation
	//

	InPin::InPin(AlgItem *parentAlgItem, int index, SignalType signalType, int size) :
		Pin(parentAlgItem, index, signalType, size)
	{
	}


	InPin& InPin::operator = (const InPin& inPin)
	{
		Pin::operator =(inPin);
		m_outPin = inPin.m_outPin;

		return *this;
	}


	void InPin::connect(const OutPin* outPin)
	{
		if (outPin == nullptr)
		{
			assert(false);
			return;
		}

		if (!isCompatible(outPin))
		{
			assert(false);
			return;
		}

		if (m_outPin == outPin)
		{
			assert(false);		// assert for warning, NOT error! pins already interconnected, repeated connect
		}
		else
		{
			if (m_outPin != nullptr)
			{
				assert(false);		// assert for warning, NOT error! pin already connected
									// preferably use "disconnect" before "connect"
			}
		}

		m_outPin = outPin;
	}


	void InPin::disconnect(const OutPin* outPin)
	{
		if (outPin == nullptr)
		{
			assert(false);
			return;
		}

		if (m_outPin != outPin)
		{
			assert(false);		// assert for warning, NOT error! pins is not connected
		}

		m_outPin = nullptr;
	}


	// ---------------------------------------------------------------------------
	//
	// AlgSignal class implementation
	//

	AlgSignal::AlgSignal(SignalType signalType, int size) :
		m_signalType(signalType),
		m_size(size)
	{
		assert(size > 0 && size < 16);	// !
	}


	// ---------------------------------------------------------------------------
	//
	// AlgInSignal class implementation
	//

	AlgInSignal::AlgInSignal(SignalType signalType, int size) :
		AlgSignal(signalType, size)
	{
		// add one output pin
		//
		OutPin outPin(this, 0, signalType, size);

		m_outPins.append(outPin);
	}


	// ---------------------------------------------------------------------------
	//
	// AlgInSignal class implementation
	//

	AlgOutSignal::AlgOutSignal(SignalType signalType, int size) :
		AlgSignal(signalType, size)
	{
		// add one input pin
		//
		InPin inPin(this, 0, signalType, size);

		m_inPins.append(inPin);
	}


	// ---------------------------------------------------------------------------
	//
	// AlgFb class implementation
	//

	AlgFb::AlgFb(const Afbl::AfbElement& afbElement)
	{
//fwefiwjefijwoefji
	}


	// ---------------------------------------------------------------------------
	//
	// Algorithm class implementation
	//

	Algorithm::Algorithm(QObject *parent) :
		QObject(parent)
	{
	}


	bool Algorithm::connect(OutPin* outPin, InPin* inPin)
	{
		if (outPin == nullptr || inPin == nullptr)
		{
			assert(false);
			return false;
		}

		outPin->connect(inPin);
		inPin->connect(outPin);

		return true;
	}
}

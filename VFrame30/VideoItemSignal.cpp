#include "Stable.h"
#include "VideoItemSignal.h"

namespace VFrame30
{
	//
	// CVideoItemSignal
	//
	CVideoItemSignal::CVideoItemSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemSignal::CVideoItemSignal(SchemeUnit unit) :
		CFblItemRect(unit)
	{
	}

	CVideoItemSignal::~CVideoItemSignal(void)
	{
	}

	bool CVideoItemSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = CFblItemRect::SaveData(message);

		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemSignal* signal = */message->mutable_videoitem()->mutable_signal();

		//pSignal->set_leftdocpt(leftDocPt);

		return true;
	}

	bool CVideoItemSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}
		
		// --
		//
		bool result = CFblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_signal() == false)
		{
			assert(message.videoitem().has_signal());
			return false;
		}
		/*const Proto::VideoItemSignal& signal = */message.videoitem().signal();

		//leftDocPt = signal.leftdocpt();

		return true;
	}


	//
	// CVideoItemInputSignal
	//
	CVideoItemInputSignal::CVideoItemInputSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemInputSignal::CVideoItemInputSignal(SchemeUnit unit) :
		CVideoItemSignal(unit)
	{
		AddOutput();
	}

	CVideoItemInputSignal::~CVideoItemInputSignal(void)
	{
#ifdef _DEBUG
		assert(outputsCount() == 1);
#endif 
	}

	// Serialization
	//
	bool CVideoItemInputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = CVideoItemSignal::SaveData(message);
		
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemInputSignal* inputSignal = */message->mutable_videoitem()->mutable_inputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool CVideoItemInputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CVideoItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		// --
		//
		if (message.videoitem().has_inputsignal() == false)
		{
			assert(message.videoitem().has_inputsignal());
			return false;
		}

		/*const Proto::VideoItemInputSignal& inputSignal = */message.videoitem().inputsignal();
		//fill = inputSignal.fill();

		return true;
	}
	
	//
	// CVideoItemOutputSignal
	//
	CVideoItemOutputSignal::CVideoItemOutputSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemOutputSignal::CVideoItemOutputSignal(SchemeUnit unit) :
		CVideoItemSignal(unit)
	{
		AddInput();
	}

	CVideoItemOutputSignal::~CVideoItemOutputSignal(void)
	{
#ifdef _DEBUG
		assert(inputsCount() == 1);
#endif
	}

	// Serialization
	//
	bool CVideoItemOutputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = CVideoItemSignal::SaveData(message);
		
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* outputSignal = */message->mutable_videoitem()->mutable_outputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool CVideoItemOutputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CVideoItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_outputsignal() == false)
		{
			assert(message.videoitem().has_outputsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& outputSignal = */message.videoitem().outputsignal();
		//fill = inputSignal.fill();

		return true;
	}

}


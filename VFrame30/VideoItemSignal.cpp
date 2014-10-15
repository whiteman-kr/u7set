#include "Stable.h"
#include "VideoItemSignal.h"

namespace VFrame30
{
	//
	// CVideoItemSignal
	//
	VideoItemSignal::VideoItemSignal(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	VideoItemSignal::VideoItemSignal(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	VideoItemSignal::~VideoItemSignal(void)
	{
	}

	bool VideoItemSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

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

	bool VideoItemSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}
		
		// --
		//
		bool result = FblItemRect::LoadData(message);
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
	VideoItemInputSignal::VideoItemInputSignal(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	VideoItemInputSignal::VideoItemInputSignal(SchemeUnit unit) :
		VideoItemSignal(unit)
	{
		AddOutput();
	}

	VideoItemInputSignal::~VideoItemInputSignal(void)
	{
#ifdef _DEBUG
		assert(outputsCount() == 1);
#endif 
	}

	// Serialization
	//
	bool VideoItemInputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = VideoItemSignal::SaveData(message);
		
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

	bool VideoItemInputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = VideoItemSignal::LoadData(message);
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
	VideoItemOutputSignal::VideoItemOutputSignal(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	VideoItemOutputSignal::VideoItemOutputSignal(SchemeUnit unit) :
		VideoItemSignal(unit)
	{
		AddInput();
	}

	VideoItemOutputSignal::~VideoItemOutputSignal(void)
	{
#ifdef _DEBUG
		assert(inputsCount() == 1);
#endif
	}

	// Serialization
	//
	bool VideoItemOutputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = VideoItemSignal::SaveData(message);
		
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

	bool VideoItemOutputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = VideoItemSignal::LoadData(message);
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


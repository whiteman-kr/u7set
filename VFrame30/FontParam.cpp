#include "Stable.h"
#include "FontParam.h"
#include "../include/ProtoSerialization.h"

namespace VFrame30
{
	FontParam::FontParam()
	{
		name = "Arial";
		size = 0;
		bold = false;
		italic = false;
	}

	bool FontParam::SaveData(Proto::FontParam* message) const
	{
		Proto::Write(message->mutable_name(), this->name);
		message->set_size(this->size);
		message->set_bold(this->bold);
		message->set_italic(this->italic);
		return true;
	}

	bool FontParam::LoadData(const Proto::FontParam& message)
	{
		this->name = Proto::Read(message.name());
		this->size = message.size();
		this->bold = message.bold();
		this->italic = message.italic();
		return true;
	}

}

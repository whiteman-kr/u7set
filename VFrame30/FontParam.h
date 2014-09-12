#pragma once


#define DECLARE_FONT_PROPERTIES(propname) \
	const QString& Get##propname##Name() const; \
		void Set##propname##Name(QString& value); \
		\
		double Get##propname##Size() const; \
		void Set##propname##Size(double value); \
		\
		bool Get##propname##Bold() const; \
		void Set##propname##Bold(bool value); \
		\
		bool Get##propname##Italic() const; \
		void Set##propname##Italic(bool value);

	
#define IMPLEMENT_FONT_PROPERTIES(classname, propname, varname) \
	const QString& classname::Get##propname##Name() const { 		return varname.name;	}\
	void classname::Set##propname##Name(QString& value) { varname.name = value; } \
	double classname::Get##propname##Size() const \
	{ \
		if (itemUnit() == SchemeUnit::Display) \
		{ \
			return CVFrameUtils::RoundDisplayPoint(varname.size); \
		} \
		else \
		{ \
			double pt = CVFrameUtils::ConvertPoint(varname.size, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz); \
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit()); \
		} \
	} \
	void classname::Set##propname##Size(double value) \
	{ \
		value = std::max(value, 0.0); \
		if (itemUnit() == SchemeUnit::Display) \
		{ \
			this->varname.size = CVFrameUtils::RoundDisplayPoint(value); \
		} \
		else \
		{ \
			double pt = CVFrameUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz); \
			this->varname.size = pt; \
		} \
	} \
	bool classname::Get##propname##Bold() const	{ return varname.bold; } \
	void classname::Set##propname##Bold(bool value)	{ varname.bold = value; } \
	bool classname::Get##propname##Italic() const { return varname.italic; } \
	void classname::Set##propname##Italic(bool value) { varname.italic = value; } 


namespace Proto
{
	class FontParam;
}

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FontParam
	{
	public:
		FontParam();

		// Serialization
		//
	public:
		bool SaveData(Proto::FontParam* message) const;
		bool LoadData(const Proto::FontParam& message);
	
		// --
		//
	public:
		QString name;
		double size;
		bool bold;
		bool italic;
	};
}



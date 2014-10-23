#pragma once
#include <functional>
#include <Settings.h>


#define DECLARE_FONT_PROPERTIES(propname) \
		const QString& get##propname##Name() const; \
		void set##propname##Name(const QString& value); \
		\
		double get##propname##Size() const; \
		void set##propname##Size(double value); \
		\
		bool get##propname##Bold() const; \
		void set##propname##Bold(bool value); \
		\
		bool get##propname##Italic() const; \
		void set##propname##Italic(bool value);


#define IMPLEMENT_FONT_PROPERTIES(classname, propname, varname) \
	\
	const QString& classname::get##propname##Name() const		{ 	return varname.name();	} \
	void classname::set##propname##Name(const QString& value)	{	varname.setName(value); } \
	\
	double classname::get##propname##Size() const				{	return varname.size(CSettings::regionalUnit());	} \
	void classname::set##propname##Size(double value)			{	varname.setSize(value, CSettings::regionalUnit());	} \
	\
	bool classname::get##propname##Bold() const					{	return varname.bold();	} \
	void classname::set##propname##Bold(bool value)				{	varname.setBold(value); } \
	\
	bool classname::get##propname##Italic() const				{ return varname.italic();	} \
	void classname::set##propname##Italic(bool value)			{ varname.setItalic(value); }


/*#define DECLARE_FONT_PROPERTIES(propname) \
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
			return CUtils::RoundDisplayPoint(varname.size); \
		} \
		else \
		{ \
			double pt = CUtils::ConvertPoint(varname.size, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz); \
			return CUtils::RoundPoint(pt, CSettings::regionalUnit()); \
		} \
	} \
	void classname::Set##propname##Size(double value) \
	{ \
		value = std::max(value, 0.0); \
		if (itemUnit() == SchemeUnit::Display) \
		{ \
			this->varname.size = CUtils::RoundDisplayPoint(value); \
		} \
		else \
		{ \
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz); \
			this->varname.size = pt; \
		} \
	} \
	bool classname::Get##propname##Bold() const	{ return varname.bold; } \
	void classname::Set##propname##Bold(bool value)	{ varname.bold = value; } \
	bool classname::Get##propname##Italic() const { return varname.italic; } \
	void classname::Set##propname##Italic(bool value) { varname.italic = value; } 

*/
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

		// Properties
		//
	public:
		const QString& name() const;
		void setName(const QString& value);

		double size(SchemeUnit unit) const;
		void setSize(double value, SchemeUnit unit);
		double drawSize() const;

		bool bold() const;
		void setBold(bool value);

		bool italic() const;
		void setItalic(bool value);

		// Data
		//
	private:
		QString m_name;
		double m_size = 0;
		bool m_bold = false;
		bool m_italic = false;
	};
}



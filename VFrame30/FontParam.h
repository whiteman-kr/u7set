#pragma once
#include <functional>
#include <Settings.h>
#include "../include/TypesAndEnums.h"


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
	double classname::get##propname##Size() const \
	{	\
		return (itemUnit() == SchemaUnit::Display) ? varname.size(SchemaUnit::Display) : varname.size(Settings::regionalUnit());\
	}\
	void classname::set##propname##Size(double value) \
	{\
		varname.setSize(value, (itemUnit() == SchemaUnit::Display) ? SchemaUnit::Display : Settings::regionalUnit());\
	} \
	\
	bool classname::get##propname##Bold() const					{	return varname.bold();	} \
	void classname::set##propname##Bold(bool value)				{	varname.setBold(value); } \
	\
	bool classname::get##propname##Italic() const				{ return varname.italic();	} \
	void classname::set##propname##Italic(bool value)			{ varname.setItalic(value); }

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

		double size(SchemaUnit unit) const;
		void setSize(double value, SchemaUnit unit);

		double drawSize() const;
		void setDrawSize(double value);

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



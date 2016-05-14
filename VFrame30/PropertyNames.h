#ifndef PROPERTYNAMES_H
#define PROPERTYNAMES_H
#include <QString>

namespace VFrame30
{

	class PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		static const QString acceptClick;
		static const QString clickScript;

		static const QString fontName;
		static const QString fontSize;
		static const QString fontBold;
		static const QString fontItalic;

		static const QString type;
		static const QString valueInteger;
		static const QString valueFloat;
		static const QString precision;

		static const QString lineColor;
		static const QString lineWeight;
		static const QString fillColor;
		static const QString fill;
		static const QString drawRect;
		static const QString textColor;
		static const QString text;

		static const QString alignHorz;
		static const QString alignVert;

		static const QString appSignalIDs;

		static const QString behaviourCategory;
		static const QString appearanceCategory;
		static const QString functionalCategory;
		static const QString textCategory;
	};

}
#endif // PROPERTYNAMES_H
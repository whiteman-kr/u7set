#pragma once

#include <QMetaType>
#include <QColor>

namespace Behavior
{
	/// @brief The color pair for the behavior tag.
	struct BehaviorColorPair
	{
		Q_GADGET

		/// @brief Returns true if the object is valid, otherwise returns false.
		Q_PROPERTY(bool isValid MEMBER isValid)

		/// @brief The first color.
		Q_PROPERTY(QColor color1 MEMBER color1)

		/// @brief The second color.
		Q_PROPERTY(QColor color2 MEMBER color2)

	public:
		bool isValid = false;
		QColor color1{};
		QColor color2{};
	};

} // namespace Behavior

Q_DECLARE_METATYPE(Behavior::BehaviorColorPair)
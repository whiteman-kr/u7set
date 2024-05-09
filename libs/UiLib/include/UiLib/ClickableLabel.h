#pragma once

#include <QLabel>

namespace UiLib
{
	class ClickableLabel : public QLabel
	{
		Q_OBJECT

	public:
		explicit ClickableLabel(QString text, QWidget* parent = nullptr) :
			QLabel{text, parent}
		{
		}

	protected:
		void mousePressEvent(QMouseEvent*) { emit clicked(); }

	signals:
		void clicked();
	};
} // namespace UiLib
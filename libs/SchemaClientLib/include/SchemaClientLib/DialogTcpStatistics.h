#pragma once

#include <QDialog>

namespace SchemaClientLib
{
	class DialogTcpStatistics : public QDialog
	{
		Q_OBJECT

	public:
		explicit DialogTcpStatistics(QWidget* parent);

	protected:
		virtual void reject() override;

	signals:
		void dialogClosed();
	};
} // namespace SchemaClientLib
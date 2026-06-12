#ifndef DIALOGTRENDSIGNALPROPERTIES_H
#define DIALOGTRENDSIGNALPROPERTIES_H

#include <TrendView/TrendSignal.h>

#include <QDialog>
#include <QLabel>

namespace Ui
{
	class DialogTrendSignalProperties;
}

namespace TrendLib
{
	class TrendSignalSet;

	class DialogTrendSignalProperties : public QDialog
	{
		Q_OBJECT

	public:
		DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal,
									TrendLib::TrendSignalSet* trendSignalSet,
									E::TimeType timeType,
									E::TrendScaleType scaleType,
									E::TrendMode trendMode,
									QWidget* parent);
		virtual ~DialogTrendSignalProperties();

		const TrendLib::TrendSignalParam& trendSignal() const; // result

	signals:
		void signalPropertiesChanged();                        // emitted when user presses apply or OK button

	public slots:
		virtual void accept() override;

	private slots:
		void on_buttonPoints_clicked();
		void on_buttonApply_clicked();
		void on_buttonApplyToAll_clicked();
		void on_resetHigh_clicked();
		void on_resetLow_clicked();
		void on_viewFormatCombo_currentIndexChanged(const QString& text);

	private:
		void initUi();
		void fillProperties();
		void updateModifiedLabels();
		bool applyProperties(TrendLib::TrendSignalParam& trendSignal);

	private:
		Ui::DialogTrendSignalProperties* ui;

		TrendLib::TrendSignalParam m_trendSignal;

		// Parameters needed for points dialog
		//
		TrendLib::TrendSignalSet* m_trendSignalSet = nullptr;
		QString m_appSignalId;
		E::TimeType m_timeType = E::TimeType::Plant;
		E::TrendScaleType m_scaleType = E::TrendScaleType::Linear;
		E::TrendMode m_trendMode = E::TrendMode::Archive;

		union
		{
			struct
			{
				bool precision : 1;
				bool format : 1;
				bool viewHighLimit : 1;
				bool viewLowLimit : 1;
				bool color : 1;
				bool lineWeight : 1;
			} bits;
			int value;
		} m_modifiedFields = { 0 };

		//
	};
} // namespace TrendLib

class ChooseColorWidget : public QLabel
{
	Q_OBJECT

public:
	explicit ChooseColorWidget(QWidget* parent = nullptr);

	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;

	QColor color() const;
	void setColor(QColor value);

	bool modified() const;

signals:
	void colorChanged();

private:
	QColor m_color = Qt::black;
	bool m_modified = false;
};

#endif // DIALOGTRENDSIGNALPROPERTIES_H

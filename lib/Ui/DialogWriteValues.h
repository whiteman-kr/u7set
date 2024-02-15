#ifndef DIALOGWRITEVALUES_H
#define DIALOGWRITEVALUES_H

#include <QDialog>
#include "../../AppSignalLib/AppSignalParam.h"

class TuningValuesTreeWidget : public QTreeWidget
{
private:
	QSize sizeHint() const override;

};

class DialogWriteValues : public QDialog
{
	Q_OBJECT

public:
	static int askConfirmation(const AppSignalParam& param,
							   const TuningValue& oldValue,
							   const TuningValue& newValue,
							   E::AnalogFormat analogFormat,
							   QWidget* parent);

	static int askConfirmation(std::vector<AppSignalParam>& params,
							   std::vector<TuningValue>& oldValues,
							   std::vector<TuningValue>& newValues,
							   E::AnalogFormat analogFormat,
							   QWidget* parent);
private:
	DialogWriteValues(const std::vector<AppSignalParam>& params,
					  const std::vector<TuningValue>& oldValues,
					  const std::vector<TuningValue>& newValues,
					  E::AnalogFormat analogFormat,
					  QWidget* parent);

private:
	void fillTable(int maxCount);


private slots:
	void onShowAll();


private:
	const int m_defaultSignalsCount = 20;

	const std::vector<AppSignalParam> m_params;
	const std::vector<TuningValue> m_oldValues;
	const std::vector<TuningValue> m_newValues;
	E::AnalogFormat m_analogFormat;

	QPushButton* m_showAll = nullptr;
	TuningValuesTreeWidget* m_table = nullptr;
};

#endif // DIALOGWRITEVALUES_H

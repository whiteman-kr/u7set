#ifndef DBPROGRESS_H
#define DBPROGRESS_H

//
//
// DbProgress
//
//
class DbProgress : public QObject
{
public:
	DbProgress();
	virtual ~DbProgress();

	bool init();
	bool run(QWidget* parentWidget, const QString& description);

	bool completed() const;
	void setCompleted(bool value);

	bool wasCanceled() const;
	void setCancel(bool value);

	QString currentOperation() const;
	void setCurrentOperation(const QString& value);

	int value() const;
	void setValue(int value);

	QString errorMessage() const;
	void setErrorMessage(const QString& value);

	bool hasError() const;

	QString completeMessage() const;
	void setCompleteMessage(const QString& value);

	void enableProgress();
	void disableProgress();
	bool isProgressEnabled();

private:
	mutable QMutex m_mutex;

	bool m_completed;				// Set true if operation is completed
	bool m_cancel;					// Cancel operation

	QString m_currentOperation;

	int m_value;					// 0 - 100%

	QString m_errorMessage;			// In case of error, this variable will contain error description
	QString m_completeMessage;		// If this field is not empty, show message box with the text

	bool m_progressEnabled;			// QProgressDialog is enabled
};


#endif // DBPROGRESS_H

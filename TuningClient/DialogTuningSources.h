#ifndef DIALOGTUNINGSOURCES_H
#define DIALOGTUNINGSOURCES_H

#include <SchemaClientLib/TuningSourcesWidget.h>


namespace ClientLib
{
	class TuningConnection;
	class TuningUserManager;
}

class ClientTuningSourcesWidget : public SchemaClientLib::TuningSourcesWidget
{
	Q_OBJECT
public:

	explicit ClientTuningSourcesWidget(ClientLib::TuningConnection& connection, ClientLib::TuningUserManager& userManager, bool hasActivationControls, QWidget* parent);
	virtual ~ClientTuningSourcesWidget();

protected:

	virtual bool login() override;

private:
	ClientLib::TuningUserManager& m_userManager;
};


//
// DialogTuningSources
//

class DialogTuningSources : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTuningSources(ClientLib::TuningConnection& tuningConnection, ClientLib::TuningUserManager& userManager, bool hasActivationControls, QWidget* parent);
	virtual ~DialogTuningSources();

protected:
	virtual void reject() override;

signals:
	void dialogClosed();

private:
	ClientTuningSourcesWidget* m_tuningSourcesWidget = nullptr;
	QPushButton* m_btnEnableControl = nullptr;
	QPushButton* m_btnDisableControl = nullptr;

	ClientLib::TuningConnection& m_tuningConnection;
};


#endif // DIALOGTUNINGSOURCES_H

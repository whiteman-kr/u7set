#pragma once

namespace Locator
{
	struct LocatedItem
	{
		QString what;		// Mathced text, it can be objects' equipmentId (USB_...), schemaId (ABCD), appSignalId (#ABC), so on
		QString caption;	// Caption on the object
		QVariant data;		// For schema: DbFileInfo
	};


	class LocatorProvider : public QObject
	{
		Q_OBJECT

	public:
		[[nodiscard]] virtual QString name() const;

	protected:
		virtual void locateFor(const QString& text);
		virtual void stopSearching();

	public slots:
		void slot_locateFor(QString text);
		void slot_stopSearching();

	signals:
		void resultReady(QString text, LocatorProvider* provider, std::vector<LocatedItem> result);
	};
}

#pragma once

#include "SchemaLayer.h"
#include "../CommonLib/DebugInstCounter.h"
#include "../CommonLib/Factory.h"
#include "../UtilsLib/ILogFile.h"
//#include "../Proto/ProtoSerialization.h"


namespace Afb
{
	class AfbElement;
}

namespace AppSignalLib
{
	class Bus;
}


namespace VFrame30
{
	class Context;
	class Schema;
	class SchemaLayer;
	class CDrawParam;
	class SchemaItem;
	class LogicSchema;
	class UfbSchema;

	extern ::Factory<VFrame30::Schema> SchemaFactory;

	// Proxy class for using in scripts
	//
	/*! \class ScriptSchema
		\ingroup view
		\brief Represents a class that contains schema. Can be get from global <b>view.Schema</b>.
	*/
	class ScriptSchema : public QObject
	{
		Q_OBJECT

		/// \brief Schema unique identifier (SchemaID).
		Q_PROPERTY(QString schemaID READ schemaId)
		Q_PROPERTY(QString SchemaID READ schemaId)

		/// \brief Schema caption.
		Q_PROPERTY(QString caption READ caption)
		Q_PROPERTY(QString Caption READ caption)

		/// \brief Schema background color.
		Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
		Q_PROPERTY(QColor BackgroundColor READ backgroundColor WRITE setBackgroundColor)

		/// \brief Layer count.
		Q_PROPERTY(int layerCount READ layerCount)

	public:
		explicit ScriptSchema(std::shared_ptr<Schema> schema);
		virtual ~ScriptSchema() = default;

	public slots:
		/// \brief Return true if is ApplicationLogic schema.
		bool isLogicSchema() const;

		/// \brief Return true if is UserFunctionalBlock schema.
		bool isUfbSchema() const;

		/// \brief Return true if is Monitor schema.
		bool isMonitorSchema() const;

		/// \brief Return true if is TuningClient schema.
		bool isTuningSchema() const;

		/// \brief Return true if is Diag schema.
		bool isDiagSchema() const;

		/// \brief Get schema layer by index. Returned value has type ScriptSchemaLayer or undefined if layer is not found.
		QJSValue layer(int index);

		/// \brief Get schema layer by caption. Returned value has type ScriptSchemaLayer or undefined if layer is not found.
		QJSValue layer(QString caption);

		/// \brief Get schema items with specified tag.
		QVariantList itemsByTag(QString tag);

		/// \brief Finds schema item by its name (ObjectName property). Returned value has type SchemaItem or undefined if item is not found.
		QJSValue findSchemaItem(QString objectName);

	private:
		QString schemaId() const;
		QString caption() const;

		QColor backgroundColor() const ;
		void setBackgroundColor(QColor value);

		int layerCount() const;

	private:
		std::shared_ptr<Schema> m_schema;
	};


	//
	//	Schema
	//
	class Schema :
		public PropertyObject,
		public Proto::ObjectSerialization<Schema>,
		public DebugInstCounter<Schema>,
		public std::enable_shared_from_this<Schema>
	{
		Q_OBJECT

	protected:
		Schema(void);

	public:
		virtual ~Schema(void);

		void Init(void);

		// Serialization
		//
		friend Proto::ObjectSerialization<Schema>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		static std::shared_ptr<Schema> CreateObject(const Proto::Envelope& message);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const QRectF& clipRect);

		int GetDocumentWidth(double DpiX, double zoom) const;
		int GetDocumentHeight(double DpiY, double zoom) const;

		void BuildFblConnectionMap() const;

		bool updateAllSchemaItemFbs(const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemUfb(const std::vector<std::shared_ptr<UfbSchema>>& ufbs, int* updatedItemCount, QString* errorMessage);
		bool updateAllSchemaItemBusses(const std::vector<AppSignalLib::Bus>& busses, int* updatedItemCount, QString* errorMessage);

		QStringList getSignalList() const;
		virtual QStringList getLabels() const;
		virtual std::vector<QUuid> getGuids() const;

		/// Get all schema items tags
		///
		QStringList itemTags() const;

		virtual QString details(const QString& path) const;				// form details JSON object (signal list)

		std::shared_ptr<SchemaItem> getItemById(const QUuid& id) const;

		template<typename SchemaItemType>
		bool hasSchemaItemType() const;

		// Scripting
		//
	public:
		bool preDrawEvent(QJSEngine* engine);
		bool onShowEvent(QJSEngine* engine, ILogFile* log);

	protected:
		bool runScript(QJSValue& evaluatedJs, QJSEngine* engine);
		QJSValue evaluateScript(QString script, QJSEngine* engine, QWidget* parentWidget) const;
		QString formatSqriptError(const QJSValue& scriptValue) const;
		void reportSqriptError(const QJSValue& scriptValue, QWidget* parent) const;

		void drawScriptError(CDrawParam* drawParam) const;

		// Layers
		//
	public:
		// Do not change to value semantic, as ITERATORS from this getter are used in algs
		//
		const std::vector<std::shared_ptr<SchemaLayer>>& layers() const;

		[[nodiscard]] int activeLayerIndex() const;
		[[nodiscard]] QUuid activeLayerGuid() const;
		[[nodiscard]] std::shared_ptr<VFrame30::SchemaLayer> activeLayer() const;
		void setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer);

	protected:
		void clearLayers();
		void addLayer(std::shared_ptr<SchemaLayer> layer);

		// Properties and Data
		//
	public:
		[[nodiscard]] QUuid guid() const;
		void setGuid(const QUuid& guid);

		[[nodiscard]] QString schemaId() const;
		void setSchemaId(const QString& id);

		[[nodiscard]] QString caption() const;
		void setCaption(const QString& caption);

		[[nodiscard]] QString tagsAsString() const;
		[[nodiscard]] QStringList tagsAsList() const;

		void setTags(QString tags);
		void setTagsList(const QStringList& tags);

		[[nodiscard]] bool joinHorzPriority() const;
		void setJoinHorzPriority(bool value);

		[[nodiscard]] QString joinLeftSchemaId() const;
		void setJoinLeftSchemaId(const QString& value);

		[[nodiscard]] QString joinTopSchemaId() const;
		void setJoinTopSchemaId(const QString& value);

		[[nodiscard]] QString joinRightSchemaId() const;
		void setJoinRightSchemaId(const QString& value);

		[[nodiscard]] QString joinBottomSchemaId() const;
		void setJoinBottomSchemaId(const QString& value);

		[[nodiscard]] double docWidth() const;
		void setDocWidth(double width);

		[[nodiscard]] double docWidthRegional() const;
		void setDocWidthRegional(double width);

		[[nodiscard]] double docHeight() const;
		void setDocHeight(double height);

		[[nodiscard]] double docHeightRegional() const;
		void setDocHeightRegional(double height);

		[[nodiscard]] SchemaUnit unit() const;
		void setUnit(SchemaUnit value);

		[[nodiscard]] double gridSize() const;
		void setGridSize(double value);

		[[nodiscard]] int pinGridStep() const;
		void setPinGridStep(int value);

		[[nodiscard]] bool excludeFromBuild() const;
		void setExcludeFromBuild(bool value);

		[[nodiscard]] QColor backgroundColor() const;
		void setBackgroundColor(const QColor& value);

		[[nodiscard]] bool isLogicSchema() const;
		[[nodiscard]] bool isUfbSchema() const;
		[[nodiscard]] bool isMonitorSchema() const;
		[[nodiscard]] bool isTuningSchema() const;
		[[nodiscard]] bool isDiagSchema() const;
		[[nodiscard]] bool isVduSchema() const;

		[[nodiscard]] LogicSchema* toLogicSchema();
		[[nodiscard]] const LogicSchema* toLogicSchema() const;

		[[nodiscard]] UfbSchema* toUfbSchema();
		[[nodiscard]] const UfbSchema* toUfbSchema() const;

		[[nodiscard]] int changeset() const;
		void setChangeset(int value);

		[[nodiscard]] QString preDrawScript() const;
		void setPreDrawScript(QString value);

		[[nodiscard]] QString onShowScript() const;
		void setOnShowScript(QString value);

		const std::shared_ptr<VFrame30::Context>& context() const;
		void setContext(std::shared_ptr<VFrame30::Context> context);

	private:
		QUuid m_guid;
		QString m_schemaID;
		QString m_caption;

		std::vector<std::shared_ptr<SchemaLayer>> m_layers;

		QStringList m_tags;

		bool m_joinHorzPriority = false;
		QString m_joinLeftSchemaId;
		QString m_joinTopSchemaId;
		QString m_joinRightSchemaId;
		QString m_joinBottomSchemaId;

		double m_width = 0.0;					// pixels or inches, depends on m_unit
		double m_height = 0.0;					// pixels or inches, depends on m_unit

		SchemaUnit m_unit = SchemaUnit::Inch;

		int m_activeLayer = 0;
		double m_gridSize = 1.0;				// Grid size for this schema, depends on SchemaUnit
		int m_pinGridStep = 2;					// Grid multiplier to determine vertical distance between pins

		bool m_excludeFromBuild = false;		// Exclude Schema from build or any other processing

		QColor m_backgroundColor = {qRgb(0xFF, 0xFF, 0xFF)};

		int m_changeset = -1;					// Changeset, this field is not stored in file

		// --
		//
		QString m_preDrawScript;
		QString m_onShowScript;

		// Online stuff
		//
	private:
		std::shared_ptr<VFrame30::Context> m_context;

		mutable QJSValue m_jsPreDrawScript;				// Evaluated m_preDrawScript
		mutable size_t m_evaluatedPreDrawScript = 0;	//

		mutable QJSValue m_jsOnShowScript;				// Evaluated m_OnShowScript
		mutable size_t m_evaluatedOnShowScript = 0;

		mutable QString m_lastScriptError;
	};
} // namespace VFrame30



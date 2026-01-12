"use strict";

module VFrame30 {
	export interface QImage {
	}

	export interface QColor {
	}

	export interface TuningValue {
	}

	export interface QWidget {
	}

	export interface ScriptSchemaView {

		// Properties
		//
		schemaID: string;
		schemaCaption: string;
		schema: Schema;
		schemaCount: number;
		zoomFactor: number;
		highlightRectColor: QColor;

		// Functions
		//
		debugOutput(str: string): string;

		// Schema displaying functions
		//
		setSchema(schemaId: string): void;

		update(): void;		// Update (redraw) schema view

		// Schema items and widgets functions
		//
		findSchemaItem(objectName: string): SchemaItem;
		findWidget(objectName: string): QWidget;

		// Timer functions
		//
		startTimer(intervalMs: number, timerId: string): void;	// Starts or restarts the timer with a timeout of duration intervalMs milliseconds.
		// intervalMs Timer interval, milliseconds.
		// timerId Timer identifier, string.
		killTimer(timerId: string): void;	// Kills the timer with timer identifier.
		killAllTimers(): void;				//Kills all active timers for the current schema tab.

		// History functions
		//
		canBackHistory(): boolean;
		canForwardHistory(): boolean;

		historyBack(): void;
		historyForward(): void;

		// Message Box functons
		//
		warningMessageBox(text: string, details?: string): void;
		errorMessageBox(text: string, details?: string): void;
		infoMessageBox(text: string, details?: string): void;
		questionMessageBox(text: string, details?: string): boolean;
		messageBox(text: string, buttons: QMessageBox.StandardButton, defaultButton:QMessageBox.StandardButton, icon: QMessageBox.Icon, details?: string): number;

		// Variable functions
		//
		variableExists(name: string): boolean;

		variable(name: string): string;
		setVariable(name: string, value: number): void;
		setVariable(name: string, value: string): void;

		// Schemas info functions
		//
		schemaByIndex(schemaIndex: number): Schema;

		schemaIdByIndex(schemaIndex: number): string;

		schemaCaptionById(schemaId: string): string;
		schemaCaptionByIndex(schemaIndex: number): string;

		// Adds an item to be highlighted with a rectangle (Ctrl + left click)..
		//
		void addHighlightItem(SchemaItem item);

		// Clears all highlighted items.
		//
		void clearHighlightItems();
	}

	export interface Schema {
		// Properties
		//
		schemaID: string;
		caption: string;
		backgroundColor: QColor;
		layerCount: number;

		// Functions
		//
		isLogicSchema(): boolean;
		isUfbSchema(): boolean;
		isMonitorSchema(): boolean;
		isTuningSchema(): boolean;
		isDiagSchema(): boolean;

		layer(index: number): SchemaLayer;
		layer(caption: string): SchemaLayer;
		itemsByTag(tag: string): Array<SchemaItem>;
		findSchemaItem(objectName: string): SchemaItem;
	}

	export interface SchemaLayer {
		caption: string;
		visible: boolean;
	}

	export interface SchemaItem {
		// Properties
		//

		/** Turns on *ClickScript* script call when user clicks mouse button on schema item.
		  When this property is set to true, *ClickScript* event handler is called when user clicks left mouse button on a schema item.
		  Warning: This property has no effect on SchemaItemPushButton and SchemaItemLineEdit.*/
		acceptClick: boolean;

		/** Value of this property is inverted approximately each 250 milliseconds.*/
		blinkPhase: boolean;

		/** Automatically generated unique label associated with SchemaItem.*/
		label: string;

		/** Object name.*/
		objectName: string;

		/** A script to run before each schema redraw event.*/
		preDrawScript: string;

		/** SchemaItem's tags.*/
		tags: Array<string>;

		/** Item's type as a string, e.g. SchemaItemInput, SchemaItemUfb, SchemaItemAfb, SchemaItemLine, etc.*/
		type: string;

		/** Show or hide schema item in client Software (Monitor, TuningClient, etc).*/
		visible: boolean;

		// Functions
		//

		/** Check if SchemaItem has specified tag. There is an implicit tag that is the type of the item, e.g. SchemaItemInput, SchemaItemUfb, SchemaItemAfb, SchemaItemLine, etc...*/
		hasTag(tag: string): boolean;

		propertyValue(name: string): any;
		setPropertyValue(name: string, value: any): void;
	}

	export interface PosLineImpl extends SchemaItem {
		top: number;
		left: number;
		width: number;
		height: number;
	}

	export interface PosRectImpl extends SchemaItem {
		top: number;
		left: number;
		width: number;
		height: number;
	}

	export interface PosConnectionImpl extends SchemaItem {
	}

	export interface SchemaItemLine extends PosLineImpl {
		lineWeight: number;
		lineColor: QColor;
	}

	export interface SchemaItemPath extends PosConnectionImpl {
		lineWeight: number;
		lineColor: QColor;
	}

	export interface SchemaItemRect extends PosRectImpl {
		lineWeight: number;
		lineColor: QColor;

		fillColor: QColor;
		fill: boolean;

		drawRect: boolean;

		lineStyle: E.LineStyle;

		textFormat: E.TextFormat;
		textColor: QColor;
		text: string;

		wordWrap: boolean;

		alignHorz: E.HorzAlign;
		alignVert: E.VertAlign;

		fontName: string;
		fontSize: number;
		fontBold: boolean;
		fontItalic: boolean;
	}

	export interface SchemaItemValue extends PosRectImpl {
		signalIDs: Array<string>;
		appSignalIDs: Array<string>;

		lineWeight: number;
		lineColor: QColor;

		fillColor: QColor;
		textColor: QColor;

		drawRect: boolean;

		alignHorz: E.HorzAlign;
		alignVert: E.VertAlign;

		fontName: string;
		fontSize: number;
		fontBold: boolean;
		fontItalic: boolean;

		text: string;

		precision: number;
	}

	export interface SchemaItemSignal extends PosRectImpl {
		// Properties
		//
		signalIDs: Array<string>;
		appSignalIDs: Array<string>;
		impactSignalIDs: Array<string>;
		impactAppSignalIDs: Array<string>;

		customText: string;

		multiLine: boolean;

		precision: number;

		analogFormat: E.AnalogFormatCode;

		columnCount: number;

		cellColumnCount: number;

		cellRowCount: number;

		// Functions
		//
		columnWidth(columnIndex: number): number;
		setColumnWidth(value: number, columnIndex: number): void;

		columnData(columnIndex: number): E.ColumnData;
		setColumnData(value: E.ColumnData, columnIndex: number): void;

		columnHorzAlign(columnIndex: number): E.HorzAlign;
		setColumnHorzAlign(value: E.HorzAlign, columnIndex: number): void;

		resetCell(row: number, column: number): void;
		resetCellText(row: number, column: number): void;
		resetCellFillColor(row: number, column: number): void;
		resetCellTextColor(row: number, column: number): void;

		cellData(row: number, column: number): E.ColumnData;

		cellAppSignalID(row: number, column: number): string;

		cellText(row: number, column: number): string;
		setCellText(row: number, column: number, text: string): void;

		cellFillColor(row: number, column: number): QColor;
		setCellFillColor(row: number, column: number, color: QColor): void;

		cellTextColor(row: number, column: number): QColor;
		setCellTextColor(row: number, column: number, color: QColor): void;
	}

	export interface SchemaItemImage extends PosRectImpl {
		allowScale: boolean;
		keepAspectRatio: boolean;
		image: QImage;
		svg: string;
	}

	export interface ScriptImageItem {
		allowScale: boolean;
		keepAspectRatio: boolean;
		imageId: string;
		svg: string;
	}

	export interface SchemaItemImageValue extends PosRectImpl {
		// Properties
		//
		signalIDs: Array<string>;
		appSignalIDs: Array<string>;

		currentImageID: string;

		lineWeight: number;

		drawRect: boolean;

		lineColor: QColor;

		fill: boolean;
		fillColor: QColor;

		// Functions
		//
		imageItem(imageId: string): ScriptImageItem;
	}

	export interface SchemaItemControl extends SchemaItem {
	}

	export interface SchemaItemPushButton extends SchemaItemControl {
		widget: PushButtonWidget;
	}

	export interface SchemaItemLineEdit extends SchemaItemControl {
		widget: LineEditWidget;
	}

	export interface SchemaItemSlider extends SchemaItemControl {
		widget: SliderWidget;
	}

	export interface PushButtonWidget extends QWidget {
		text: string;

		autoRepeat: boolean;
		autoRepeatDelay: number;
		autoRepeatInterval: number;

		checkable: boolean;
		checked: boolean

		down: boolean;
		enabled: boolean;

		styleSheet: string;

		toolTip: string;
		toolTipDuration: number;
	}

	export interface LineEditWidget extends QWidget {
		text: string;

		alignment: E.Alignment;

		maxLength: number;

		readOnly: boolean;
		enabled: boolean;

		placeholderText: string;

		styleSheet: string;

		toolTip: string;
		toolTipDuration: number;

		modified: boolean;
	}

	export interface SliderWidget extends QWidget {
		orientation: Qt.Orientation;

		invertedAppearance: boolean;	/**< This property holds whether or not a slider shows its values inverted.*/
		invertedControls: boolean; 		/**< This property holds whether or not the slider inverts its wheel and key events.*/

		maximum: number;				/**< This property holds the slider's maximum signed integer value.*/
		minimum: number;				/**< This property holds the slider's minimum signed integer value.*/

		pageStep: number;				/**< This property holds the page step.*/
		singleStep: number;				/**< This property holds the single step.*/

		tracking: boolean; 				/**< This property holds whether slider tracking is enabled.*/

		tickInterval: number;			/**< This property holds the interval between tickmarks.*/
		tickPosition: QSlider.TickPosition;	/**< This property holds the tickmark position for this slider.*/

		value: number;					/**< This property holds the slider's current signed integer value.*/
	}
}

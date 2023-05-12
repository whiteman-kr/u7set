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

		// Functions
		//
		debugOutput(str: string): string;

		// Schema displaying functions
		//
		setSchema(schemaId: string): void;

		// Schema items and widgets functions
		//
		findSchemaItem(objectName: string): SchemaItem;
		findWidget(objectName: string): QWidget;

		// History functions
		//
		canBackHistory(): boolean;
		canForwardHistory(): boolean;

		historyBack(): void;
		historyForward(): void;

		// Message Box functons
		//
		warningMessageBox(text: string, details? : string): void;
		errorMessageBox(text: string, details? : string): void;
		infoMessageBox(text: string, details? : string): void;
		questionMessageBox(text: string, details? : string): boolean;

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
		objectName: string;
		acceptClick: boolean;
		preDrawScript: string;
		blinkPhase: boolean;
		visible: boolean;
		tags: Array<string>;
		type: string;

		// Functions
		//
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

	export interface SchemaItemRect extends SchemaItem {
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

	export interface SchemaItemSignal extends SchemaItem {
		// Properties
		//
		signalIDs: Array<string>;
		appSignalIDs: Array<string>;
		impactSignalIDs: Array<string>;
		impactAppSignalIDs: Array<string>;

		customText: string;

		multiLine: boolean;

		precision: number;

		analogFormat: E.AnalogFormat;

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
		// Properties
		//
		widget: QWidget;
	}

	export interface SchemaItemPushButton extends SchemaItemControl {
	}

	export interface SchemaItemLineEdit extends SchemaItemControl {
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
	}

}
"use strict"

// Description of class VFrame30::FblItemRect
//
interface FblItemRect {
	 adjustHeight() : void;
}

// Description of class VFrame30::SchemaItemAfb
//
interface SchemaItemAfb extends FblItemRect
{
	// Methods
	//
	setAfbParamByOpName(opName : string, value : (string | number | boolean)) : boolean;

	getParamIntValue(name : string) : number;
	getParamBoolValue(name : string) : boolean;

	addInputSignal(caption : string, type : number, opIndex : number, size : number) : void;
	addOutputSignal(caption : string, type : number, opIndex : number, size : number) : void;
	
	removeInputSignals() : void;
	removeOutputSignals() : void;
}

// Description of class Afb::AfbSignal
//
interface AfbSignal
{	
	jsCaption() : string;
	jsType() : number;
	operandIndex() : number;
	size() : number;
}

// Description of class Afb::AfbElement
//
interface AfbElement
{
	getAfbSignalByOpIndex(opIndex : number) : AfbSignal;
	getAfbSignalByCaption(signalCaption : string) : AfbSignal;
}



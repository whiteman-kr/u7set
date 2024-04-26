"use strict";

// Global objects declarations

declare var view: VFrame30.ScriptSchemaView;
declare var signals: Signal.AppSignalController;
declare var tuning: Signal.TuningController;
declare var log: Log.LogController;

//declare var app: App.ScriptMonitorApplication;		// For Monitor script
//declare var equipment: Hardware.ScriptEquipment;		// For Monitor script
//declare var app: App.ScriptTuningClientApplication;	// For TuningClient script

// Global functions

function showSignalValue(schemaItem: VFrame30.SchemaItemValue, signalId: string) {

	schemaItem.alignHorz = E.HorzAlign.AlignHCenter;
	schemaItem.alignVert = E.VertAlign.AlignVCenter;

	if (signals.signalExists(signalId) == false)
	{
		schemaItem.text = "Signal not found!";
		return;
	}

	let state: Signal.AppSignalState = signals.signalState(signalId);

	if (state == null)
	{
		schemaItem.text = "State not found!";
		return;
	}

	if (state.valid == true)
	{
		schemaItem.text = state.value.toString();
	}
	else
	{
		schemaItem.text = "???";
	}

	return;
}

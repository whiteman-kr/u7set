"use strict";

module Signal {
	export interface TuningValue {
	}

	export interface AppSignalParam {
		// Properties
		//
		hash: number;

		appSignalID: string;
		customAppSignalID: string;
		customSignalID: string;
		caption: string;
		equipmentID: string;
		lmEquipmentID: string;

		unit: string;

		precision: number;

		channel: E.Channel;

		lowValidRange: number;
		highValidRange: number;

		lowEngineeringUnits: number;
		highEngineeringUnits: number;

		enableTuning: boolean;

		tuningDefaultValue: TuningValue;
		tuningLowBound: TuningValue;
		tuningHighBound: TuningValue;

		isInput: boolean;
		isOutput: boolean;

		isInternal: boolean;

		isAnalog: boolean;
		isDiscrete: boolean;

		isEndpoint: boolean;

		// Functions
		//
		hasTag(tag: string): boolean;
	}

	export interface AppSignalState {
		value: number;

		valid: boolean;
		stateAvailable: boolean;
		simulated: boolean;
		blocked: boolean;
		mismatch: boolean;
		aboveHighLimit: boolean;
		belowLowLimit: boolean;
		outOfLimits: boolean;
		tuningDefault: boolean;
	}

	export interface TuningSignalState {
		value: number;

		lowBound: TuningValue;
		highBound: TuningValue;

		valid: boolean;
		outOfRange: boolean;
		writeInProgress: boolean;
		controlIsEnabled: boolean;
		writingIsEnabled: boolean;
		tuningDefault: boolean;
	}

	export interface AppSignalController {
		signalsCount(): number;

		signalParam(signalId: string): AppSignalParam;
		signalParam(signalHash: number): AppSignalParam;

		equipmentToAppSignalId(equipmentId: string): string;

		signalState(signalId: string): AppSignalState;
		signalState(signalHash: number): AppSignalState;
		signalStates(signalIds : Array<string>) : Array<AppSignalState>;

		signalExists(signalId: string): boolean;
		signalsExist(signalIds: Array<string>) : boolean;

		isDiscrete(signalId: string): boolean;
		isAnalog(signalId: string): boolean;

		precision(signalId: string): number;

		signalIdsByTag(tag: string): Array<string>;
	}

	export interface TuningController {
		signalParam(appSignalId: string): AppSignalParam;
		signalState(appSignalId: string): TuningSignalState;
		signalStates(appSignalIds : Array<string>) : Array<TuningSignalState>;

		signalExists(appSignalId: string): boolean;
		signalsExist(appSignalIds: Array<string>) : boolean;

		isDiscrete(appSignalId: string): boolean;
		isAnalog(appSignalId: string): boolean;

		precision(appSignalId: string): number;

		signalIdsByTag(tag: string): Array<string>;

		writeValue(appSignalId: string, value: number): boolean;
		apply(): void;
		
		isLoggedIn(): boolean;
	}
}
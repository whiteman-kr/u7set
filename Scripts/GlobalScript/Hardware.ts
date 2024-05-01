"use strict";

module Hardware
{
    export interface ScriptDeviceObject
    {
        // Properties
        //
        equipmentId: string;
        caption: string;
        uuid: any;
        deviceType: E.DeviceType;
        place: number;
        childrenCount: number;

        // Functions
        //
        parent(): ScriptDeviceObject;
        child(index: number): ScriptDeviceObject;

        childByEquipmentId(id: string): ScriptDeviceObject;

        toSystem(): ScriptDeviceSystem;
        toRack(): ScriptDeviceRack;
        toChassis(): ScriptDeviceChassis;
        toModule(): ScriptDeviceModule;
        toController(): ScriptDeviceController;
        toWorkstation(): ScriptDeviceWorkstation;
        toSoftware(): ScriptDeviceSoftware;
        toAppSignal(): ScriptDeviceAppSignal;

        isRoot(): boolean;
        isSystem(): boolean;
        isRack(): boolean;
        isChassis(): boolean;
        isModule(): boolean;
        isController(): boolean;
        isWorkstation(): boolean;
        isSoftware(): boolean;
        isAppSignal(): boolean;

        propertyValue(caption: string): any;

        propertyInt(caption: string): number;
        propertyBool(caption: string): boolean;
        propertyString(caption: string): string;
        propertyIP(caption: string): number;
    };

    //
    // System
    //
    export interface ScriptDeviceSystem extends ScriptDeviceObject
    {
    };

    //
    // Rack
    //
    export interface ScriptDeviceRack extends ScriptDeviceObject
    {
    };

    //
    // Chassis
    //
    export interface ScriptDeviceChassis extends ScriptDeviceObject
    {
    };

    //
    // Module
    //
    export interface ScriptDeviceModule extends ScriptDeviceObject
    {
        // Properties
        //
        moduleFamily: number;
        customModuleFamily: number;
        moduleVersion: number;
    };

    //
    // Controller
    //
    export interface ScriptDeviceController extends ScriptDeviceObject
    {
    };

    //
    // Workstation
    //
    export interface ScriptDeviceWorkstation extends ScriptDeviceObject
    {
    };

    //
    // Software
    //
    export interface ScriptDeviceSoftware extends ScriptDeviceObject
    {
        softwareType: E.SoftwareType;
    };

    //
    // AppSignal
    //
    export interface ScriptDeviceAppSignal extends ScriptDeviceObject
    {
    };

    //
    // ScriptEquipment
    //
    export interface ScriptEquipment
    {
        root(): ScriptDeviceObject;
        find(equipmentId: string): ScriptDeviceObject;
        deviceProperty(equipmentId: string, propertyName: string): any;
    }
}
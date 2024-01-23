"use strict";

module Qt {
    export enum Orientation {
        Horizontal = 1,
        Vertical = 2
    }
}

module QMessageBox{
    export enum Icon {
        NoIcon = 0,
        Information = 1,
        Warning = 2,
        Critical = 3,
        Question = 4
    }
    export enum StandardButton {
        NoButton           = 0x00000000,
        Ok                 = 0x00000400,
        Save               = 0x00000800,
        SaveAll            = 0x00001000,
        Open               = 0x00002000,
        Yes                = 0x00004000,
        YesToAll           = 0x00008000,
        No                 = 0x00010000,
        NoToAll            = 0x00020000,
        Abort              = 0x00040000,
        Retry              = 0x00080000,
        Ignore             = 0x00100000,
        Close              = 0x00200000,
        Cancel             = 0x00400000,
        Discard            = 0x00800000,
        Help               = 0x01000000,
        Apply              = 0x02000000,
        Reset              = 0x04000000,
        RestoreDefaults    = 0x08000000
    }
}

module QSlider {
    export enum TickPosition {
        NoTicks = 0,             /**< Do not draw any tick marks.*/
        TicksBothSides = 3,      /**< Draw tick marks on both sides of the groove.*/
        TicksAbove = 1,          /**< Draw tick marks above the (horizontal) slider.*/
        TicksBelow = 2,          /**< Draw tick marks below the (horizontal) slider.*/
        TicksLeft = TicksAbove,  /**< Draw tick marks to the left of the (vertical) slider.*/
        TicksRight = TicksBelow  /**< Draw tick marks to the right of the (vertical) slider.*/
    }
}
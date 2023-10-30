"use strict";

module Qt {
    export enum Orientation {
        Horizontal = 1,
        Vertical = 2
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
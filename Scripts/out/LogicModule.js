///
/// #add_fp_v1
///
var add_fp_v1;
(function (add_fp_v1) {
    function createOutput(schemaItemAfb, afbElement, paramName, signalCaption) {
        let show = schemaItemAfb.getParamBoolValue(paramName);
        if (show == true) {
            var s = afbElement.getAfbSignalByCaption(signalCaption);
            if (s != null) {
                schemaItemAfb.addOutputSignal(s.jsCaption(), s.jsType(), s.operandIndex(), s.size());
            }
        }
    }
    function createOutputSignals(schemaItemAfb, afbElement) {
        schemaItemAfb.removeOutputSignals();
        var out = afbElement.getAfbSignalByCaption("out");
        if (out != null) {
            schemaItemAfb.addOutputSignal(out.jsCaption(), out.jsType(), out.operandIndex(), out.size());
        }
        createOutput(schemaItemAfb, afbElement, "OutOverflow", "overflow");
        createOutput(schemaItemAfb, afbElement, "OutUnderflow", "underflow");
        createOutput(schemaItemAfb, afbElement, "OutZero", "zero");
        createOutput(schemaItemAfb, afbElement, "OutNaN", "nan");
        schemaItemAfb.adjustHeight();
        return true;
    }
})(add_fp_v1 || (add_fp_v1 = {}));
///
/// #add_si_v1
///
var add_si_v1;
(function (add_si_v1) {
    function createOutput(schemaItemAfb, afbElement, paramName, signalCaption) {
        var show = schemaItemAfb.getParamBoolValue(paramName);
        if (show == true) {
            var s = afbElement.getAfbSignalByCaption(signalCaption);
            if (s != null) {
                schemaItemAfb.addOutputSignal(s.jsCaption(), s.jsType(), s.operandIndex(), s.size());
            }
        }
    }
    function createOutputSignals(schemaItemAfb, afbElement) {
        schemaItemAfb.removeOutputSignals();
        var out = afbElement.getAfbSignalByCaption("out");
        if (out != null) {
            schemaItemAfb.addOutputSignal(out.jsCaption(), out.jsType(), out.operandIndex(), out.size());
        }
        createOutput(schemaItemAfb, afbElement, "OutOverflow", "overflow");
        createOutput(schemaItemAfb, afbElement, "OutZero", "zero");
        schemaItemAfb.adjustHeight();
        return true;
    }
})(add_si_v1 || (add_si_v1 = {}));
///
/// #and_v1
///
var and_v1;
(function (and_v1) {
    function createInputSignals(schemaItemAfb, afbElement) {
        schemaItemAfb.removeInputSignals();
        var operandCount = schemaItemAfb.getParamIntValue("OperandCount");
        if (operandCount == -1) {
            return false;
        }
        operandCount = Math.max(operandCount, 2);
        operandCount = Math.min(operandCount, 16);
        var opIndex = 3; // 3 is ???
        for (; operandCount != 0; operandCount--) {
            var afbInputSignal = afbElement.getAfbSignalByOpIndex(opIndex);
            if (afbInputSignal != null) {
                var caption = afbInputSignal.jsCaption();
                var type = afbInputSignal.jsType();
                var operandIndex = afbInputSignal.operandIndex();
                var size = afbInputSignal.size();
                schemaItemAfb.addInputSignal(caption, type, operandIndex, size);
            }
            opIndex++;
        }
        schemaItemAfb.adjustHeight();
        return true;
    }
})(and_v1 || (and_v1 = {}));

///
/// #add_fp_v1
///
namespace add_fp_v1
{
    function createOutput(schemaItemAfb : SchemaItemAfb, afbElement : AfbElement, paramName : string, signalCaption : string)
    {
        let show : boolean = schemaItemAfb.getParamBoolValue(paramName);
        if (show == true)
        {
            var s : AfbSignal = afbElement.getAfbSignalByCaption(signalCaption);
            if (s != null)
            {
                schemaItemAfb.addOutputSignal(s.jsCaption(), s.jsType(), s.operandIndex(), s.size());
            }
        }
    }

    function createOutputSignals(schemaItemAfb : SchemaItemAfb, afbElement : AfbElement) : boolean
    {
        schemaItemAfb.removeOutputSignals();

        var out : AfbSignal = afbElement.getAfbSignalByCaption("out");
        if (out != null)
        {
            schemaItemAfb.addOutputSignal(out.jsCaption(), out.jsType(), out.operandIndex(), out.size());
        }

        createOutput(schemaItemAfb, afbElement, "OutOverflow", "overflow");
        createOutput(schemaItemAfb, afbElement, "OutUnderflow", "underflow");
        createOutput(schemaItemAfb, afbElement, "OutZero", "zero");
        createOutput(schemaItemAfb, afbElement, "OutNaN", "nan");

        schemaItemAfb.adjustHeight();

        return true;
    }
}

///
/// #add_si_v1
///
namespace add_si_v1
{
    function createOutput(schemaItemAfb : SchemaItemAfb, afbElement : AfbElement, paramName : string, signalCaption : string)
    {
        var show : boolean = schemaItemAfb.getParamBoolValue(paramName);
        if (show == true)
        {
            var s : AfbSignal = afbElement.getAfbSignalByCaption(signalCaption);
            if (s != null)
            {
                schemaItemAfb.addOutputSignal(s.jsCaption(), s.jsType(), s.operandIndex(), s.size());
            }
        }
    }

    function createOutputSignals(schemaItemAfb : SchemaItemAfb, afbElement : AfbElement) : boolean
    {
        schemaItemAfb.removeOutputSignals();

        var out : AfbSignal = afbElement.getAfbSignalByCaption("out");
        if (out != null)
        {
            schemaItemAfb.addOutputSignal(out.jsCaption(), out.jsType(), out.operandIndex(), out.size());
        }

        createOutput(schemaItemAfb, afbElement, "OutOverflow", "overflow");
        createOutput(schemaItemAfb, afbElement, "OutZero", "zero");

        schemaItemAfb.adjustHeight();

        return true;
    }
}

///
/// #and_v1
///
namespace and_v1
{
    function createInputSignals(schemaItemAfb : SchemaItemAfb, afbElement : AfbElement) : boolean
    {
        schemaItemAfb.removeInputSignals();

        var operandCount : number = schemaItemAfb.getParamIntValue("OperandCount");
        if (operandCount == -1)
        {
            return false;
        }
    
        operandCount = Math.max(operandCount, 2);
        operandCount = Math.min(operandCount, 16);

        var opIndex = 3;        // 3 is ???
        for (; operandCount != 0; operandCount--)
        {
            var afbInputSignal : AfbSignal = afbElement.getAfbSignalByOpIndex(opIndex);
            if (afbInputSignal != null)
            {
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
}
CREATE OR REPLACE FUNCTION hascheckedoutsignals()
  RETURNS boolean AS
$BODY$
DECLARE
    checkedOutSignalsCount integer;
BEGIN
    SELECT count(*) INTO checkedOutSignalsCount FROM Checkout WHERE SignalID IS NOT NULL;

    RETURN checkedOutSignalsCount <> 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

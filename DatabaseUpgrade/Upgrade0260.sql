-- RPCT-2250: Optimization of search operations in signalinstance table

CREATE INDEX equipmentid_index ON signalinstance (equipmentid);
CREATE INDEX customappsignalid_index ON signalinstance (customappsignalid);
CREATE INDEX appsignalid_index ON signalinstance (appsignalid);

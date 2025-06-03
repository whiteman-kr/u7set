#pragma once

class TuningSourceStateModel : public QAbstractTableModel
{
public:
	TuningSourceStateModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void updateData(const Network::TuningSourceInfoState& state);

private:
	void updateValueTime(int row, qint64 value);
	bool valueChanged(int row, bool receivesData) const;

	void onTimer1s();

private:
	Network::TuningSourceInfoState m_state;

	std::vector<std::pair<qint64, qint64>> m_valueTime;

	QTimer m_timer1s;
	qint64 m_curTime = 0;

	inline static const QVariant m_cleanVariant;

	// Tuning Source channel identification
	//
	//
	optional bool isReply = 10 [default = false];
	optional int64 requestCount = 11 [default = 0];
	optional int64 replyCount = 12 [default = 0];
	optional int32 commandQueueSize = 13 [default = 0];
	optional bool controlIsActive = 14 [default = false];
	optional bool setSOR = 15 [default = false];
	optional bool writingDisabled = 16 [default = false];
	optional bool hasUnappliedParams = 17 [default = false];

	// Tuning Source processing errors
	//
	optional int64 errUntimelyReplay = 70 [default = 0];
	optional int64 errSent = 71 [default = 0];
	optional int64 errPartialSent = 72 [default = 0];
	optional int64 errReplySize = 73 [default = 0];
	optional int64 errNoReply = 74 [default = 0];
	optional int64 errTuningFrameUpdate = 75 [default = 0];

	// errors in reply RupFrameHeader
	//
	optional int64 errRupProtocolVersion = 50 [default = 0];
	optional int64 errRupFrameSize = 51 [default = 0];
	optional int64 errRupNonTuningData = 52 [default = 0];
	optional int64 errRupModuleType = 53 [default = 0];
	optional int64 errRupFramesQuantity = 54 [default = 0];
	optional int64 errRupFrameNumber = 55 [default = 0];
	optional int64 errRupCRC = 56 [default = 0];

	// errors in reply FotipHeader
	//
	optional int64 errFotipProtocolVersion = 60 [default = 0];
	optional int64 errFotipUniqueID = 61 [default = 0];
	optional int64 errFotipLmNumber = 62 [default = 0];
	optional int64 errFotipSubsystemCode = 63 [default = 0];
	optional int64 errFotipOperationCode = 64 [default = 0];
	optional int64 errFotipFrameSize = 65 [default = 0];
	optional int64 errFotipRomSize = 66 [default = 0];
	optional int64 errFotipRomFrameSize = 67 [default = 0];
	optional int64 errAnalogLowBoundCheck = 68 [default = 0];
	optional int64 errAnalogHighBoundCheck = 69 [default = 0];


	// flags reported by LM in reply FotipHeader.flags
	//
	optional int64 fotipFlagBoundsCheckSuccess = 30 [default = 0];
	optional int64 fotipFlagWriteSuccess = 31 [default = 0];
	optional int64 fotipFlagDataTypeErr = 32 [default = 0];
	optional int64 fotipFlagOpCodeErr = 33 [default = 0];
	optional int64 fotipFlagStartAddrErr = 34 [default = 0];
	optional int64 fotipFlagRomSizeErr = 35 [default = 0];
	optional int64 fotipFlagRomFrameSizeErr = 36 [default = 0];
	optional int64 fotipFlagFrameSizeErr = 37 [default = 0];
	optional int64 fotipFlagProtocolVersionErr = 38 [default = 0];
	optional int64 fotipFlagSubsystemKeyErr = 39 [default = 0];
	optional int64 fotipFlagUniueIDErr = 40 [default = 0];
	optional int64 fotipFlagOffsetErr = 41 [default = 0];
	optional int64 fotipFlagApplySuccess = 42 [default = 0];
	optional int64 fotipFlagSetSOR = 43 [default = 0];
	optional int64 fotipFlagWritingDisabled = 44 [default = 0];
	optional uint64 fotipProcessingNumerator = 45 [default = 0];

	inline static const std::vector<QString> m_rows =
	{
			QString("Source reply"),						// 0
			QString("LM time"),								// 1
			QString("Request count"),						// 2
			QString("Reply count"),							// 3
			QString("Control is active"),					// 4
			QString("Set SOR"),								// 5
			QString("Writing disabled"),					// 6
			QString("Has unapplied params"),				// 7
			QString("Received DataUID"),					// 8
			QString("Error RUP protocol version"),			// 9
			QString("Error frames quantity"),				// 10
			QString("Error frame No"),						// 11
			QString("Error frame CRC"),						// 12
			QString("Error DataUID"),						// 13
			QString("Error duplicate plant time"),			// 14
			QString("Error non-monotonic plant time"),		// 15
			QString("Error plant time format"),				// 16
	};

	inline static const int ROW_LOST_PACKET_COUINT = 7;
	inline static const int ROW_SS_QUEUE_CUR_SIZE = 8;
	inline static const int ROW_RECEIVED_DATA_UID = 10;
	inline static const int ROW_ERR_PROTOCOL_VERSION = 11;
	inline static const int ROW_ERR_FRAMES_QUANTITY = 12;
	inline static const int ROW_ERR_FRAME_NO = 13;
	inline static const int ROW_ERR_FRAME_CRC = 14;
	inline static const int ROW_ERR_DATA_UID = 15;
	inline static const int ROW_ERR_DUP_PLANT_TIME = 16;
	inline static const int ROW_ERR_NONMONO_PLANT_TIME = 17;
	inline static const int ROW_ERR_PLANT_TIME_FORMAT = 18;
};


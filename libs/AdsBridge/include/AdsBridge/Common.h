/**
 * @file Common.h
 * @brief Contains common definitions and structures used in the AdsBridge library.
 */

#ifndef ADSB_COMMON_H
#define ADSB_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/** @brief Log levels.
	 *
	 * The log levels are used to specify the severity of a log message.
	 */
	enum MatsLogLevel
	{
		LOG_LEVEL_DEBUG,   ///< Debug log level.
		LOG_LEVEL_WARNING, ///< Warning log level.
		LOG_LEVEL_ERROR    ///< Error log level.
	};

	/// @brief Connection results for MatsConnectionStatus.
	enum MatsConnectionResult
	{
		ADS_SET_CONNECTION_RESULT_UNDEFINED = 0,              ///< Undefined connection result.
		ADS_SET_CONNECTION_RESULT_OK = 1,                     ///< Connection successful.
		ADS_SET_CONNECTION_RESULT_UNKNOWN_CLIENT_ID = 2,      ///< Unknown client ID.
		ADS_SET_CONNECTION_RESULT_WRONG_CLIENT_HOST_NAME = 3, ///< Wrong client host name.
		ADS_SET_CONNECTION_RESULT_WRONG_SERVER_ID = 4         ///< Wrong server ID.
	};

	/**
	 * @brief Log handler function pointer type.
	 *
	 * This function pointer type is used to define a log handler that will be called to handle log messages.
	 * The log handler should be thread-safe.
	 *
	 * @param level The log level of the message.
	 * @param message The log message.
	 */
	typedef void (*MatsLogHandler)(enum MatsLogLevel level, const char* message);

	/**
	 * @struct AdsConnectionStatus
	 * @brief Represents the status of an AppDataService connection.
	 */
	struct AdsConnectionStatus
	{
		uintptr_t id;                                  ///< Internal connection identifier.
		bool status;                                   ///< Connection status.
		enum MatsConnectionResult setConnectionResult; ///< Connection result.
		const char* connectionType;                    ///< Type of the connection (states, recents, etc.).
		int port;                                      ///< Port number.
		const char* address;                           ///< IP address.
		const char* adsEquipmentId;                    ///< AppDataService EquipmentID.
		size_t received;                               ///< Number of received bytes.
		size_t sent;                                   ///< Number of sent bytes.
		size_t requestCount;                           ///< Number of requests.
		size_t replyCount;                             ///< Number of replies.
	};

	/**
	 * @brief Represents a timestamp. The timestamp is a 64-bit unsigned integer that represents the number of milliseconds since the epoch.
	 */
	typedef uint64_t MatsTimeStamp;

	/**
	 * @brief Represents a hash value of appSignalId.
	 */
	typedef uint64_t MatsSignalHash;

	/**
	 * @brief Channel types.
	 */
	enum MatsChannel
	{
		MATS_CHANNEL_A = 0, ///< Represents channel A.
		MATS_CHANNEL_B = 1, ///< Represents channel B.
		MATS_CHANNEL_C = 2, ///< Represents channel C.
		MATS_CHANNEL_D = 3  ///< Represents channel D.
	};

	/**
	 * @brief Signal input/output/internal types.
	 */
	enum MatsSignalInOutType
	{
		MATS_SIGNAL_INPUT = 0,   ///< Represents an input signal type.
		MATS_SIGNAL_OUTPUT = 1,  ///< Represents an output signal type.
		MATS_SIGNAL_INTERNAL = 2 ///< Represents an internal signal type.
	};

	/**
	 * @brief Signal types.
	 */
	enum MatsSignalType
	{
		MATS_SIGNAL_ANALOG,   ///< Represents an analog signal type.
		MATS_SIGNAL_DISCRETE, ///< Represents a discrete signal type.
		MATS_SIGNAL_BUS       /// < Represents a bus signal type.
	};

	// clang-format off
#define MATS_FLAG_VALID				0x00000001	///< Flag mask to indicate that the signal value is valid.
#define MATS_FLAG_STATE_AVAILABLE	0x00000002	///< Flag mask to indicate that the signal state is available (there is connection with the logic module).
#define MATS_FLAG_SIMULATED			0x00000004	///< Flag mask to indicate that the AFB sim_lock is applied with the specified flag.
#define MATS_FLAG_BLOCKED			0x00000008	///< Flag mask to indicate that the signal is blocked (AFB sim_lock is applied with the specified flag).
#define MATS_FLAG_MISMATCH			0x00000010	///< Flag mask to indicate that the signal value is mismatched.
#define MATS_FLAG_ABOVE_HIGH_LIMIT	0x00000020	///< Flag mask to indicate that the signal value is above the high limit.
#define MATS_FLAG_BELOW_LOW_LIMIT	0x00000040	///< Flag mask to indicate that the signal value is below the low limit.
#define MATS_FLAG_SW_SIMULATED		0x00000080	///< Flag mask to indicate that the signal is software simulated.
#define MATS_FLAG_TUNING_DEFAULT	0x00000100	///< Flag mask to indicate that the signal is tunable and current value is equal to the default tuning value.
	// clang-format on

	/**
	 * @struct MatsAppSignalParam
	 * @brief Represents the parameters of an application signal.
	 */
	struct MatsAppSignalParam
	{
		MatsSignalHash hash;                ///< The hash value of the signal.
		const char* appSignalId;            ///< The ID of the application signal.
		const char* customSignalId;         ///< The custom signal ID of the application signal.

		const char* caption;                ///< The caption of the signal.
		const char* equipmentId;            ///< The ID of the equipment (lmEquipmentId for internal signals).
		const char* lmEquipmentId;          ///< The ID of the LM equipment.
		const char* unit;                   ///< The unit of the signal.
		const char* tags;                   ///< Space separated tags associated with the signal.

		enum MatsChannel channel;           ///< The channel of the signal.
		enum MatsSignalInOutType inOutType; ///< The input/output/internal type of the signal.
		enum MatsSignalType type;           ///< The analog/discrete type of the signal.
		int decimalPlaces;                  ///< The number of decimal places for the signal value.

		double lowValidRange;               ///< The lower valid range of the signal.
		double highValidRange;              ///< The upper valid range of the signal.

		bool tuning;                        ///< Indicates if the signal is used for tuning.
	};

	/**
	 * @struct MatsAppSignalState
	 * @brief Represents the state of an application signal.
	 */
	struct MatsAppSignalState
	{
		MatsSignalHash hash;      ///< The hash from the appSignalId of the signal.
		MatsTimeStamp plantTime;  ///< The plant time of the acquired value.
		MatsTimeStamp serverTime; ///< The server system time (UTC+0) of the acquired value..

		double value;             ///< The value of the signal.
		uint32_t flags;           ///< The flags associated with the signal. Use MATS_FLAG_XXX.
	};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ADSB_COMMON_H
/*
Module title: Optical Signals Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Sends application signals by the optical interface.

-- *************************************************************
-- Version v1.00
-- Date:	13.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_opto initial update

-- *************************************************************
-- Version v1.01
-- Date:	29.04.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_opto module refactoring

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09.05.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs

-- *************************************************************

-- Version v1.03
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	25.09.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4148
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4148, rn_opto сorrecting errors identified by static analysis testing

-- *************************************************************

*/

#include "rn_opto.h"
#include "rn_sifp.h"
#include "rn_utils.h"
#include "rn_diag.h"

#include <string.h>
#include <stdio.h>

#include "defs.h"

// This function performs writing application signal values by optical interfaces
//
static void optoTxProcImpl(void)
{
	char msg[STRING_MAX_LEN];

	{
		alt_u16 count = optoPortsCount();
		for (alt_u16 i = 0; i < count; i++)
		{
			const struct VduOptoPort* optoPort = vduOptoPort(i);
			if (optoPort == NULL)
			{
				RN_ASSERT(optoPort != NULL);
				return;
			}

			if (optoPort->txDataSizeW != 0u)
			{
				alt_u16 offset = i * WORDS_PER_OPTO_CHANNEL;

				alt_u32 txId = optoPort->txDataUID;

				writeEBB16(offset, (alt_u16)(txId >> 16u));			// Write the high word of the data UID (shifted to right by 16 bits) to the offset
				writeEBB16(offset + 1u, (alt_u16)(txId & 0xffffu));	// Write the low word of the data UID (using 0xffff mask) to the offset plus one word
			}
		}
	}

	alt_u16 bitsUnion = 0;

	alt_u16 count = txAppSignalsCount();
	alt_u16 si = 0;
	while (si < count)
	{
		const struct VduTxRxAppSignal* const txRxSignal = txRxAppSignal(si);
		if (txRxSignal == NULL)
		{
			RN_ASSERT(txRxSignal != NULL);
			return;
		}

		if (txRxSignal->optoPortIndex >= optoPortsCount()) 
		{
			RN_ASSERT(txRxSignal->optoPortIndex < optoPortsCount());
			return;
		}

		if (txRxSignal->offsetW >= WORDS_PER_OPTO_CHANNEL)
		{
			RN_ASSERT(txRxSignal->offsetW < WORDS_PER_OPTO_CHANNEL);
			return;
		}

		alt_u16 offset = txRxSignal->offsetW + (txRxSignal->optoPortIndex * WORDS_PER_OPTO_CHANNEL);

		const struct VduAppSignalParam* const txRxSignalParam = appSignalParamByIndex(txRxSignal->signalIndex);
		if (txRxSignalParam == NULL)
		{
			RN_ASSERT(txRxSignalParam != NULL);
			return;
		}

		if (txRxSignalParam->vduSignalType == (alt_u16)Discrete)
		{
			// Process all discrete signals with this offset
			// It is assumed that bits in word of discretes are started from 0 without gaps
			//
			const struct VduTxRxAppSignal* txRxSignalDiscrete = txRxAppSignal(si);
			do
			{
				if (txRxSignalDiscrete == NULL)
				{
					RN_ASSERT(txRxSignalDiscrete != NULL);
					return;
				}

				alt_32 res = 0;

				if (readSignalValueIntByIndex(txRxSignalDiscrete->signalIndex, &res) == true)
				{
					bitsUnion &= ~(1u << (txRxSignalDiscrete->bitNo));
					bitsUnion |= ((alt_u16)res << (txRxSignalDiscrete->bitNo));
				}
				else
				{
					(void)snprintf(msg, STRING_MAX_LEN, "optoTxProc: can't read discrete signal by index = %d.", txRxSignalDiscrete->signalIndex);
					logWarning(SignalReadTypeWrong, msg);

					// Force the discrete to fail-safe (zero) state
					//
					bitsUnion &= ~(1u << (txRxSignalDiscrete->bitNo));
				}

				si++;

				// Check if there is more signals and the next signal has non-zero bit no
				//
				if (si < txAppSignalsCount())
				{
					txRxSignalDiscrete = txRxAppSignal(si);
					if (txRxSignalDiscrete == NULL)
					{
						RN_ASSERT(txRxSignalDiscrete != NULL);
						return;
					}
				}
			} while ((si < txAppSignalsCount()) && (txRxSignalDiscrete->bitNo != 0u));

			writeEBB16(offset, bitsUnion);

			bitsUnion = 0;
		}
		else if (txRxSignalParam->vduSignalType == (alt_u16)AnalogFloat32)
		{
			float res = 0.0f;

			if (readSignalValueFloatByIndex(txRxSignal->signalIndex, &res) == true)
			{
				alt_u32 floatRes = 0;
				(void)memcpy(&floatRes, &res, sizeof(res));	// float-to-alt_u32-to bit reinterpretation

				writeEBB16(offset, (alt_u16)(floatRes >> 16u));			// Write the high word of the data (shifted to right by 16 bits) to the offset
				writeEBB16(offset + 1u, (alt_u16)(floatRes & 0xffffu));	// Write the low word of the data (using 0xffff mask) to the offset plus one word

			}
			else
			{
				(void)snprintf(msg, STRING_MAX_LEN, "optoTxProc: can't read float signal by index = %d.", txRxSignal->signalIndex);
				logWarning(SignalReadTypeWrong, msg);
			}

			si++;
		}
		else if (txRxSignalParam->vduSignalType == (alt_u16)AnalogSignedInt32)
		{
			alt_32 res = 0;

			if (readSignalValueIntByIndex(txRxSignal->signalIndex, &res) == true)
			{
				alt_u32 intRes = 0;
				(void)memcpy(&intRes, &res, sizeof(res));	// alt_32-to-alt_u32-to bit reinterpretation

				writeEBB16(offset, (alt_u16)(intRes >> 16u));			// Write the high word of the data (shifted to right by 16 bits) to the offset
				writeEBB16(offset + 1u, (alt_u16)(intRes & 0xffffu));	// Write the low word of the data (using 0xffff mask) to the offset plus one word
			}
			else
			{
				(void)snprintf(msg, STRING_MAX_LEN, "optoTxProc: can't read integer signal by index = %d.", txRxSignal->signalIndex);
				logWarning(SignalReadTypeWrong, msg);
			}

			si++;
		}
		else
		{
			RN_ASSERT(false);
			si++;
		}
	}

	return;
}

// This function locks/unlocks the read and write memory and calls a procedure that writes
// application signal values by optical interfaces, works as a wrapper to automatically unlock the memory
//
void optoTxProc(void)
{
	externalBusBridgeReadLock();

	externalBusBridgeWriteLock();

	optoTxProcImpl();

	externalBusBridgeWriteUnlock();

	externalBusBridgeReadUnlock();
}

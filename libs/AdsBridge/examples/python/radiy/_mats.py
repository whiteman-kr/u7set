import enum


class MatsLogLevel(enum.Enum):
    """The log levels are used to specify the severity of a log message."""
    Debug = 0     # Debug log level.
    Warning = 1   # Warning log level.
    Error = 2     # Error log level.


class MatsConnectionResult(enum.Enum):
    """Connection results for MatsConnectionStatus."""
    Undefined = 0               # Undefined connection result.
    Ok = 1                      # Connection successful.
    UnknownClientId = 2         # Unknown client ID.
    WrongClientHostName = 3     # Wrong client host name.
    WrongServerId = 4           # Wrong server ID.


class MatsChannel(enum.Enum):
    """Channel types."""
    ChannelA = 0  # Represents channel A.
    ChannelB = 1  # Represents channel B.
    ChannelC = 2  # Represents channel C.
    ChannelD = 3  # Represents channel D.


class MatsSignalInOutType(enum.Enum):
    """Signal input/output/internal types."""
    Input = 0       # Represents an input signal type.
    Output = 1      # Represents an output signal type.
    Internal = 2    # Represents an internal signal type.


class MatsSignalType(enum.Enum):
    """Signal types."""
    Analog = 0      # Represents an analog signal type.
    Discrete = 1    # Represents a discrete signal type.
    Bus = 2         # Represents a bus signal type.


class MatsFlag(enum.Enum):
    """Flag masks to indicate the signal value"""
    VALID = 0x00000001	                # Flag mask to indicate that the signal value is valid.
    # Flag mask to indicate that the signal state is available (there is connection with the logic module).
    STATE_AVAILABLE = 0x00000002

    # Flag mask to indicate that the AFB sim_lock is applied with the specified flag.
    SIMULATED = 0x00000004

    # Flag mask to indicate that the signal is blocked (AFB sim_lock is applied with the specified flag).
    BLOCKED = 0x00000008

    # Flag mask to indicate that the signal value is mismatched.
    MISMATCH = 0x00000010

    # Flag mask to indicate that the signal value is above the high limit.
    ABOVE_HIGH_LIMIT = 0x00000020

    # Flag mask to indicate that the signal value is below the low limit.
    BELOW_LOW_LIMIT = 0x00000040

    # Flag mask to indicate that the signal is software simulated.
    SW_SIMULATED = 0x00000080

    # Flag mask to indicate that the signal is tunable and current value is equal to the default tuning value.
    TUNING_DEFAULT = 0x00000100
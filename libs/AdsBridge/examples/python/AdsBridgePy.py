import radiy
import threading
import time

exit_flag = False  # Set to True to exit the main loop


def logHandler(level, message):
    level = radiy.MatsLogLevel(level)
    message = message.decode("utf-8")
    print(f"{level.name}: {message}")


def main():
    ads = radiy.AdsBridge("AZPZ_WS1_ADSBRIDGE", False,
                          "../../../AdsBridge.dll")

    ads.setLogHandler(logHandler)
    ads.setLogLevel(radiy.MatsLogLevel.Debug)

    ads.addService("AZPZ_WS1_ADS", "127.0.0.1", 13323)
    ads.connect()

    # Start the user input thread
    #
    def user_input_thread():
        input("Press Enter to exit...")
        global exit_flag
        exit_flag = True

    input_thread = threading.Thread(target=user_input_thread)
    input_thread.start()

    # Wait for 5 seconds till AdsBridge connects to services  (ads.signalParamsLoaded() && ads.signalStatesLoaded())
    #
    for i in range(10):
        if ads.signalParamsLoaded() and ads.signalStatesLoaded():
            break
        time.sleep(0.5)

    # Get total signal count, assuming signals are loaded (if ads.signalParamsLoaded() == True)
    #
    print(f"Signal count: {ads.getSignalCount()}")

    # Get signal list
    #
    print_signal_list = False

    if print_signal_list:
        # ok may be False if signals are not loaded yet
        ok, signal_hashes = ads.getSignalList()
        for hash in signal_hashes:
            print(f"Signal: 0x{hash:016x}")

    # Example of getting AppSignalParam for two signals
    #
    ok, param = ads.getSignalParams(
        ["#AZPZ_RACK1_CH01_MD00_CTRLIN_INH02A", "#AZPZ_RACK1_CH01_MD00_CTRLIN_INH03A"])

    for p in param:
        print(f"AppSignalParam: {p.customSignalId}")
        print(f"    hash: 0x{p.hash:016x}")
        print(f"    customSignalId: {p.customSignalId}")
        print(f"    caption: {p.caption}")
        print(f"    equipmentId: {p.equipmentId}")
        print(f"    lmEquipmentId: {p.lmEquipmentId}")
        print(f"    unit: {p.unit}")
        print(f"    tags: {p.tags}")
        print(f"    channel: {p.channel}")
        print(f"    inOutType: {p.inOutType}")
        print(f"    type: {p.type}")
        print(f"    decimalPlaces: {p.decimalPlaces}")
        print(f"    lowValidRange: {p.lowValidRange}")
        print(f"    highValidRange: {p.highValidRange}")
        print(f"    tuning: {p.tuning}")

    # --
    #
    show_connection_status = True
    show_signal_states = True

    while not exit_flag:
        # Get connection statuses
        #
        if show_connection_status:
            statuses = ads.getTcpConnectionStatuses()
            for status in statuses:
                print(
                    f"Connection: {status.connectionType}, Status: {status.status}, Received: {status.received}, Sent: {status.sent}")

        # Get signal states
        #
        if show_signal_states:
            ok, states = ads.getSignalStates(
                ["#AZPZ_RACK1_CH01_MD00_CTRLIN_INH02A", "#AZPZ_RACK1_CH01_MD00_CTRLIN_INH03A"])
            # states is a list of MatsAppSignalState
            print(f"States: {len(states)}")
            for state in states:
                print(
                    f"Signal: {state.hash}, Value: {state.value}, PlantTime: {state.plantTime}, ServerTime: {state.serverTime}, Flags: {state.flags}")

        time.sleep(1)

    # Close all connections.
    ads.closeConnection()
    ads.shutdown()


main()

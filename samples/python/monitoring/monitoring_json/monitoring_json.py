import time
import ecal.core.core as ecal_core
from monitor import Monitor
import argparse

def main(interval, verbose):
    # print eCAL version and date
    print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))

    # initialize eCAL API
    ecal_core.initialize("monitoring")

    # initialize eCAL monitoring API
    ecal_core.mon_initialize()
    time.sleep(2)

    monitor = Monitor()

    while ecal_core.ok():
        monitor.update_monitor(ecal_data=ecal_core.mon_monitoring())
        if verbose:
            print("Monitoring data updated.")

        time.sleep(interval)

    # finalize eCAL monitoring API
    ecal_core.mon_finalize()

    # finalize eCAL API
    ecal_core.finalize()

    # finalie the monitor
    monitor.finalize_monitor()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="eCAL Monitoring Script.")

    parser.add_argument(
        "-i",
        "--interval",
        type=int,
        default=1,
        help="Monitoring interval in seconds (default: 1)",
    )

    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose output"
    )

    args = parser.parse_args()

    main(interval=args.interval, verbose=args.verbose)

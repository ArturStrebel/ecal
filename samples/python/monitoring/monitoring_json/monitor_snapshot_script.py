import os
import json
import time
import ecal.core.core as ecal_core
import argparse
from datetime import datetime
import string
import random

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

class SnapshotModifier:
    def __init__(self):
        """
        Im Dictionary  "fields" wird spezifiziert, Attribute aus den Snapshots entfernt bzw. pseudonymisiert werden. Um die Werte 
        eines Feldes zu entfernen muss der Name des Feldes als Key mit Value "" in das dict eingefügt werden. Um das Feld zu 
        pseudonymisieren muss als Value "pseudo" genutzt werden. Ein Beispiel, bei dem das Skript die Inhalte des Feldes pid entfernt,
        sowie die Inhalte von hname und tname psudeonymisiert ist wie folgt:  self.fields = {"pid": "", "hname": "pseudo", "tname":"pseudo"}
        """
        self.fields = {}
        self.all_pseudonyms = set()
        self.pseudonym_dict = {}

    def generate_random_string(self, length=10):
        # generate unique random string
        characters = string.ascii_letters + string.digits
        random_string = "".join(random.choice(characters) for _ in range(length))
        while random_string in self.all_pseudonyms:
            random_string = self.generate_random_string(10)
        self.all_pseudonyms.add(random_string)
        return random_string

    def modify_snapshot(self, snapshot):
        for _, objs in snapshot.items():
            for obj in objs:
                for attribute, replacement in self.fields.items():
                    if attribute in obj.keys():
                        if replacement == "":
                            obj[attribute] = replacement
                        elif replacement == "pseudo":
                            to_replace = obj[attribute]
                            if to_replace in self.pseudonym_dict.keys():
                                obj[attribute] = self.pseudonym_dict[to_replace]
                            else:
                                pseudonym = self.generate_random_string(10)
                                obj[attribute] = pseudonym
                                self.pseudonym_dict[to_replace] = pseudonym
        return snapshot


def main(interval, store_original):
    print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
    ecal_core.initialize("snapshotting")
    ecal_core.mon_initialize()
    time.sleep(2)

    snapshot_modifier = SnapshotModifier()

    snapshots_dir = os.path.join(os.path.dirname(__file__), f"snapshots_{timestamp}")
    os.makedirs(snapshots_dir)

    if store_original:
        original_dir = os.path.join(snapshots_dir, "original")
        os.makedirs(original_dir)  

    modified_dir = os.path.join(snapshots_dir, "modified")
    os.makedirs(modified_dir) 

    i = 0
    while ecal_core.ok():
        monitor_snapshot = ecal_core.mon_monitoring()

        if store_original:
            original_file_path = os.path.join(original_dir, f"original_snapshot-{i}.json")
            with open(original_file_path, "w") as original_file:
                json.dump(monitor_snapshot[1], original_file, indent=4)

        modified_snapshot = snapshot_modifier.modify_snapshot(monitor_snapshot[1])
        modified_file_path = os.path.join(modified_dir, f"modified_snapshot-{i}.json")
        with open(modified_file_path, "w") as modified_file:
            json.dump(modified_snapshot, modified_file, indent=4)

        i += 1
        time.sleep(interval)

    ecal_core.mon_finalize()
    ecal_core.finalize()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="eCAL Monitor-snapshot Script.")

    parser.add_argument(
        "-i",
        "--interval",
        type=int,
        default=1,
        help="Snaphsot interval in seconds (default: 1)",
    )

    parser.add_argument(
        "--store_original",
        action='store_true',  
        help="Store the original snapshot in addition to a modified version."
    )

    args = parser.parse_args()

    main(interval=args.interval, store_original=args.store_original)



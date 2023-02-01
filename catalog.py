from pathlib import Path
import pickle
import os


class Catalog:
    def __init__(self, db_loc):
        self.db_path = Path(db_loc)
        self.relation_metadata = {}
        self.attribute_metadata = {}
        self.catalog_db = "catalog.db"
        if os.path.exists(self.db_path / "catalog.db"):
            print("restoring catalog from save...")
            self.restore()
        else:
            print("creating new catalog...")

    def save(self):
        """save relation metadata and attribute metadata in between runs"""
        print("saving catalog to disk...")
        with open("catalog.db", "wb") as f:
            pickle.dump(self.relation_metadata, f)
            pickle.dump(self.attribute_metadata, f)

    def restore(self):
        """restore relation and attribute metadata to previous save"""
        with open("catalog.db", "rb") as f:
            self.relation_metadata = pickle.load(f)
            self.attribute_metadata = pickle.load(f)

    def populate_test(self):
        """just using to populate relation and attribute dicts with dummy data"""
        pass

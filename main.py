import argparse
import catalog

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Database System")
    parser.add_argument(
        "db_loc",
        type=str,
        help="absolute path to the directory to store the database",
    )
    parser.add_argument(
        "page_size",
        type=int,
        help="size in bytes of a page of a table",
    )
    parser.add_argument(
        "buffer_size",
        type=int,
        help="size limit of in-memory buffer",
    )
    args = parser.parse_args()
    db_loc, page_size, buffer_size = (
        args.db_loc,
        args.page_size,
        args.buffer_size,
    )

    catalog = catalog.Catalog(db_loc)

    while input() != "q":
        continue

    catalog.save()

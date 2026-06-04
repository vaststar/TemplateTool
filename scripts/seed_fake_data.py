"""Seed fake data into the TemplateTool shared SQLite database.

Inserts a small but realistic Contact + Camera directory tree:

  Contacts (Department slice)        Contacts (Reporting slice)
  ---------------------------        ---------------------------
  Engineering (group)                Alice Chen  (CTO, top)
    +-- Alice Chen                     +-- Bob Liu
    +-- Bob Liu                            +-- Cathy Wang
    +-- Backend Team (group)               +-- David Zhang
          +-- Cathy Wang               Eve Sun     (Sales Director, top)
          +-- David Zhang
  Sales (group)                      Cameras
    +-- Eve Sun                      -------
                                     Local (group)
                                       +-- Built-in Webcam (index 0)
                                       +-- USB Camera #2 (index 1)
                                     Remote (group)
                                       +-- Front Door (rtsp)
                                       +-- Backyard (rtsp)
                                     Yard (group)
                                       +-- (empty placeholder)

The Department slice and the Reporting slice are independent: the same person
shows up in both, attached to different parents via two different relation rows
(each carrying its own RELATION_ID UUID).

Usage (PowerShell):
  python scripts/seed_fake_data.py            # default debug DB
  python scripts/seed_fake_data.py --release  # release DB
  python scripts/seed_fake_data.py --db <path-to-shared_database.db>
  python scripts/seed_fake_data.py --wipe     # delete existing fake rows first

The script is idempotent: re-running it overwrites the same fake IDs via
INSERT OR REPLACE, so the directory tree stays stable across runs.
"""

from __future__ import annotations

import argparse
import os
import sqlite3
import sys
from pathlib import Path

# ----- Schema constants (mirror DataWarehouseSchemas.h) -----
T_USER_CONTACT     = "UserContact"
T_GROUP_CONTACT    = "GroupContact"
T_DEPARTMENT_GROUP = "DepartmentGroup"
T_TEAM_GROUP       = "TeamGroup"
T_CONTACT_REL      = "ContactRelation"
T_CAMERA_GROUP     = "CameraGroup"
T_CAMERA           = "Camera"
T_CAMERA_REL       = "CameraDirectoryRelation"

STATUS_ACTIVE              = 0   # ContactStatus::Active / CameraNodeStatus::Active
CAMERA_RELATION_CONT       = 0   # CameraDirectoryRelation::RelationType::Containment
CONTACT_RELATION_DEPARTMENT = 0  # IContactRelation::RelationType::Department
CONTACT_RELATION_REPORTING  = 1  # IContactRelation::RelationType::Reporting
SOURCE_TYPE_LOCAL          = 0
SOURCE_TYPE_NETWORK        = 1

# IGroupContact::GroupType
GROUP_TYPE_DEPARTMENT      = 0
GROUP_TYPE_PROJECT         = 1
GROUP_TYPE_TEAM            = 2
GROUP_TYPE_CUSTOM          = 3


def default_db_path(release: bool) -> Path:
    base = os.environ.get("LOCALAPPDATA") or os.environ.get("USERPROFILE")
    if not base:
        raise RuntimeError("Cannot resolve LOCALAPPDATA / USERPROFILE")
    folder = "TemplateToolApp" if release else "TemplateToolAppDebug"
    return Path(base) / folder / "app_data" / "shared_database.db"


# ----- Fake data -----

CONTACT_PERSONS = [
    # (id, full_name)
    ("seed_person_alice",  "Alice Chen"),
    ("seed_person_bob",    "Bob Liu"),
    ("seed_person_cathy",  "Cathy Wang"),
    ("seed_person_david",  "David Zhang"),
    ("seed_person_eve",    "Eve Sun"),
]

CONTACT_GROUPS = [
    # (id, name, group_type)
    ("seed_group_eng",     "Engineering",  GROUP_TYPE_DEPARTMENT),
    ("seed_group_backend", "Backend Team", GROUP_TYPE_TEAM),
    ("seed_group_sales",   "Sales",        GROUP_TYPE_DEPARTMENT),
]

# CTI sub-table rows. Each row's GROUP_ID must match a GROUP_TYPE_DEPARTMENT /
# GROUP_TYPE_TEAM row in CONTACT_GROUPS above.
# (group_id, manager_id, headcount)
DEPARTMENT_GROUP_ROWS = [
    ("seed_group_eng",   "seed_person_alice", 50),
    ("seed_group_sales", "seed_person_eve",   12),
]
# (group_id, team_lead_id, mission)
TEAM_GROUP_ROWS = [
    ("seed_group_backend", "seed_person_bob", "Own the backend service platform."),
]

# (relationId, child, parent, relationType)
# Department slice: who belongs to which org-chart group.
# Reporting slice: who reports to whom (top managers have no row here, they live
# at the virtual root). The same person can appear in both slices with two
# independent relation rows (different RELATION_IDs).
CONTACT_RELATIONS = [
    # ---- Department ----
    ("seed_rel_dep_alice",   "seed_person_alice",  "seed_group_eng",     CONTACT_RELATION_DEPARTMENT),
    ("seed_rel_dep_bob",     "seed_person_bob",    "seed_group_eng",     CONTACT_RELATION_DEPARTMENT),
    ("seed_rel_dep_backend", "seed_group_backend", "seed_group_eng",     CONTACT_RELATION_DEPARTMENT),
    ("seed_rel_dep_cathy",   "seed_person_cathy",  "seed_group_backend", CONTACT_RELATION_DEPARTMENT),
    ("seed_rel_dep_david",   "seed_person_david",  "seed_group_backend", CONTACT_RELATION_DEPARTMENT),
    ("seed_rel_dep_eve",     "seed_person_eve",    "seed_group_sales",   CONTACT_RELATION_DEPARTMENT),
    # ---- Reporting (上下级) ----
    # Alice (CTO) and Eve (Sales Director) sit at the virtual root, so they have
    # no reporting row of their own here.
    ("seed_rel_rep_bob",     "seed_person_bob",    "seed_person_alice",  CONTACT_RELATION_REPORTING),
    ("seed_rel_rep_cathy",   "seed_person_cathy",  "seed_person_bob",    CONTACT_RELATION_REPORTING),
    ("seed_rel_rep_david",   "seed_person_david",  "seed_person_bob",    CONTACT_RELATION_REPORTING),
]


CAMERA_GROUPS = [
    # (id, name)
    ("seed_cgroup_local",  "Local"),
    ("seed_cgroup_remote", "Remote"),
    ("seed_cgroup_yard",   "Yard"),
]

# (id, name, source_type, local_index, url, transport, open_timeout_ms, read_timeout_ms)
CAMERAS = [
    ("seed_cam_local_0",  "Built-in Webcam", SOURCE_TYPE_LOCAL,   0, "",                                  "",     0,    0),
    ("seed_cam_local_1",  "USB Camera #2",   SOURCE_TYPE_LOCAL,   1, "",                                  "",     0,    0),
    ("seed_cam_front",    "Front Door",      SOURCE_TYPE_NETWORK, 0, "rtsp://192.168.1.10:554/stream1",   "tcp",  5000, 5000),
    ("seed_cam_backyard", "Backyard",        SOURCE_TYPE_NETWORK, 0, "rtsp://192.168.1.11:554/stream1",   "tcp",  5000, 5000),
]

# (relationId, childId, parentId)
CAMERA_RELATIONS = [
    ("seed_rel_cam_local_0",  "seed_cam_local_0",  "seed_cgroup_local"),
    ("seed_rel_cam_local_1",  "seed_cam_local_1",  "seed_cgroup_local"),
    ("seed_rel_cam_front",    "seed_cam_front",    "seed_cgroup_remote"),
    ("seed_rel_cam_backyard", "seed_cam_backyard", "seed_cgroup_remote"),
]


def _table_exists(cur: sqlite3.Cursor, table: str) -> bool:
    cur.execute("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?", (table,))
    return cur.fetchone()[0] > 0


def _column_exists(cur: sqlite3.Cursor, table: str, column: str) -> bool:
    if not _table_exists(cur, table):
        return False
    cur.execute(f"PRAGMA table_info({table})")
    return any(row[1] == column for row in cur.fetchall())


def _migrate_contact_relation_schema(cur: sqlite3.Cursor) -> None:
    """If ContactRelation predates the RELATION_ID surrogate PK, drop the table so
    the C++ schema layer recreates it with the new shape on the next app launch.
    Old rows would be invalid under the new schema anyway (no RELATION_ID), so it
    is safe to discard them in dev.
    """
    if not _table_exists(cur, T_CONTACT_REL):
        return
    if _column_exists(cur, T_CONTACT_REL, "RELATION_ID"):
        return
    print(f"[migrate] {T_CONTACT_REL} is on the old schema (no RELATION_ID); dropping it so the app can recreate.")
    cur.execute(f"DROP TABLE {T_CONTACT_REL}")


def _migrate_camera_directory_relation_schema(cur: sqlite3.Cursor) -> None:
    """Bring CameraDirectoryRelation onto the RELATION_ID surrogate-PK schema.
    If the table is missing or on the old schema we (re)create it here so this
    same seed run can populate it — otherwise insert_cameras would skip the
    relation rows and the tree would render flat until the next app launch.
    Must mirror the C++ schema in DataWarehouseSchemas.cpp.
    """
    if _table_exists(cur, T_CAMERA_REL) and _column_exists(cur, T_CAMERA_REL, "RELATION_ID"):
        return
    if _table_exists(cur, T_CAMERA_REL):
        print(f"[migrate] {T_CAMERA_REL} is on the old schema (no RELATION_ID); dropping and recreating.")
        cur.execute(f"DROP TABLE {T_CAMERA_REL}")
    cur.execute(
        f"CREATE TABLE IF NOT EXISTS {T_CAMERA_REL} ("
        "RELATION_ID TEXT UNIQUE NOT NULL, "
        "CHILD_ID TEXT NOT NULL, "
        "PARENT_ID TEXT NOT NULL, "
        "RELATION_TYPE INTEGER NOT NULL)"
    )


def wipe_seed_rows(cur: sqlite3.Cursor) -> None:
    """Remove rows we previously inserted (matched by `seed_` id prefix)."""
    if _table_exists(cur, T_CONTACT_REL):
        if _column_exists(cur, T_CONTACT_REL, "RELATION_ID"):
            cur.execute(f"DELETE FROM {T_CONTACT_REL} WHERE RELATION_ID LIKE 'seed_%' OR CHILD_ID LIKE 'seed_%' OR PARENT_ID LIKE 'seed_%'")
        else:
            # Old-schema fallback: no RELATION_ID column yet.
            cur.execute(f"DELETE FROM {T_CONTACT_REL} WHERE CHILD_ID LIKE 'seed_%' OR PARENT_ID LIKE 'seed_%'")
    cur.execute(f"DELETE FROM {T_USER_CONTACT}  WHERE CONTACT_ID LIKE 'seed_%'")
    cur.execute(f"DELETE FROM {T_GROUP_CONTACT} WHERE GROUP_ID   LIKE 'seed_%'")
    if _table_exists(cur, T_DEPARTMENT_GROUP):
        cur.execute(f"DELETE FROM {T_DEPARTMENT_GROUP} WHERE GROUP_ID LIKE 'seed_%'")
    if _table_exists(cur, T_TEAM_GROUP):
        cur.execute(f"DELETE FROM {T_TEAM_GROUP}       WHERE GROUP_ID LIKE 'seed_%'")
    if _table_exists(cur, T_CAMERA_REL):
        if _column_exists(cur, T_CAMERA_REL, "RELATION_ID"):
            cur.execute(f"DELETE FROM {T_CAMERA_REL}    WHERE RELATION_ID LIKE 'seed_%' OR CHILD_ID LIKE 'seed_%' OR PARENT_ID LIKE 'seed_%'")
        else:
            cur.execute(f"DELETE FROM {T_CAMERA_REL}    WHERE CHILD_ID  LIKE 'seed_%' OR PARENT_ID LIKE 'seed_%'")
    if _table_exists(cur, T_CAMERA):
        cur.execute(f"DELETE FROM {T_CAMERA}        WHERE NODE_ID    LIKE 'seed_%'")
    if _table_exists(cur, T_CAMERA_GROUP):
        cur.execute(f"DELETE FROM {T_CAMERA_GROUP}  WHERE NODE_ID    LIKE 'seed_%'")


def insert_contacts(cur: sqlite3.Cursor) -> None:
    cur.executemany(
        f"INSERT OR REPLACE INTO {T_USER_CONTACT} (CONTACT_ID, CONTACT_FULL_NAME, CONTACT_STATUS) VALUES (?, ?, ?)",
        [(cid, name, STATUS_ACTIVE) for cid, name in CONTACT_PERSONS],
    )
    cur.executemany(
        f"INSERT OR REPLACE INTO {T_GROUP_CONTACT} (GROUP_ID, GROUP_NAME, GROUP_TYPE, CONTACT_STATUS) VALUES (?, ?, ?, ?)",
        [(gid, name, gtype, STATUS_ACTIVE) for gid, name, gtype in CONTACT_GROUPS],
    )
    # CTI sub-tables. They may not exist on a DB created before the CTI migration;
    # in that case ask the user to launch the app once so the C++ schema layer can
    # create them, then re-run this script.
    if _table_exists(cur, T_DEPARTMENT_GROUP):
        cur.executemany(
            f"INSERT OR REPLACE INTO {T_DEPARTMENT_GROUP} (GROUP_ID, MANAGER_ID, HEADCOUNT) VALUES (?, ?, ?)",
            DEPARTMENT_GROUP_ROWS,
        )
    else:
        print(f"[skip] {T_DEPARTMENT_GROUP} table missing; launch the app once to create CTI tables, then re-run.")
    if _table_exists(cur, T_TEAM_GROUP):
        cur.executemany(
            f"INSERT OR REPLACE INTO {T_TEAM_GROUP} (GROUP_ID, TEAM_LEAD_ID, MISSION) VALUES (?, ?, ?)",
            TEAM_GROUP_ROWS,
        )
    else:
        print(f"[skip] {T_TEAM_GROUP} table missing; launch the app once to create CTI tables, then re-run.")
    if _table_exists(cur, T_CONTACT_REL):
        if not _column_exists(cur, T_CONTACT_REL, "RELATION_ID"):
            print(f"[skip] {T_CONTACT_REL} is missing RELATION_ID; skipping relation seed. Launch the app once to let it recreate the table, then re-run this script.")
        else:
            cur.executemany(
                f"INSERT OR REPLACE INTO {T_CONTACT_REL} (RELATION_ID, CHILD_ID, PARENT_ID, RELATION_TYPE) VALUES (?, ?, ?, ?)",
                CONTACT_RELATIONS,
            )


def insert_cameras(cur: sqlite3.Cursor) -> None:
    if not _table_exists(cur, T_CAMERA_GROUP):
        print("[skip] Camera tables not found, skipping camera seed.")
        return
    cur.executemany(
        f"INSERT OR REPLACE INTO {T_CAMERA_GROUP} (NODE_ID, DISPLAY_NAME, NODE_STATUS) VALUES (?, ?, ?)",
        [(nid, name, STATUS_ACTIVE) for nid, name in CAMERA_GROUPS],
    )
    cur.executemany(
        f"INSERT OR REPLACE INTO {T_CAMERA} "
        "(NODE_ID, DISPLAY_NAME, NODE_STATUS, SOURCE_TYPE, LOCAL_INDEX, NETWORK_URL, NETWORK_TRANSPORT, OPEN_TIMEOUT_MS, READ_TIMEOUT_MS) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
        [(nid, name, STATUS_ACTIVE, st, idx, url, transport, ot, rt)
         for (nid, name, st, idx, url, transport, ot, rt) in CAMERAS],
    )
    if _table_exists(cur, T_CAMERA_REL):
        if not _column_exists(cur, T_CAMERA_REL, "RELATION_ID"):
            print(f"[skip] {T_CAMERA_REL} is missing RELATION_ID; skipping camera relation seed. Launch the app once to let it recreate the table, then re-run this script.")
        else:
            cur.executemany(
                f"INSERT OR REPLACE INTO {T_CAMERA_REL} (RELATION_ID, CHILD_ID, PARENT_ID, RELATION_TYPE) VALUES (?, ?, ?, ?)",
                [(rid, child, parent, CAMERA_RELATION_CONT) for rid, child, parent in CAMERA_RELATIONS],
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--release", action="store_true",
                        help="Target the release DB (TemplateToolApp); default is debug.")
    parser.add_argument("--db", type=Path, default=None,
                        help="Explicit path to shared_database.db (overrides --release).")
    parser.add_argument("--wipe", action="store_true",
                        help="Delete any previously seeded rows (id prefix 'seed_') before inserting.")
    args = parser.parse_args()

    db_path = args.db if args.db else default_db_path(args.release)
    if not db_path.exists():
        print(f"[error] DB not found: {db_path}", file=sys.stderr)
        print("        Run the app at least once so the schema is created, then retry.", file=sys.stderr)
        return 1

    print(f"[info] using db: {db_path}")
    conn = sqlite3.connect(str(db_path))
    try:
        cur = conn.cursor()
        _migrate_contact_relation_schema(cur)
        _migrate_camera_directory_relation_schema(cur)
        if args.wipe:
            print("[info] wiping previously seeded rows...")
            wipe_seed_rows(cur)

        insert_contacts(cur)
        insert_cameras(cur)
        conn.commit()

        # Summary
        def count(table: str) -> int:
            return cur.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]

        print("[done] table row counts:")
        for t in (T_USER_CONTACT, T_GROUP_CONTACT, T_DEPARTMENT_GROUP, T_TEAM_GROUP, T_CONTACT_REL,
                  T_CAMERA, T_CAMERA_GROUP, T_CAMERA_REL):
            if _table_exists(cur, t):
                print(f"  {t:30s} {count(t)}")
    finally:
        conn.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())

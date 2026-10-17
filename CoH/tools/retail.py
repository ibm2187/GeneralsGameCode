"""CoH: the retail Zero Hour data as plain files, so it can be searched instead of re-read from archives.

    python CoH/tools/retail.py ["<Zero Hour folder>"]

writes build/retail/ in the checkout (ignored by git):
    INI/...            every INI file of INIZH.big, with its folders (Object/AmericaAirUnits.ini, CommandSet.ini, ...)
    English/...        the INI files of EnglishZH.big (CommandMap.ini, ...)
    strings.txt        generals.csf as LABEL = text, one per line: unit names, button labels, tooltips
    Scripts/*.scb      the skirmish and multiplayer AI scripts (binary)
    files.txt          every file in every .big, with its archive

Packs that generate INI can import read_big and read_csf from here:
    sys.path.insert(0, <path to CoH/tools>); from retail import read_big
"""

import os
import shutil
import struct
import sys

GAME_DIR = r"C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour"
OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "build", "retail")


def big_entries(path):
    """(name, bytes) for every file in a .big archive."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] not in (b"BIGF", b"BIG4"):
        return
    count = struct.unpack(">I", data[8:12])[0]
    pos = 16
    for _ in range(count):
        offset, size = struct.unpack(">II", data[pos:pos + 8])
        end = data.index(b"\0", pos + 8)
        name = data[pos + 8:end].decode("latin-1")
        pos = end + 1
        yield name, data[offset:offset + size]


def read_big(path):
    """The INI files of a .big archive, as {name: text}."""
    return {name: body.decode("latin-1") for name, body in big_entries(path) if name.lower().endswith(".ini")}


def read_csf(body):
    """A .csf string table as {label: text}."""
    label_count = struct.unpack("<I", body[8:12])[0]
    strings, pos = {}, 24
    for _ in range(label_count):
        pos += 4  # "LBL "
        string_count, length = struct.unpack("<II", body[pos:pos + 8])
        pos += 8
        label = body[pos:pos + length].decode("latin-1")
        pos += length
        texts = []
        for _ in range(string_count):
            kind = body[pos:pos + 4]
            chars = struct.unpack("<I", body[pos + 4:pos + 8])[0]
            pos += 8
            raw = bytes(~b & 0xFF for b in body[pos:pos + chars * 2])
            texts.append(raw.decode("utf-16-le"))
            pos += chars * 2
            if kind == b"WRTS":
                extra = struct.unpack("<I", body[pos:pos + 4])[0]
                pos += 4 + extra
        strings[label] = texts[0] if texts else ""
    return strings


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def relative_ini_path(name):
    """Data\\INI\\Object\\X.ini -> INI/Object/X.ini; Data\\English\\X.ini -> English/X.ini."""
    parts = name.replace("\\", "/").split("/")
    if parts[0].lower() == "data":
        parts = parts[1:]
    return os.path.join(*parts)


def main():
    game_dir = sys.argv[1] if len(sys.argv) > 1 else GAME_DIR
    out_dir = os.path.normpath(OUT_DIR)
    if os.path.isdir(out_dir):
        shutil.rmtree(out_dir)

    listing = []
    for big in sorted(os.listdir(game_dir)):
        if not big.lower().endswith(".big"):
            continue
        for name, body in big_entries(os.path.join(game_dir, big)):
            listing.append(f"{big}\t{name}")
            lower = name.lower()
            if big.lower() in ("inizh.big", "englishzh.big") and lower.endswith(".ini"):
                write(os.path.join(out_dir, relative_ini_path(name)), body)
            elif big.lower() == "englishzh.big" and lower.endswith("generals.csf"):
                lines = [f"{label} = {text}".replace("\n", "\\n") for label, text in sorted(read_csf(body).items())]
                write(os.path.join(out_dir, "strings.txt"), ("\n".join(lines) + "\n").encode("utf-8"))

    scripts = os.path.join(game_dir, "Data", "Scripts")
    if os.path.isdir(scripts):
        os.makedirs(os.path.join(out_dir, "Scripts"), exist_ok=True)
        for name in os.listdir(scripts):
            if name.lower().endswith(".scb"):
                shutil.copy(os.path.join(scripts, name), os.path.join(out_dir, "Scripts", name))

    write(os.path.join(out_dir, "files.txt"), ("\n".join(listing) + "\n").encode("utf-8"))
    print(f"retail data written to {out_dir}")


if __name__ == "__main__":
    main()

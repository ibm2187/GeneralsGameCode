# IBM Engine: builds IbmDemo.map, a blank two-player skirmish map for the Demo setup: flat grass,
# no props, two start positions facing each other across the middle.
#
#   python make_demo_map.py <MapsZH.big> <output .map>
#
# It starts from the retail Tournament Desert map (for its skirmish sides, teams and lighting) and
# replaces the terrain, textures and objects. The map is written uncompressed; the game reads both.

import struct
import sys

SOURCE_MAP = 'maps\\tournament desert\\tournament desert.map'

PLAYABLE = 300          # cells across the playable area (10 world units each)
BORDER = 30             # cells of border around it
HEIGHT = 32             # flat ground height (x0.625 = 20 world units)
TEXTURE = 'GrassMediumType35'
TEXTURE_TILES = 16      # tiles in that texture class (Terrain.ini), laid out TEXTURE_WIDTH across
TEXTURE_WIDTH = 4
STARTS = [(600.0, 1500.0), (2400.0, 1500.0)]   # world units, inside the playable area


def read_big(path, wanted):
    with open(path, 'rb') as f:
        f.read(8)
        count, _ = struct.unpack('>II', f.read(8))
        for _ in range(count):
            off, size = struct.unpack('>II', f.read(8))
            name = b''
            while (c := f.read(1)) != b'\0':
                name += c
            if name.decode('latin-1').lower() == wanted:
                f.seek(off)
                return f.read(size)
    raise SystemExit('not found in the archive: ' + wanted)


def refpack_decode(src):
    t = (src[0] << 8) | src[1]
    s = 2
    if t & 0x8000:
        s += 4 if t & 0x100 else 0
        ulen = int.from_bytes(src[s:s + 4], 'big')
        s += 4
    else:
        s += 3 if t & 0x100 else 0
        ulen = int.from_bytes(src[s:s + 3], 'big')
        s += 3
    d = bytearray()
    while True:
        first = src[s]
        s += 1
        if not first & 0x80:
            second = src[s]
            s += 1
            run = first & 3
            d += src[s:s + run]
            s += run
            ref = len(d) - 1 - (((first & 0x60) << 3) + second)
            n = ((first & 0x1c) >> 2) + 3
        elif not first & 0x40:
            second, third = src[s], src[s + 1]
            s += 2
            run = second >> 6
            d += src[s:s + run]
            s += run
            ref = len(d) - 1 - (((second & 0x3f) << 8) + third)
            n = (first & 0x3f) + 4
        elif not first & 0x20:
            second, third, forth = src[s], src[s + 1], src[s + 2]
            s += 3
            run = first & 3
            d += src[s:s + run]
            s += run
            ref = len(d) - 1 - ((((first & 0x10) >> 4) << 16) + (second << 8) + third)
            n = (((first & 0x0c) >> 2) << 8) + forth + 5
        else:
            run = ((first & 0x1f) << 2) + 4
            if run <= 112:
                d += src[s:s + run]
                s += run
                continue
            run = first & 3
            d += src[s:s + run]
            break
        for _ in range(n):
            d.append(d[ref])
            ref += 1
    assert len(d) == ulen
    return bytes(d)


def parse_chunks(data):
    assert data[:4] == b'CkMp'
    p = 4
    count = struct.unpack_from('<i', data, p)[0]
    p += 4
    ids = {}
    for _ in range(count):
        n = data[p]
        name = data[p + 1:p + 1 + n].decode('latin-1')
        ids[name] = struct.unpack_from('<I', data, p + 1 + n)[0]
        p += 5 + n
    table = data[4:p]
    chunks = []
    names = {v: k for k, v in ids.items()}
    while p < len(data):
        cid, ver, size = struct.unpack_from('<IHi', data, p)
        chunks.append((names[cid], ver, data[p + 10:p + 10 + size]))
        p += 10 + size
    return table, ids, chunks


def chunk(ids, name, version, payload):
    return struct.pack('<IHi', ids[name], version, len(payload)) + payload


def string(s):
    b = s.encode('latin-1')
    return struct.pack('<H', len(b)) + b


def dictionary(ids, items):
    out = struct.pack('<H', len(items))
    for key, kind, value in items:
        out += struct.pack('<i', (ids[key] << 8) | kind)
        if kind == 0:
            out += struct.pack('<B', 1 if value else 0)
        elif kind == 1:
            out += struct.pack('<i', value)
        elif kind == 2:
            out += struct.pack('<f', value)
        elif kind == 3:
            out += string(value)
    return out


def height_map():
    size = PLAYABLE + 2 * BORDER
    out = struct.pack('<iiii', size, size, BORDER, 1)
    out += struct.pack('<ii', PLAYABLE, PLAYABLE)
    out += struct.pack('<i', size * size) + bytes([HEIGHT]) * (size * size)
    return out


def blend_tiles():
    size = PLAYABLE + 2 * BORDER
    tiles = bytearray()
    for y in range(size):
        for x in range(size):
            # WorldBuilder's getTileNdxForClass: a texture tile spans 2x2 cells.
            tile = (x // 2) % TEXTURE_WIDTH + TEXTURE_WIDTH * ((y // 2) % TEXTURE_WIDTH)
            tiles += struct.pack('<h', (tile << 2) + 2 * (y & 1) + (x & 1))
    zeros = bytes(2 * size * size)
    out = struct.pack('<i', size * size)
    out += bytes(tiles) + zeros + zeros + zeros          # tiles, blends, 3-way blends, cliff infos
    out += bytes(size * ((size + 7) // 8))              # cliff state bits
    out += struct.pack('<iiii', TEXTURE_TILES, 1, 1, 1)  # bitmap tiles, blended tiles, cliff infos, texture classes
    out += struct.pack('<iiii', 0, TEXTURE_TILES, TEXTURE_WIDTH, 0) + string(TEXTURE)
    out += struct.pack('<ii', 0, 0)                      # no edge tiles or edge texture classes
    return out


def objects(ids):
    out = b''
    for i, (x, y) in enumerate(STARTS):
        name = 'Player_%d_Start' % (i + 1)
        props = [
            ('objectInitialHealth', 1, 100), ('objectEnabled', 0, True), ('objectIndestructible', 0, False),
            ('objectUnsellable', 0, False), ('objectPowered', 0, True), ('objectRecruitableAI', 0, True),
            ('originalOwner', 3, 'team'), ('uniqueID', 3, name), ('objectLayer', 3, 'Start Points'),
            ('waypointID', 1, i + 1), ('objectSelectable', 0, False), ('waypointName', 3, name),
        ]
        body = struct.pack('<ffffi', x, y, 0.0, 0.0, 0) + string('*Waypoints/Waypoint') + dictionary(ids, props)
        out += chunk(ids, 'Object', 3, body)
    return out


def main():
    big, output = sys.argv[1], sys.argv[2]
    data = read_big(big, SOURCE_MAP)
    if data[:4] == b'EAR\0':
        data = refpack_decode(data[8:])
    table, ids, chunks = parse_chunks(data)

    out = b'CkMp' + table
    for name, ver, payload in chunks:
        if name == 'HeightMapData':
            payload = height_map()
        elif name == 'BlendTileData':
            payload = blend_tiles()
        elif name == 'WorldInfo':
            payload = dictionary(ids, [('weather', 1, 0)])
        elif name == 'ObjectsList':
            payload = objects(ids)
        elif name == 'PolygonTriggers':
            payload = struct.pack('<i', 0)  # no water, no areas
        elif name == 'WaypointsList':
            payload = struct.pack('<i', 0)  # no waypoint paths
        # SidesList (skirmish sides and teams) and GlobalLighting stay as they are.
        out += struct.pack('<IHi', ids[name], ver, len(payload)) + payload

    with open(output, 'wb') as f:
        f.write(out)
    print('wrote %s (%d bytes)' % (output, len(out)))


if __name__ == '__main__':
    main()

"""Minimal OSC 1.0 message encoding/decoding (no bundles).

Only what UnTETered Sync needs: int32 (i), float32 (f), float64 (d),
string (s) and the argument-less True/False tags (T/F).
"""

import struct


def _read_string(data, index):
    """Read a null-terminated, 4-byte-padded OSC string starting at index.
    Returns (string, index of the next field)."""
    end = data.index(b"\x00", index)
    text = data[index:end].decode("utf-8")
    next_index = (end + 4) & ~3  # skip the null and pad to a multiple of 4
    return text, next_index


def _pad(raw):
    """Null-terminate and pad bytes to a multiple of 4 (always adds at least one null)."""
    return raw + b"\x00" * (4 - len(raw) % 4)


def parse_message(data):
    """Parse one OSC message. Returns (address, [args]) or None if the packet
    is a bundle or malformed."""
    try:
        if data.startswith(b"#bundle"):
            return None
        address, index = _read_string(data, 0)
        if not address.startswith("/"):
            return None
        if index >= len(data):
            return address, []

        tags, index = _read_string(data, index)
        if not tags.startswith(","):
            return None

        args = []
        for tag in tags[1:]:
            if tag == "i":
                args.append(struct.unpack(">i", data[index:index + 4])[0])
                index += 4
            elif tag == "f":
                args.append(struct.unpack(">f", data[index:index + 4])[0])
                index += 4
            elif tag == "d":
                args.append(struct.unpack(">d", data[index:index + 8])[0])
                index += 8
            elif tag == "s":
                text, index = _read_string(data, index)
                args.append(text)
            elif tag == "T":
                args.append(True)
            elif tag == "F":
                args.append(False)
            else:
                return None  # unsupported type: ignore the whole message
        return address, args
    except (ValueError, struct.error, UnicodeDecodeError):
        return None


def build_message(address, *args):
    """Encode an OSC message with int, float, bool and str arguments."""
    tags = ","
    payload = b""
    for arg in args:
        if isinstance(arg, bool):
            tags += "T" if arg else "F"
        elif isinstance(arg, int):
            tags += "i"
            payload += struct.pack(">i", arg)
        elif isinstance(arg, float):
            tags += "f"
            payload += struct.pack(">f", arg)
        elif isinstance(arg, str):
            tags += "s"
            payload += _pad(arg.encode("utf-8"))
        else:
            raise TypeError("Unsupported OSC argument type: %r" % type(arg))
    return _pad(address.encode("utf-8")) + _pad(tags.encode("utf-8")) + payload
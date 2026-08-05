"""Research-only, source-anchored filename observation streams.

Exact filesystem identity and native name bytes remain outside Kolmogrov.  This
module accepts a decoded Unicode-scalar filename and emits several independent
one-atom-per-source-scalar views so mutation flow remains inspectable.
"""

from __future__ import annotations

import unicodedata
from dataclasses import dataclass


PROFILE_ID = "org.filemanager.kolmogrov.filename-observation@0.1.0-research.1"
FOLD_CAPSULE_MAX_BYTES = 33
STRUCTURAL_ROLES = (
    "letter",
    "mark",
    "number",
    "dot",
    "separator",
    "punctuation",
    "symbol",
    "control",
    "other",
)


@dataclass(frozen=True, slots=True)
class FilenameAtomStream:
    view_id: str
    atoms: tuple[str, ...]
    source_spans: tuple[tuple[int, int], ...]


@dataclass(frozen=True, slots=True)
class FilenameObservation:
    profile_id: str
    unicode_version: str
    source: str
    streams: tuple[FilenameAtomStream, ...]

    def stream(self, view_id: str) -> FilenameAtomStream:
        for stream in self.streams:
            if stream.view_id == view_id:
                return stream
        raise KeyError(view_id)


@dataclass(frozen=True, slots=True)
class FilenameAtomCode:
    value: int
    bit_width: int


def _validate_scalar_filename(name: str) -> None:
    if not isinstance(name, str):
        raise TypeError("filename must be a decoded string")
    if not name:
        raise ValueError("filename component must not be empty")
    for character in name:
        codepoint = ord(character)
        if character == "\x00":
            raise ValueError("filename component must not contain NUL")
        if 0xD800 <= codepoint <= 0xDFFF:
            raise ValueError("filename component must contain Unicode scalars")


def _literal_atom(character: str) -> str:
    return f"scalar:{ord(character):06x}"


def _fold_capsule(character: str) -> str:
    folded = unicodedata.normalize("NFKC", character)
    folded = folded.casefold()
    folded = unicodedata.normalize("NFKC", folded)
    payload = folded.encode("utf-8")
    if len(payload) > FOLD_CAPSULE_MAX_BYTES:
        raise ValueError("fold capsule exceeds the profile's frozen byte bound")
    return "fold:" + payload.hex()


def _structural_atom(character: str) -> str:
    if character == ".":
        role = "dot"
    elif character in {"_", "-"} or character.isspace():
        role = "separator"
    else:
        category_family = unicodedata.category(character)[:1]
        role = {
            "L": "letter",
            "M": "mark",
            "N": "number",
            "P": "punctuation",
            "S": "symbol",
            "Z": "separator",
            "C": "control",
        }.get(category_family, "other")
    return "class:" + role


def observe_filename(name: str) -> FilenameObservation:
    """Emit literal, anchored-fold, and structural streams independently."""

    _validate_scalar_filename(name)
    spans = tuple((index, index + 1) for index in range(len(name)))
    streams = (
        FilenameAtomStream(
            view_id="filename.literal-scalar",
            atoms=tuple(_literal_atom(character) for character in name),
            source_spans=spans,
        ),
        FilenameAtomStream(
            view_id="filename.anchored-nfkc-casefold",
            atoms=tuple(_fold_capsule(character) for character in name),
            source_spans=spans,
        ),
        FilenameAtomStream(
            view_id="filename.structural-class",
            atoms=tuple(_structural_atom(character) for character in name),
            source_spans=spans,
        ),
    )
    return FilenameObservation(
        profile_id=PROFILE_ID,
        unicode_version=unicodedata.unidata_version,
        source=name,
        streams=streams,
    )


def encode_filename_atom(view_id: str, atom: str) -> FilenameAtomCode:
    """Return the profile's exact bounded integer code for one emitted atom."""

    if view_id == "filename.literal-scalar":
        prefix = "scalar:"
        if not atom.startswith(prefix):
            raise ValueError("literal atom has the wrong tag")
        value = int(atom[len(prefix) :], 16)
        if value >= 1 << 21:
            raise ValueError("literal scalar exceeds 21 bits")
        return FilenameAtomCode(value=value, bit_width=21)
    if view_id == "filename.anchored-nfkc-casefold":
        prefix = "fold:"
        if not atom.startswith(prefix):
            raise ValueError("fold atom has the wrong tag")
        payload = bytes.fromhex(atom[len(prefix) :])
        if len(payload) > FOLD_CAPSULE_MAX_BYTES:
            raise ValueError("fold atom exceeds the frozen byte bound")
        value = len(payload) | (int.from_bytes(payload, "little") << 6)
        return FilenameAtomCode(
            value=value,
            bit_width=6 + 8 * FOLD_CAPSULE_MAX_BYTES,
        )
    if view_id == "filename.structural-class":
        prefix = "class:"
        if not atom.startswith(prefix):
            raise ValueError("structural atom has the wrong tag")
        try:
            value = STRUCTURAL_ROLES.index(atom[len(prefix) :])
        except ValueError as error:
            raise ValueError("unknown structural role") from error
        return FilenameAtomCode(value=value, bit_width=4)
    raise ValueError("unknown filename observation view")

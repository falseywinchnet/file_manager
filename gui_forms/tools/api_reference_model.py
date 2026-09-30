"""Typed records at the compiler JSON and authored-contract boundaries."""
from __future__ import annotations
from typing import TypedDict

class SourceLocation(TypedDict, total=False):
    file: str
    line: int
    offset: int
    tokLen: int
    spellingLoc: SourceLocation

class SourceRange(TypedDict, total=False):
    begin: SourceLocation
    end: SourceLocation

class TypeDescription(TypedDict, total=False):
    qualType: str
    desugaredQualType: str

class BaseDescription(TypedDict):
    type: TypeDescription

class AstNode(TypedDict, total=False):
    kind: str
    name: str
    loc: SourceLocation
    range: SourceRange
    inner: list[AstNode]
    type: TypeDescription
    text: str
    completeDefinition: bool
    isImplicit: bool
    tagUsed: str
    access: str
    bases: list[BaseDescription]

class Parameter(TypedDict):
    name: str
    type: str
    declaration: str

class Symbol(TypedDict, total=False):
    id: str
    name: str
    type: str
    namespace: str
    kind: str
    access: str
    parameters: list[Parameter]
    declaration: str
    comment: str
    header: str
    source: str
    line: int
    page: str
    bases: list[str]
    members: list[Symbol]
    enclosing_type: str | None
    associated_type: str

class Inventory(TypedDict):
    schema: int
    types: list[Symbol]
    functions: list[Symbol]
    aliases: list[Symbol]

class Example(TypedDict):
    source: str
    caption: str | list[str]

class Contract(TypedDict, total=False):
    reviewed: bool
    summary: str
    remarks: str | list[str]
    ownership: str | list[str]
    threading: str | list[str]
    availability: str | list[str]
    returns: str | list[str]
    errors: str | list[str]
    parameters: dict[str, str]
    see_also: list[str]
    examples: list[Example]
    contract_source: str

class ContractFile(TypedDict):
    schema: int
    symbols: dict[str, Contract]

class Coverage(TypedDict):
    schema: int
    types: int
    declared_members: int
    namespace_functions: int
    namespace_aliases_and_constants: int
    symbols: int
    reviewed_contracts: int
    pending_contracts: int
    missing: list[str]

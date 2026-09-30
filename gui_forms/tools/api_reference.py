#!/usr/bin/env python3
"""Extract exact native C++ API declarations through Clang and render reference pages.

No behavior is inferred from method names. Compiler facts and authored contracts
remain separate so documentation coverage is measurable.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "docs" / "reference"
RECORD_KINDS = {"CXXRecordDecl", "RecordDecl", "EnumDecl"}
CALLABLE_KINDS = {"CXXMethodDecl", "CXXConstructorDecl", "CXXDestructorDecl", "FunctionDecl", "FunctionTemplateDecl"}
MEMBER_KINDS = CALLABLE_KINDS | {"FieldDecl", "VarDecl", "TypeAliasDecl", "TypedefDecl", "EnumConstantDecl"}


def roots_from_json(text):
    decoder = json.JSONDecoder()
    offset = 0
    while offset < len(text):
        while offset < len(text) and text[offset].isspace():
            offset += 1
        if offset == len(text):
            break
        value, offset = decoder.raw_decode(text, offset)
        yield value


def node_file(node, inherited):
    location = node.get("loc", {})
    return location.get("file", location.get("spellingLoc", {}).get("file", inherited))


def source_declaration(node, filename):
    path = ROOT / filename
    if not path.is_file():
        raise RuntimeError("Missing source for compiler declaration: " + filename)
    source = path.read_bytes()
    bounds = node.get("range", {})
    begin = bounds.get("begin", {})
    end = bounds.get("end", {})
    begin = begin.get("spellingLoc", begin)
    end = end.get("spellingLoc", end)
    first = begin.get("offset")
    last = end.get("offset")
    if first is None or last is None:
        return ""
    # Namespace functions begin at the return type in Clang's range even when
    # a preceding standard attribute belongs to the declaration.
    prefix = source[:first].decode("utf-8")
    attributes = re.search(r"(?:\[\[[^\]]*\]\]\s*)+$", prefix)
    if attributes:
        first = len(prefix[:attributes.start()].encode("utf-8"))
    last += end.get("tokLen", 1)
    children = node.get("inner", [])
    if node.get("kind") == "FunctionTemplateDecl":
        children = next((c.get("inner", []) for c in children if c.get("kind") in CALLABLE_KINDS), [])
    for child in children:
        if child.get("kind") == "CompoundStmt":
            last = child["range"]["begin"]["offset"]
            break
    declaration = source[first:last].decode("utf-8").strip().rstrip(";")
    if node.get("kind") in RECORD_KINDS:
        declaration = declaration.split("{", 1)[0].strip()
    if node.get("kind") == "CXXConstructorDecl":
        depth = 0
        for index, character in enumerate(declaration):
            if character == "(": depth += 1
            elif character == ")": depth -= 1
            elif character == ":" and depth == 0 and declaration[index:index + 2] != "::" and declaration[index - 1:index] != ":":
                declaration = declaration[:index].rstrip()
                break
    return re.sub(r"\s+", " ", declaration)


def comment_text(node):
    parts = []
    def collect(current):
        if current.get("kind") == "TextComment":
            parts.append(current.get("text", "").strip())
        for child in current.get("inner", []):
            collect(child)
    for child in node.get("inner", []):
        if child.get("kind") == "FullComment":
            collect(child)
    return " ".join(parts)


def slug(identifier):
    readable = re.sub(r"[^a-z0-9]+", "-", identifier.lower()).strip("-")[:110]
    return readable + "-" + hashlib.sha256(identifier.encode()).hexdigest()[:10]


def inventory(ast_text):
    records = {}
    functions = {}
    aliases = {}

    def member(node, scope, filename, access, owner_name=None):
        actual = node
        if node.get("kind") == "FunctionTemplateDecl":
            actual = next((c for c in node.get("inner", []) if c.get("kind") in CALLABLE_KINDS), node)
        name = actual.get("name", "")
        if actual.get("kind") == "CXXConstructorDecl": name = owner_name
        if actual.get("kind") == "CXXDestructorDecl": name = "~" + owner_name
        signature = actual.get("type", {}).get("qualType", "")
        identifier = "::".join(scope + [name]) + "|" + signature
        params = [{"name": p.get("name", ""), "type": p.get("type", {}).get("qualType", ""),
                   "declaration": source_declaration(p, filename)}
                  for p in actual.get("inner", []) if p.get("kind") == "ParmVarDecl"]
        location = actual.get("loc", {})
        return {"id": identifier, "name": name, "type": signature,
            "namespace": "::".join(scope), "kind": node.get("kind"), "access": access,
            "parameters": params, "declaration": source_declaration(node, filename),
            "comment": comment_text(node), "header": filename.removeprefix("include/"),
            "source": filename,
            "line": location.get("line", (ROOT / filename).read_bytes()[:location.get("offset", 0)].count(b"\n") + 1),
            "page": slug(identifier) + ".html"}

    def visit(node, scope, filename, access="public", template=None, namespace_scope=None):
        namespace_scope = namespace_scope or []
        filename = node_file(node, filename)
        if filename.startswith(str(ROOT) + "/"):
            filename = str(Path(filename).relative_to(ROOT))
        kind = node.get("kind")
        name = node.get("name", "")
        if kind == "NamespaceDecl":
            if not name or name == "detail":
                return
            for child in node.get("inner", []):
                visit(child, scope + [name], filename, namespace_scope=namespace_scope + [name])
            return
        if kind == "ClassTemplateDecl":
            for child in node.get("inner", []):
                if child.get("kind") == "CXXRecordDecl" and child.get("completeDefinition"):
                    visit(child, scope, filename, access, node, namespace_scope)
            return
        if (filename.startswith("include/gui_forms/") and access != "private"
                and not node.get("isImplicit") and name):
            if kind in {"FunctionDecl", "FunctionTemplateDecl"}:
                entry = member(node, scope, filename, access)
                functions[entry["id"]] = entry
                return
            if kind in {"TypeAliasDecl", "TypedefDecl", "VarDecl"}:
                entry = member(node, scope, filename, access)
                aliases[entry["id"]] = entry
                return
        if kind not in RECORD_KINDS or not name or node.get("isImplicit") or access == "private":
            return
        if kind != "EnumDecl" and not node.get("completeDefinition"):
            return
        if not filename.startswith("include/gui_forms/"):
            return
        identifier = "::".join(scope + [name])
        location = node.get("loc", {})
        record = {"id": identifier, "name": name, "namespace": "::".join(scope),
                  "kind": "enum" if kind == "EnumDecl" else node.get("tagUsed", "class"),
                  "header": filename.removeprefix("include/"), "source": filename,
                  "line": location.get("line", (ROOT / filename).read_bytes()[:location.get("offset", 0)].count(b"\n") + 1),
                  "declaration": source_declaration(template or node, filename).split("{", 1)[0].strip(),
                  "comment": comment_text(template or node), "access": access,
                  "bases": [b["type"].get("desugaredQualType", b["type"]["qualType"]) for b in node.get("bases", [])],
                  "members": [], "page": slug(identifier) + ".html"}
        record["namespace"] = "::".join(namespace_scope)
        record["enclosing_type"] = "::".join(scope) if scope != namespace_scope else None
        current_access = "private" if record["kind"] == "class" else "public"
        for child in node.get("inner", []):
            child_kind = child.get("kind")
            if child_kind == "AccessSpecDecl":
                current_access = child["access"]
                continue
            if child_kind == "FriendDecl":
                for friend in child.get("inner", []):
                    if friend.get("kind") in {"FunctionDecl", "FunctionTemplateDecl"}:
                        entry = member(friend, namespace_scope, filename, "public")
                        entry["associated_type"] = identifier
                        # A hidden friend is declared in the enclosing namespace.
                        # Qualify its record parameter in its identity to distinguish
                        # nested records with the same short name.
                        signature = re.sub(r"(?<![\w:])" + re.escape(name) + r"\b", identifier, entry["type"])
                        entry["id"] = "::".join(namespace_scope + [entry["name"]]) + "|" + signature
                        entry["page"] = slug(entry["id"]) + ".html"
                        functions[entry["id"]] = entry
                continue
            if current_access == "private" or child.get("isImplicit"):
                continue
            if child_kind in RECORD_KINDS or child_kind == "ClassTemplateDecl":
                visit(child, scope + [name], filename, current_access, namespace_scope=namespace_scope)
                continue
            if child_kind not in MEMBER_KINDS:
                continue
            record["members"].append(member(child, scope + [name], filename, current_access, name))
        # Only one source definition may own a public qualified identity.
        previous = records.get(identifier)
        if previous and previous["source"] != filename:
            raise RuntimeError("Ambiguous API identity: " + identifier)
        records[identifier] = record
    for root in roots_from_json(ast_text):
        visit(root, [], "")
    return {"schema": 2, "types": sorted(records.values(), key=lambda x: x["id"]),
            "functions": sorted(functions.values(), key=lambda x: x["id"]),
            "aliases": sorted(aliases.values(), key=lambda x: x["id"])}


def extract(compiler):
    with tempfile.TemporaryDirectory(prefix="gui-forms-api-") as scratch:
        translation = Path(scratch) / "public.cpp"
        translation.write_text("".join('#include "' + str(p.relative_to(ROOT / "include")) + '"\n'
                                       for p in sorted((ROOT / "include/gui_forms").rglob("*.hpp"))))
        result = subprocess.run([compiler, "-std=c++20", "-x", "c++", "-Iinclude", "-fsyntax-only",
            "-Xclang", "-ast-dump=json", "-Xclang", "-ast-dump-filter=gui_", str(translation)],
            cwd=ROOT, capture_output=True, text=True, check=False)
        if result.returncode:
            raise RuntimeError(result.stderr)
        return inventory(result.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang", default="clang++")
    parser.add_argument("--ast", type=Path, help="Read an already captured Clang AST")
    parser.add_argument("--output", type=Path, default=REFERENCE / "site", help="Generated HTML directory")
    parser.add_argument("--check", action="store_true", help="Check the committed declaration inventory and contract schema")
    parser.add_argument("--inventory-only", action="store_true")
    args = parser.parse_args()
    data = inventory(args.ast.read_text()) if args.ast else extract(args.clang)
    REFERENCE.mkdir(exist_ok=True)
    encoded = json.dumps(data, indent=2) + "\n"
    inventory_path = REFERENCE / "inventory.json"
    if args.check:
        if not inventory_path.exists() or inventory_path.read_text() != encoded:
            raise SystemExit("API inventory is stale; run tools/api_reference.py")
    else:
        inventory_path.write_text(encoded)
    from api_reference_render import load_contracts, render
    contracts = load_contracts(REFERENCE / "contracts", data, ROOT)
    if not args.inventory_only and not args.check:
        coverage = render(data, contracts, args.output, ROOT)
        print(str(coverage['reviewed_contracts']) + " reviewed contracts / " + str(coverage['symbols']) + " declarations")
        print("Reference: " + str(args.output / "index.html"))
    print(str(len(data["types"])) + " exact public/protected C++ types; " +
          str(sum(len(t["members"]) for t in data["types"])) + " declared members; " +
          str(len(data["functions"])) + " namespace functions")

if __name__ == "__main__":
    main()

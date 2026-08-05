#!/usr/bin/env python3
"""Apply a reviewable disposition policy to a Capture-0 manifest."""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import pathlib
import sys
from typing import Any

CATALOGUE_SCHEMA = "gui.forms.compat.facade-catalogue/v1"
ALLOWED_DISPOSITIONS = {
    "required", "deferred", "excluded", "application_side_port"
}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_json(path: pathlib.Path) -> tuple[dict[str, Any], bytes]:
    data = path.read_bytes()
    return json.loads(data), data


def stable_id(prefix: str, *parts: object) -> str:
    encoded = "\0".join("" if part is None else str(part) for part in parts)
    return f"{prefix}-{sha256_bytes(encoded.encode('utf-8'))[:24]}"


def match_type(type_name: str, exact: list[str], prefixes: list[str]) -> bool:
    return type_name in exact or any(type_name.startswith(value) for value in prefixes)


def api_owner(row: dict[str, Any], policy: dict[str, Any]) -> str:
    for rule in policy["apiOwnership"]:
        if row["type"].startswith(rule.get("typePrefix", "\0")):
            return rule["owner"]
        if row["targetAssembly"].startswith(rule.get("assemblyPrefix", "\0")):
            return rule["owner"]
    raise ValueError(
        f"no API owner rule matches {row['targetAssembly']}::{row['type']}"
    )


def classify_api(row: dict[str, Any], policy: dict[str, Any]) -> tuple[str, str, str]:
    type_name = row["type"]
    exclusions = policy["exclusions"]
    if match_type(type_name, exclusions["exactTypes"], exclusions["typePrefixes"]):
        return "excluded", "scope_exclusion", exclusions["reason"]

    deferrals = policy["forcedDeferrals"]
    if match_type(type_name, deferrals["exactTypes"], deferrals["typePrefixes"]):
        return "deferred", "design_or_legacy_surface", deferrals["reason"]

    overrides = policy.get("apiOverrides", {})
    key = "::".join(
        [row["targetAssembly"], type_name, row.get("member", ""),
         row.get("signature", "")]
    )
    if key in overrides:
        override = overrides[key]
        return override["disposition"], "explicit_override", override["reason"]

    if row["ilOccurrenceCount"] > 0:
        return (
            "required", "static_il_operand",
            "A compiled method contains at least one operand for this API."
        )
    return (
        "deferred", "metadata_only",
        "The API is present in metadata but has no captured static IL operand."
    )


def api_record(row: dict[str, Any], policy: dict[str, Any]) -> dict[str, Any]:
    disposition, rationale_code, rationale = classify_api(row, policy)
    if disposition not in ALLOWED_DISPOSITIONS:
        raise ValueError(f"invalid API disposition {disposition}")
    owner = api_owner(row, policy)
    return {
        "id": stable_id(
            "api", row["targetAssembly"], row["type"], row.get("member"),
            row.get("signature"), row["memberKind"]
        ),
        "targetAssembly": row["targetAssembly"],
        "type": row["type"],
        **({"member": row["member"]} if "member" in row else {}),
        **({"signature": row["signature"]} if "signature" in row else {}),
        "memberKind": row["memberKind"],
        "operations": row["operations"],
        "ilOccurrenceCount": row["ilOccurrenceCount"],
        "metadataReference": row["metadataReference"],
        "sourceAssemblies": row["sourceAssemblies"],
        "evidence": row["evidence"],
        "disposition": disposition,
        "implementationOwner": owner,
        "rationaleCode": rationale_code,
        "rationale": rationale,
    }


def custom_control_record(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": stable_id("custom", row["privateTypeId"], row["sourceAssembly"]),
        "privateTypeId": row["privateTypeId"],
        "sourceAssembly": row["sourceAssembly"],
        "formsBaseAssembly": row["formsBaseAssembly"],
        "formsBaseType": row["formsBaseType"],
        "inheritanceDepth": row["inheritanceDepth"],
        "evidence": row["evidence"],
        "disposition": "application_side_port",
        "implementationOwner": "specimen_or_plugin",
        "rationaleCode": "consumer_defined_type",
        "rationale": (
            "GUI.Forms supplies the captured public base contract; the private "
            "derived type remains owned by its defining specimen or plugin."
        ),
    }


def native_import_record(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": stable_id(
            "native", row["sourceAssembly"], row["module"], row["entryPoint"],
            row["attributes"]
        ),
        **{key: row[key] for key in (
            "sourceAssembly", "module", "entryPoint", "attributes", "count", "evidence"
        )},
        "disposition": "application_side_port",
        "implementationOwner": "specimen_native_dependency",
        "rationaleCode": "direct_native_import",
        "rationale": (
            "This is a direct specimen/plugin native dependency, not a call into "
            "the GUI.Forms compatibility facade."
        ),
    }


def count_values(rows: list[dict[str, Any]], key: str,
                 universe: set[str] | None = None) -> dict[str, int]:
    counts = collections.Counter(row[key] for row in rows)
    if universe is not None:
        for value in universe:
            counts.setdefault(value, 0)
    return dict(sorted(counts.items()))


def build_catalogue(manifest: dict[str, Any], manifest_bytes: bytes,
                    policy: dict[str, Any], policy_bytes: bytes) -> dict[str, Any]:
    if manifest.get("schema") != "gui.forms.compat.capture/v0":
        raise ValueError("input is not a Capture-0 manifest")
    manifest_hash = sha256_bytes(manifest_bytes)
    expected_manifest_hash = policy["sourceCaptureSha256"]
    if manifest_hash != expected_manifest_hash:
        raise ValueError(
            f"capture SHA-256 {manifest_hash} does not match pinned {expected_manifest_hash}"
        )
    if manifest["specimen"]["sha256"] != policy["specimenSha256"]:
        raise ValueError("capture specimen identity does not match disposition policy")

    api_rows = [api_record(row, policy) for row in manifest["apiUses"]]
    custom_rows = [custom_control_record(row) for row in manifest["customControls"]]
    native_rows = [native_import_record(row) for row in manifest["nativeImports"]]

    ids = [row["id"] for row in api_rows + custom_rows + native_rows]
    if len(ids) != len(set(ids)):
        raise ValueError("generated catalogue IDs are not unique")
    if any(row["disposition"] not in ALLOWED_DISPOSITIONS
           for row in api_rows + custom_rows + native_rows):
        raise ValueError("catalogue contains an unclassified or unknown disposition")

    required_facade_types = sorted({
        row["type"] for row in api_rows
        if row["disposition"] == "required" and
        row["implementationOwner"] == "gui_forms_managed_facade"
    })
    return {
        "schema": CATALOGUE_SCHEMA,
        "toolVersion": "0.1.0",
        "source": {
            "captureSchema": manifest["schema"],
            "captureSha256": manifest_hash,
            "specimenLabel": manifest["specimen"]["label"],
            "specimenSha256": manifest["specimen"]["sha256"],
            "policySchema": policy["schema"],
            "policySha256": sha256_bytes(policy_bytes),
        },
        "policy": {
            "evidenceBoundary": policy["evidenceBoundary"],
            "requiredRule": policy["requiredRule"],
            "metadataOnlyRule": policy["metadataOnlyRule"],
            "customControlRule": policy["customControlRule"],
            "nativeImportRule": policy["nativeImportRule"],
        },
        "apiRows": api_rows,
        "customControlRows": custom_rows,
        "nativeImportRows": native_rows,
        "facade": {"requiredTypes": required_facade_types},
        "summary": {
            "apiRowCount": len(api_rows),
            "customControlRowCount": len(custom_rows),
            "nativeImportRowCount": len(native_rows),
            "totalRowCount": len(api_rows) + len(custom_rows) + len(native_rows),
            "apiByDisposition": count_values(
                api_rows, "disposition", ALLOWED_DISPOSITIONS
            ),
            "apiByOwner": count_values(api_rows, "implementationOwner"),
            "customControlByDisposition": count_values(custom_rows, "disposition"),
            "nativeImportByDisposition": count_values(native_rows, "disposition"),
            "requiredFacadeTypeCount": len(required_facade_types),
            "unclassifiedCount": 0,
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--capture", type=pathlib.Path, required=True)
    parser.add_argument("--policy", type=pathlib.Path, required=True)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args()
    try:
        manifest, manifest_bytes = load_json(args.capture)
        policy, policy_bytes = load_json(args.policy)
        catalogue = build_catalogue(manifest, manifest_bytes, policy, policy_bytes)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(
            json.dumps(catalogue, indent=2, ensure_ascii=True) + "\n", encoding="utf-8"
        )
        print(
            f"facade-catalogue: {catalogue['summary']['totalRowCount']} rows; "
            f"{catalogue['summary']['unclassifiedCount']} unclassified"
        )
        return 0
    except (OSError, KeyError, TypeError, ValueError, json.JSONDecodeError) as error:
        print(f"facade-catalogue: error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

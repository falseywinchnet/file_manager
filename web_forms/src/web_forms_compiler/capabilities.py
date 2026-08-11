from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from .diagnostics import Diagnostic, WebFormsError


CAPABILITY_SCHEMA = "web.forms.gui-capabilities/0.1-experimental"


def read_capabilities(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise WebFormsError([Diagnostic("WFM001", f"cannot read capability manifest: {error}", str(path))]) from error
    if not isinstance(value, dict) or value.get("schema") != CAPABILITY_SCHEMA:
        raise WebFormsError([Diagnostic("WFM002", f"expected capability schema {CAPABILITY_SCHEMA!r}", str(path))])
    return value


def assess_capabilities(ir: dict[str, Any], manifest: dict[str, Any]) -> dict[str, Any]:
    declared = manifest.get("features", {})
    if not isinstance(declared, dict):
        raise WebFormsError([Diagnostic("WFM003", "manifest features must be an object", "<manifest>")])
    results = []
    counts = {"supported": 0, "degraded": 0, "adapter-required": 0, "unavailable": 0, "unknown": 0}
    policies = ir["requirements"].get("feature_policies", {})
    for feature in ir["requirements"].get("features", []):
        entry = declared.get(feature)
        if entry is None:
            classification = "unknown"
            evidence = ""
            fallback = "none"
        else:
            status = entry.get("status", "unknown")
            if status == "supported":
                classification = "supported"
            elif status == "primitive-present":
                classification = "adapter-required"
            elif status == "unavailable":
                policy = policies.get(feature, "required")
                fallback_value = entry.get("fallback", "none")
                if policy != "structural-required" and fallback_value != "none":
                    classification = "degraded"
                else:
                    classification = "unavailable"
            else:
                classification = "unknown"
            evidence = entry.get("evidence", "")
            fallback = entry.get("fallback", "none")
        counts[classification] += 1
        results.append(
            {
                "feature": feature,
                "classification": classification,
                "evidence": evidence,
                "fallback": fallback,
            }
        )
    return {
        "schema": "web.forms.capability-assessment/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "counts": counts,
        "ready_for_native_lowering": counts["adapter-required"] == 0 and counts["unavailable"] == 0 and counts["unknown"] == 0,
        "features": results,
    }

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Diagnostic:
    code: str
    message: str
    path: str
    line: int = 1
    column: int = 1

    def render(self) -> str:
        return f"{self.path}:{self.line}:{self.column}: {self.code}: {self.message}"


class WebFormsError(Exception):
    def __init__(self, diagnostics: list[Diagnostic]):
        self.diagnostics = tuple(diagnostics)
        super().__init__("\n".join(item.render() for item in diagnostics))


def fail(code: str, message: str, path: str, line: int = 1, column: int = 1) -> None:
    raise WebFormsError([Diagnostic(code, message, path, line, column)])


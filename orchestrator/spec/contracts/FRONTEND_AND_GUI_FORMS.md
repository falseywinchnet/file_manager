# ORC-GUI / ORC-FE: frontend and GUI.Forms contracts

Status: **paper outline; GUI.Forms experimental ABI import pending**.

GUI.Forms owns retained control/rendering semantics and its C ABI implementation.
The Oracle registry records the version consumed by the frontend; it does not
move GUI state into the Oracle.

The File Manager frontend owns window, navigation, selection, file-operation
presentation, preview/properties composition, and user interaction state. It
must be able to:

- render cached handler/command declarations without plugin code;
- submit command context as exact object IDs plus immutable selection snapshot;
- receive engine/Oracle results without foreign callbacks on arbitrary threads;
- preserve selection across generation updates;
- display unavailable, stale, partial, denied, and provider-derived states;
- continue basic navigation and file operations while Oracle/plugin services
  restart or are unavailable.

The frontend does not expose GUI.Forms controls to plugins and does not let the
Oracle dictate house rendering.

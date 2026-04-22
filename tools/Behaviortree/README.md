# Behavior Tree XML Viewer

This folder now provides a pure offline HTML viewer for BehaviorTree.CPP XML files.

- No ROS launch required.
- No Groot2 required.
- No runtime logger/env setup required.

## Files

- `bt_xml_viewer.html`: open in browser, load XML, render full tree structure.

## Quick Start

Open directly:

```bash
xdg-open tools/Behaviortree/bt_xml_viewer.html
```

Then click `Choose XML` and select:

- `src/behavior_tree/Scripts/main.xml`
or
- `install/behavior_tree/share/behavior_tree/Scripts/main.xml`

## Optional: Open via local HTTP

```bash
python3 -m http.server 18080
```

Then visit:

- `http://127.0.0.1:18080/tools/Behaviortree/bt_xml_viewer.html`

## Features

- Parse `main_tree_to_execute` automatically.
- Show all `BehaviorTree ID`s in a dropdown.
- Resolve `SubTree ID="..."` references inline.
- Detect recursive subtree cycles.
- Click node or `+/-` to collapse/expand children.
- `Expand All` / `Collapse All`.
- `Collapse To Depth` for fast high-level view.
- Search and highlight matching nodes, with auto-expand path.
- Optional `Show Attributes` switch for cleaner reading.

#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

grep -q 'saveFileToPath(const QString &fileName, QWidget \*document' include/app/texteditor.h
grep -q 'saveFileToPath(ed->getFileName(), ed)' src/app/texteditor_features.cpp
grep -q 'activeTabWidget' include/app/texteditor.h
grep -q 'findOpenDocument(path' src/app/texteditor_documents.cpp
grep -q 'QStringDecoder decoder' src/app/texteditor_documents.cpp
grep -q 'setMarkdownPreviewVisible(bool visible)' src/app/texteditor_panels.cpp
grep -q 'symbolRequested' include/app/search_everywhere.h
grep -q 'validateShortcuts' src/app/texteditor_actions.cpp
grep -q 'QProcessEnvironment::systemEnvironment' src/app/terminal_widget.cpp
grep -q 'rebuildPaintCache' src/editor/codeeditor.cpp
grep -q 'byName.contains(c.target)' src/story/storygraph.cpp
grep -q 'Never fall back to a microphone' src/audio/audiomonitor.cpp
if rg -q 'defaultAudioInput' src/audio/audiomonitor.cpp; then
    echo "DJ Mode must not capture microphone input" >&2
    exit 1
fi

if rg -q 'command_palette|class CommandPalette' jim.pro include/app src/app; then
    echo "dead command-palette implementation remains" >&2
    exit 1
fi

if git ls-files --error-unmatch .qmake.stash Makefile Makefile.Debug Makefile.Release moc_predefs.h >/dev/null 2>&1; then
    echo "generated build files are tracked" >&2
    exit 1
fi

echo "Jim regression invariants: PASS"

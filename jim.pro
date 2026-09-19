QT += core gui widgets network multimedia
TARGET = jim
TEMPLATE = app
RC_ICONS = logo.ico
CONFIG += c++17

OBJECTS_DIR = .build/obj
MOC_DIR = .build/moc
RCC_DIR = .build/rcc
UI_DIR = .build/ui
DESTDIR = .build/bin

INCLUDEPATH += \
    include \
    include/app \
    include/editor \
    include/ai \
    include/audio \
    include/tools/binary \
    include/tools/preview \
    include/tools/visualization \
    include/analysis \
    include/story

win32 {
    LIBS += -lole32 -luuid
}

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += libpulse-simple
}

SOURCES += src/app/main.cpp \
           src/app/breadcrumb_bar.cpp \
           src/app/draggable_tabs.cpp \
           src/app/find_bar.cpp \
           src/app/search_everywhere.cpp \
           src/app/terminal_widget.cpp \
           src/app/texteditor.cpp \
           src/app/texteditor_actions.cpp \
           src/app/texteditor_documents.cpp \
           src/app/texteditor_features.cpp \
           src/app/texteditor_panels.cpp \
           src/app/texteditor_tools.cpp \
           src/app/title_bar.cpp \
           src/app/todo_panel.cpp \
           src/app/welcome_widget.cpp \
           src/editor/animationwidget.cpp \
           src/editor/codeeditor.cpp \
           src/editor/keyheatmap.cpp \
           src/editor/linenumberarea.cpp \
           src/editor/overlays.cpp \
           src/editor/syntaxhighlighter.cpp \
           src/editor/vimmode.cpp \
           src/ai/aiautocomplete.cpp \
           src/ai/aisettingsdialog.cpp \
           src/audio/audiomonitor.cpp \
           src/tools/binary/binaryinspector.cpp \
           src/tools/binary/disassembler.cpp \
           src/tools/binary/hexeditor.cpp \
           src/tools/preview/markdownviewer.cpp \
           src/tools/visualization/cityscape_widget.cpp \
           src/tools/visualization/codegraph.cpp \
           src/analysis/solidityanalyzer.cpp \
           src/analysis/storageslotvisualizer.cpp \
           src/analysis/vulnscanner.cpp \
           src/story/storyexporter.cpp \
           src/story/storygraph.cpp \
           src/story/storyparser.cpp \
           src/story/storyplaytest.cpp
HEADERS += include/app/texteditor.h \
           include/app/breadcrumb_bar.h \
           include/app/draggable_tabs.h \
           include/app/find_bar.h \
           include/app/search_everywhere.h \
           include/app/terminal_widget.h \
           include/app/todo_panel.h \
           include/app/title_bar.h \
           include/app/welcome_widget.h \
           include/editor/animationwidget.h \
           include/editor/codeeditor.h \
           include/editor/folding_area.h \
           include/editor/line_number_area.h \
           include/editor/minimap.h \
           include/editor/overlays.h \
           include/editor/keyheatmap.h \
           include/editor/syntaxhighlighter.h \
           include/editor/vimmode.h \
           include/ai/aiautocomplete.h \
           include/ai/aisettingsdialog.h \
           include/audio/audiomonitor.h \
           include/tools/binary/binaryinspector.h \
           include/tools/binary/disassembler.h \
           include/tools/binary/hexeditor.h \
           include/tools/preview/markdownviewer.h \
           include/tools/visualization/cityscape_widget.h \
           include/tools/visualization/codegraph.h \
           include/analysis/solidityanalyzer.h \
           include/analysis/storageslotvisualizer.h \
           include/analysis/vulnscanner.h \
           include/story/storyexporter.h \
           include/story/storygraph.h \
           include/story/storyparser.h \
           include/story/storyplaytest.h

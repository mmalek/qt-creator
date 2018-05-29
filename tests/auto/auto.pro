TEMPLATE = subdirs

SUBDIRS += \
    algorithm \
    aggregation \
    changeset \
    clangstaticanalyzer \
    cplusplus \
    debugger \
    diff \
    extensionsystem \
    externaltool \
    environment \
    generichighlighter \
    profilewriter \
    rust \
    treeviewfind \
    toolchaincache \
    qtcprocess \
    json \
    utils \
    filesearch \
    mapreduce \
    runextensions \
    sdktool \
    valgrind

qtHaveModule(qml): SUBDIRS += qml
qtHaveModule(quick): SUBDIRS += flamegraph timeline

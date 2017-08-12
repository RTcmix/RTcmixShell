#ifndef UTILS_H
#define UTILS_H

QT_BEGIN_NAMESPACE
class QWidget;
class MainWindow;
QT_END_NAMESPACE

// NB: This comes from the Spectrum Qt example.
// Macro which connects a signal to a slot, and which causes application to
// abort if the connection fails.  This is intended to catch programming errors
// such as mis-typing a signal or slot name.  It is necessary to write our own
// macro to do this - the following idiom
//     Q_ASSERT(connect(source, signal, receiver, slot));
// will not work because Q_ASSERT compiles to a no-op in release builds.

#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if (!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

MainWindow *getMainWindow();

#endif // UTILS_H

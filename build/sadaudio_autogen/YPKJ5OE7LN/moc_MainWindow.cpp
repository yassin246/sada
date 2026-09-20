/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "retranslate",
        "",
        "playOrPause",
        "nextTrack",
        "prevTrack",
        "playQueueRow",
        "row",
        "onEndOfFile",
        "pollPosition",
        "updateNowPlaying",
        "onSearchChanged",
        "text",
        "onFolderActivated",
        "path",
        "onArtistActivated",
        "artist",
        "onClearArtist",
        "onBrowseFolder",
        "onRescan",
        "onLibraryDoubleClicked",
        "QModelIndex",
        "idx",
        "onQueueDoubleClicked",
        "playLibrarySelection",
        "addLibrarySelectionToQueue",
        "onEqGains",
        "QList<float>",
        "gains",
        "toggleRepeat",
        "openFilesDialog",
        "openFolderDialog",
        "savePlaylistDialog",
        "onQueueContextMenu",
        "QPoint",
        "pos",
        "showMini",
        "showAbout",
        "quitApp"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'retranslate'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'playOrPause'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'nextTrack'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'prevTrack'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'playQueueRow'
        QtMocHelpers::SlotData<void(int)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Slot 'onEndOfFile'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'pollPosition'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateNowPlaying'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSearchChanged'
        QtMocHelpers::SlotData<void(const QString &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'onFolderActivated'
        QtMocHelpers::SlotData<void(const QString &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Slot 'onArtistActivated'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'onClearArtist'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBrowseFolder'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRescan'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLibraryDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onQueueDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'playLibrarySelection'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addLibrarySelectionToQueue'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEqGains'
        QtMocHelpers::SlotData<void(const QVector<float> &)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 27, 28 },
        }}),
        // Slot 'toggleRepeat'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openFilesDialog'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openFolderDialog'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'savePlaylistDialog'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onQueueContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 34, 35 },
        }}),
        // Slot 'showMini'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showAbout'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'quitApp'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->retranslate(); break;
        case 1: _t->playOrPause(); break;
        case 2: _t->nextTrack(); break;
        case 3: _t->prevTrack(); break;
        case 4: _t->playQueueRow((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onEndOfFile(); break;
        case 6: _t->pollPosition(); break;
        case 7: _t->updateNowPlaying(); break;
        case 8: _t->onSearchChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->onFolderActivated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onArtistActivated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onClearArtist(); break;
        case 12: _t->onBrowseFolder(); break;
        case 13: _t->onRescan(); break;
        case 14: _t->onLibraryDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 15: _t->onQueueDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 16: _t->playLibrarySelection(); break;
        case 17: _t->addLibrarySelectionToQueue(); break;
        case 18: _t->onEqGains((*reinterpret_cast<std::add_pointer_t<QList<float>>>(_a[1]))); break;
        case 19: _t->toggleRepeat(); break;
        case 20: _t->openFilesDialog(); break;
        case 21: _t->openFolderDialog(); break;
        case 22: _t->savePlaylistDialog(); break;
        case 23: _t->onQueueContextMenu((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 24: _t->showMini(); break;
        case 25: _t->showAbout(); break;
        case 26: _t->quitApp(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<float> >(); break;
            }
            break;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    return _id;
}
QT_WARNING_POP

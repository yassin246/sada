/****************************************************************************
** Meta object code from reading C++ file 'NowPlayingBar.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/NowPlayingBar.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NowPlayingBar.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9BeatMeterE_t {};
} // unnamed namespace

template <> constexpr inline auto BeatMeter::qt_create_metaobjectdata<qt_meta_tag_ZN9BeatMeterE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BeatMeter"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BeatMeter, qt_meta_tag_ZN9BeatMeterE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BeatMeter::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BeatMeterE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BeatMeterE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9BeatMeterE_t>.metaTypes,
    nullptr
} };

void BeatMeter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BeatMeter *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *BeatMeter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BeatMeter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BeatMeterE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int BeatMeter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN13NowPlayingBarE_t {};
} // unnamed namespace

template <> constexpr inline auto NowPlayingBar::qt_create_metaobjectdata<qt_meta_tag_ZN13NowPlayingBarE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NowPlayingBar",
        "playPauseRequested",
        "",
        "prevRequested",
        "nextRequested",
        "seekPreview",
        "seconds",
        "seekRequested",
        "volumeChanged",
        "volume",
        "muteRequested",
        "equalizerRequested",
        "repeatRequested",
        "shuffleRequested",
        "miniRequested",
        "fullPageRequested",
        "updateTimeLabel"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'playPauseRequested'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'prevRequested'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'nextRequested'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'seekPreview'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Signal 'seekRequested'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Signal 'muteRequested'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'equalizerRequested'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'repeatRequested'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'shuffleRequested'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'miniRequested'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fullPageRequested'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateTimeLabel'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NowPlayingBar, qt_meta_tag_ZN13NowPlayingBarE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NowPlayingBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NowPlayingBarE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NowPlayingBarE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13NowPlayingBarE_t>.metaTypes,
    nullptr
} };

void NowPlayingBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NowPlayingBar *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->playPauseRequested(); break;
        case 1: _t->prevRequested(); break;
        case 2: _t->nextRequested(); break;
        case 3: _t->seekPreview((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->seekRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->volumeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->muteRequested(); break;
        case 7: _t->equalizerRequested(); break;
        case 8: _t->repeatRequested(); break;
        case 9: _t->shuffleRequested(); break;
        case 10: _t->miniRequested(); break;
        case 11: _t->fullPageRequested(); break;
        case 12: _t->updateTimeLabel(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::playPauseRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::prevRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::nextRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)(int )>(_a, &NowPlayingBar::seekPreview, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)(int )>(_a, &NowPlayingBar::seekRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)(int )>(_a, &NowPlayingBar::volumeChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::muteRequested, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::equalizerRequested, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::repeatRequested, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::shuffleRequested, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::miniRequested, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (NowPlayingBar::*)()>(_a, &NowPlayingBar::fullPageRequested, 11))
            return;
    }
}

const QMetaObject *NowPlayingBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NowPlayingBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NowPlayingBarE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int NowPlayingBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void NowPlayingBar::playPauseRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void NowPlayingBar::prevRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void NowPlayingBar::nextRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void NowPlayingBar::seekPreview(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void NowPlayingBar::seekRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void NowPlayingBar::volumeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void NowPlayingBar::muteRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void NowPlayingBar::equalizerRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void NowPlayingBar::repeatRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void NowPlayingBar::shuffleRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void NowPlayingBar::miniRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void NowPlayingBar::fullPageRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}
QT_WARNING_POP

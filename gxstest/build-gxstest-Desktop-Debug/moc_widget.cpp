/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../xstoolslib/widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Widget_t {
    QByteArrayData data[15];
    char stringdata0[178];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 12), // "toggleThread"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 14), // "sendWorkAmount"
QT_MOC_LITERAL(4, 36, 10), // "workAmount"
QT_MOC_LITERAL(5, 47, 13), // "sendWorkSpeed"
QT_MOC_LITERAL(6, 61, 9), // "workSpeed"
QT_MOC_LITERAL(7, 71, 19), // "receiveThreadStatus"
QT_MOC_LITERAL(8, 91, 3), // "msg"
QT_MOC_LITERAL(9, 95, 14), // "setThreadState"
QT_MOC_LITERAL(10, 110, 13), // "setWorkAmount"
QT_MOC_LITERAL(11, 124, 12), // "setWorkSpeed"
QT_MOC_LITERAL(12, 137, 15), // "receiveFinished"
QT_MOC_LITERAL(13, 153, 15), // "receiveProgress"
QT_MOC_LITERAL(14, 169, 8) // "workDone"

    },
    "Widget\0toggleThread\0\0sendWorkAmount\0"
    "workAmount\0sendWorkSpeed\0workSpeed\0"
    "receiveThreadStatus\0msg\0setThreadState\0"
    "setWorkAmount\0setWorkSpeed\0receiveFinished\0"
    "receiveProgress\0workDone"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    1,   60,    2, 0x06 /* Public */,
       5,    1,   63,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   66,    2, 0x08 /* Private */,
       9,    0,   69,    2, 0x08 /* Private */,
      10,    1,   70,    2, 0x08 /* Private */,
      11,    1,   73,    2, 0x08 /* Private */,
      12,    0,   76,    2, 0x08 /* Private */,
      13,    2,   77,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   14,    6,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->toggleThread(); break;
        case 1: _t->sendWorkAmount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sendWorkSpeed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->receiveThreadStatus((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->setThreadState(); break;
        case 5: _t->setWorkAmount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->setWorkSpeed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->receiveFinished(); break;
        case 8: _t->receiveProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Widget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Widget::toggleThread)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Widget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Widget::sendWorkAmount)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Widget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Widget::sendWorkSpeed)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Widget.data,
    qt_meta_data_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Widget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Widget::toggleThread()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Widget::sendWorkAmount(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Widget::sendWorkSpeed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
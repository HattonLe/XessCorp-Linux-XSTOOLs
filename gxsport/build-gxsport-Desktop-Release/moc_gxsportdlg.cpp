/****************************************************************************
** Meta object code from reading C++ file 'gxsportdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../gxsportdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gxsportdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_gxsportDlg_t {
    QByteArrayData data[16];
    char stringdata0[189];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_gxsportDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_gxsportDlg_t qt_meta_stringdata_gxsportDlg = {
    {
QT_MOC_LITERAL(0, 0, 10), // "gxsportDlg"
QT_MOC_LITERAL(1, 11, 18), // "ExitButton_clicked"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 20), // "StrobeButton_clicked"
QT_MOC_LITERAL(4, 52, 10), // "OnBtnData0"
QT_MOC_LITERAL(5, 63, 10), // "OnBtnData1"
QT_MOC_LITERAL(6, 74, 10), // "OnBtnData2"
QT_MOC_LITERAL(7, 85, 10), // "OnBtnData3"
QT_MOC_LITERAL(8, 96, 10), // "OnBtnData4"
QT_MOC_LITERAL(9, 107, 10), // "OnBtnData5"
QT_MOC_LITERAL(10, 118, 10), // "OnBtnData6"
QT_MOC_LITERAL(11, 129, 10), // "OnBtnData7"
QT_MOC_LITERAL(12, 140, 11), // "OnBtnStrobe"
QT_MOC_LITERAL(13, 152, 10), // "OnChkCount"
QT_MOC_LITERAL(14, 163, 19), // "OnSelchangeComboLpt"
QT_MOC_LITERAL(15, 183, 5) // "index"

    },
    "gxsportDlg\0ExitButton_clicked\0\0"
    "StrobeButton_clicked\0OnBtnData0\0"
    "OnBtnData1\0OnBtnData2\0OnBtnData3\0"
    "OnBtnData4\0OnBtnData5\0OnBtnData6\0"
    "OnBtnData7\0OnBtnStrobe\0OnChkCount\0"
    "OnSelchangeComboLpt\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_gxsportDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    0,   90,    2, 0x08 /* Private */,
      14,    1,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,

       0        // eod
};

void gxsportDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<gxsportDlg *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->ExitButton_clicked(); break;
        case 1: _t->StrobeButton_clicked(); break;
        case 2: _t->OnBtnData0(); break;
        case 3: _t->OnBtnData1(); break;
        case 4: _t->OnBtnData2(); break;
        case 5: _t->OnBtnData3(); break;
        case 6: _t->OnBtnData4(); break;
        case 7: _t->OnBtnData5(); break;
        case 8: _t->OnBtnData6(); break;
        case 9: _t->OnBtnData7(); break;
        case 10: _t->OnBtnStrobe(); break;
        case 11: _t->OnChkCount(); break;
        case 12: _t->OnSelchangeComboLpt((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject gxsportDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_gxsportDlg.data,
    qt_meta_data_gxsportDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *gxsportDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *gxsportDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_gxsportDlg.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int gxsportDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'gxsloaddlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../gxsloaddlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gxsloaddlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GxsloadDlg_t {
    QByteArrayData data[13];
    char stringdata0[198];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GxsloadDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GxsloadDlg_t qt_meta_stringdata_GxsloadDlg = {
    {
QT_MOC_LITERAL(0, 0, 10), // "GxsloadDlg"
QT_MOC_LITERAL(1, 11, 18), // "ExitButton_clicked"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 19), // "OnSelchangeComboLpt"
QT_MOC_LITERAL(4, 51, 19), // "OnSelchangeCmbBoard"
QT_MOC_LITERAL(5, 71, 22), // "OnSelchangeFlashformat"
QT_MOC_LITERAL(6, 94, 20), // "OnSelchangeRamformat"
QT_MOC_LITERAL(7, 115, 6), // "OnLoad"
QT_MOC_LITERAL(8, 122, 14), // "SelUpdatedFPLD"
QT_MOC_LITERAL(9, 137, 13), // "SelUpdatedRAM"
QT_MOC_LITERAL(10, 151, 13), // "SelUpdatedNON"
QT_MOC_LITERAL(11, 165, 10), // "UploadData"
QT_MOC_LITERAL(12, 176, 21) // "DownloadSelectedFiles"

    },
    "GxsloadDlg\0ExitButton_clicked\0\0"
    "OnSelchangeComboLpt\0OnSelchangeCmbBoard\0"
    "OnSelchangeFlashformat\0OnSelchangeRamformat\0"
    "OnLoad\0SelUpdatedFPLD\0SelUpdatedRAM\0"
    "SelUpdatedNON\0UploadData\0DownloadSelectedFiles"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GxsloadDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    1,   70,    2, 0x08 /* Private */,
       4,    1,   73,    2, 0x08 /* Private */,
       5,    1,   76,    2, 0x08 /* Private */,
       6,    1,   79,    2, 0x08 /* Private */,
       7,    0,   82,    2, 0x08 /* Private */,
       8,    1,   83,    2, 0x08 /* Private */,
       9,    1,   86,    2, 0x08 /* Private */,
      10,    1,   89,    2, 0x08 /* Private */,
      11,    1,   92,    2, 0x08 /* Private */,
      12,    0,   95,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void GxsloadDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GxsloadDlg *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->ExitButton_clicked(); break;
        case 1: _t->OnSelchangeComboLpt((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->OnSelchangeCmbBoard((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->OnSelchangeFlashformat((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->OnSelchangeRamformat((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->OnLoad(); break;
        case 6: _t->SelUpdatedFPLD((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->SelUpdatedRAM((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->SelUpdatedNON((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->UploadData((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->DownloadSelectedFiles(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GxsloadDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_GxsloadDlg.data,
    qt_meta_data_GxsloadDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GxsloadDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GxsloadDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GxsloadDlg.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int GxsloadDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

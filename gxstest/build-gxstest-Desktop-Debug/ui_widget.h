/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayoutComponents;
    QHBoxLayout *horizontalLayoutControlsAndStatus;
    QVBoxLayout *verticalLayoutToggleAndWorkAmount;
    QPushButton *buttonToggleWorker;
    QHBoxLayout *horizontalLayoutWorkAmount;
    QLabel *labelWorkAmount;
    QSpinBox *spinBoxWorkAmount;
    QFrame *line;
    QGridLayout *gridLayoutSpeed;
    QSlider *sliderWorkSpeed;
    QLabel *labelWorkSpeedSlow;
    QLabel *labelWorkSpeedFast;
    QLabel *labelWorkSpeedNormal;
    QLabel *labelWorkSpeed;
    QFrame *line_2;
    QVBoxLayout *verticalLayoutStatus;
    QLabel *labelStatus;
    QLabel *labelWorkerStatus;
    QProgressBar *progressBarWork;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(331, 168);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Widget->sizePolicy().hasHeightForWidth());
        Widget->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(Widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayoutComponents = new QVBoxLayout();
        verticalLayoutComponents->setSpacing(6);
        verticalLayoutComponents->setObjectName(QString::fromUtf8("verticalLayoutComponents"));
        horizontalLayoutControlsAndStatus = new QHBoxLayout();
        horizontalLayoutControlsAndStatus->setSpacing(6);
        horizontalLayoutControlsAndStatus->setObjectName(QString::fromUtf8("horizontalLayoutControlsAndStatus"));
        verticalLayoutToggleAndWorkAmount = new QVBoxLayout();
        verticalLayoutToggleAndWorkAmount->setSpacing(6);
        verticalLayoutToggleAndWorkAmount->setObjectName(QString::fromUtf8("verticalLayoutToggleAndWorkAmount"));
        buttonToggleWorker = new QPushButton(Widget);
        buttonToggleWorker->setObjectName(QString::fromUtf8("buttonToggleWorker"));

        verticalLayoutToggleAndWorkAmount->addWidget(buttonToggleWorker);

        horizontalLayoutWorkAmount = new QHBoxLayout();
        horizontalLayoutWorkAmount->setSpacing(6);
        horizontalLayoutWorkAmount->setObjectName(QString::fromUtf8("horizontalLayoutWorkAmount"));
        labelWorkAmount = new QLabel(Widget);
        labelWorkAmount->setObjectName(QString::fromUtf8("labelWorkAmount"));

        horizontalLayoutWorkAmount->addWidget(labelWorkAmount);

        spinBoxWorkAmount = new QSpinBox(Widget);
        spinBoxWorkAmount->setObjectName(QString::fromUtf8("spinBoxWorkAmount"));
        spinBoxWorkAmount->setCursor(QCursor(Qt::PointingHandCursor));
        spinBoxWorkAmount->setMinimum(1);
        spinBoxWorkAmount->setMaximum(10000);
        spinBoxWorkAmount->setValue(100);
        spinBoxWorkAmount->setDisplayIntegerBase(10);

        horizontalLayoutWorkAmount->addWidget(spinBoxWorkAmount);


        verticalLayoutToggleAndWorkAmount->addLayout(horizontalLayoutWorkAmount);


        horizontalLayoutControlsAndStatus->addLayout(verticalLayoutToggleAndWorkAmount);

        line = new QFrame(Widget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayoutControlsAndStatus->addWidget(line);

        gridLayoutSpeed = new QGridLayout();
        gridLayoutSpeed->setSpacing(6);
        gridLayoutSpeed->setObjectName(QString::fromUtf8("gridLayoutSpeed"));
        sliderWorkSpeed = new QSlider(Widget);
        sliderWorkSpeed->setObjectName(QString::fromUtf8("sliderWorkSpeed"));
        sliderWorkSpeed->setToolTipDuration(-1);
        sliderWorkSpeed->setStyleSheet(QString::fromUtf8(""));
        sliderWorkSpeed->setMinimum(1);
        sliderWorkSpeed->setMaximum(10);
        sliderWorkSpeed->setValue(1);
        sliderWorkSpeed->setSliderPosition(1);
        sliderWorkSpeed->setTracking(true);
        sliderWorkSpeed->setOrientation(Qt::Vertical);
        sliderWorkSpeed->setInvertedAppearance(false);
        sliderWorkSpeed->setInvertedControls(false);
        sliderWorkSpeed->setTickPosition(QSlider::TicksBothSides);
        sliderWorkSpeed->setTickInterval(1);

        gridLayoutSpeed->addWidget(sliderWorkSpeed, 1, 1, 3, 1);

        labelWorkSpeedSlow = new QLabel(Widget);
        labelWorkSpeedSlow->setObjectName(QString::fromUtf8("labelWorkSpeedSlow"));
        labelWorkSpeedSlow->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutSpeed->addWidget(labelWorkSpeedSlow, 1, 0, 1, 1);

        labelWorkSpeedFast = new QLabel(Widget);
        labelWorkSpeedFast->setObjectName(QString::fromUtf8("labelWorkSpeedFast"));
        labelWorkSpeedFast->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutSpeed->addWidget(labelWorkSpeedFast, 3, 0, 1, 1);

        labelWorkSpeedNormal = new QLabel(Widget);
        labelWorkSpeedNormal->setObjectName(QString::fromUtf8("labelWorkSpeedNormal"));
        labelWorkSpeedNormal->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutSpeed->addWidget(labelWorkSpeedNormal, 2, 0, 1, 1);

        labelWorkSpeed = new QLabel(Widget);
        labelWorkSpeed->setObjectName(QString::fromUtf8("labelWorkSpeed"));

        gridLayoutSpeed->addWidget(labelWorkSpeed, 0, 0, 1, 2);


        horizontalLayoutControlsAndStatus->addLayout(gridLayoutSpeed);

        line_2 = new QFrame(Widget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayoutControlsAndStatus->addWidget(line_2);

        verticalLayoutStatus = new QVBoxLayout();
        verticalLayoutStatus->setSpacing(6);
        verticalLayoutStatus->setObjectName(QString::fromUtf8("verticalLayoutStatus"));
        labelStatus = new QLabel(Widget);
        labelStatus->setObjectName(QString::fromUtf8("labelStatus"));
        labelStatus->setAlignment(Qt::AlignCenter);

        verticalLayoutStatus->addWidget(labelStatus);

        labelWorkerStatus = new QLabel(Widget);
        labelWorkerStatus->setObjectName(QString::fromUtf8("labelWorkerStatus"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelWorkerStatus->sizePolicy().hasHeightForWidth());
        labelWorkerStatus->setSizePolicy(sizePolicy1);
        QFont font;
        font.setPointSize(14);
        labelWorkerStatus->setFont(font);
        labelWorkerStatus->setFrameShape(QFrame::NoFrame);
        labelWorkerStatus->setAlignment(Qt::AlignCenter);

        verticalLayoutStatus->addWidget(labelWorkerStatus);


        horizontalLayoutControlsAndStatus->addLayout(verticalLayoutStatus);


        verticalLayoutComponents->addLayout(horizontalLayoutControlsAndStatus);

        progressBarWork = new QProgressBar(Widget);
        progressBarWork->setObjectName(QString::fromUtf8("progressBarWork"));
        progressBarWork->setValue(0);

        verticalLayoutComponents->addWidget(progressBarWork);


        gridLayout->addLayout(verticalLayoutComponents, 0, 0, 1, 1);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "QObject QThread Demo", nullptr));
        buttonToggleWorker->setText(QCoreApplication::translate("Widget", "Start", nullptr));
        labelWorkAmount->setText(QCoreApplication::translate("Widget", "Work amount:", nullptr));
#if QT_CONFIG(tooltip)
        spinBoxWorkAmount->setToolTip(QCoreApplication::translate("Widget", "<html><head/><body><p align=\"center\"><span style=\" font-weight:600;\">Work amount</span></p><p align=\"justify\"><span style=\" font-size:8pt;\">Determines how many items will be processed. This number has to be between:</span></p><ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent:1;\"><li align=\"justify\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">minimum</span><span style=\" font-size:8pt;\"> = 1 item</span></li><li align=\"justify\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">maximum</span><span style=\" font-size:8pt;\"> = 10 000 items</span></li></ul></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        sliderWorkSpeed->setToolTip(QCoreApplication::translate("Widget", "<html><head/><body><p align=\"center\"><span style=\" font-weight:600;\">Work speed</span></p><p align=\"justify\"><span style=\" font-size:8pt;\">Determines how fast the given work amount is processed. Three levels are available:</span></p><ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent:1;\"><li align=\"justify\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">fast</span><span style=\" font-size:8pt;\"> = 10ms per item</span></li><li align=\"justify\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">normal</span><span style=\" font-size:8pt;\"> = 50ms per item</span></li><li align=\"justify\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style="
                        "\" font-size:8pt; font-weight:600;\">slow</span><span style=\" font-size:8pt;\"> = 100ms per item</span></li></ul><p align=\"justify\"><span style=\" font-size:8pt;\">Each tick of the slider is multiplied by 10 starting from 1 to 10 as possible ticks</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        labelWorkSpeedSlow->setText(QCoreApplication::translate("Widget", "slow", nullptr));
        labelWorkSpeedFast->setText(QCoreApplication::translate("Widget", "fast", nullptr));
        labelWorkSpeedNormal->setText(QCoreApplication::translate("Widget", "normal", nullptr));
        labelWorkSpeed->setText(QCoreApplication::translate("Widget", "Work speed:", nullptr));
        labelStatus->setText(QCoreApplication::translate("Widget", "Status:", nullptr));
        labelWorkerStatus->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H

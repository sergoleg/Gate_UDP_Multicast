/********************************************************************************
** Form generated from reading UI file 'RecvUdp.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECVUDP_H
#define UI_RECVUDP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QRecvUdpClass
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *pMainLayout;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QComboBox *pIfBox;
    QLabel *label;
    QLineEdit *pHostEdit;
    QLabel *label_2;
    QLineEdit *pPortEdit;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *pShortCheck;
    QCheckBox *pWriteCheck;
    QLineEdit *pPathEdit;
    QToolButton *pSelectPathBtn;
    QPushButton *pProcessBtn;
    QFrame *line;
    QListWidget *pOutList;

    void setupUi(QWidget *QRecvUdpClass)
    {
        if (QRecvUdpClass->objectName().isEmpty())
            QRecvUdpClass->setObjectName(QStringLiteral("QRecvUdpClass"));
        QRecvUdpClass->resize(658, 380);
        verticalLayoutWidget = new QWidget(QRecvUdpClass);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 661, 381));
        pMainLayout = new QVBoxLayout(verticalLayoutWidget);
        pMainLayout->setSpacing(6);
        pMainLayout->setContentsMargins(11, 11, 11, 11);
        pMainLayout->setObjectName(QStringLiteral("pMainLayout"));
        pMainLayout->setContentsMargins(10, 9, 10, 10);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout->addWidget(label_3);

        pIfBox = new QComboBox(verticalLayoutWidget);
        pIfBox->setObjectName(QStringLiteral("pIfBox"));

        horizontalLayout->addWidget(pIfBox);

        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        pHostEdit = new QLineEdit(verticalLayoutWidget);
        pHostEdit->setObjectName(QStringLiteral("pHostEdit"));

        horizontalLayout->addWidget(pHostEdit);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        pPortEdit = new QLineEdit(verticalLayoutWidget);
        pPortEdit->setObjectName(QStringLiteral("pPortEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pPortEdit->sizePolicy().hasHeightForWidth());
        pPortEdit->setSizePolicy(sizePolicy);
        pPortEdit->setMaximumSize(QSize(120, 16777215));

        horizontalLayout->addWidget(pPortEdit);

        horizontalLayout->setStretch(3, 1);

        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        pShortCheck = new QCheckBox(verticalLayoutWidget);
        pShortCheck->setObjectName(QStringLiteral("pShortCheck"));
        pShortCheck->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pShortCheck->sizePolicy().hasHeightForWidth());
        pShortCheck->setSizePolicy(sizePolicy1);
        pShortCheck->setChecked(true);

        horizontalLayout_5->addWidget(pShortCheck);

        pWriteCheck = new QCheckBox(verticalLayoutWidget);
        pWriteCheck->setObjectName(QStringLiteral("pWriteCheck"));
        sizePolicy1.setHeightForWidth(pWriteCheck->sizePolicy().hasHeightForWidth());
        pWriteCheck->setSizePolicy(sizePolicy1);

        horizontalLayout_5->addWidget(pWriteCheck);

        pPathEdit = new QLineEdit(verticalLayoutWidget);
        pPathEdit->setObjectName(QStringLiteral("pPathEdit"));

        horizontalLayout_5->addWidget(pPathEdit);

        pSelectPathBtn = new QToolButton(verticalLayoutWidget);
        pSelectPathBtn->setObjectName(QStringLiteral("pSelectPathBtn"));

        horizontalLayout_5->addWidget(pSelectPathBtn);


        verticalLayout->addLayout(horizontalLayout_5);


        horizontalLayout_4->addLayout(verticalLayout);

        pProcessBtn = new QPushButton(verticalLayoutWidget);
        pProcessBtn->setObjectName(QStringLiteral("pProcessBtn"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pProcessBtn->sizePolicy().hasHeightForWidth());
        pProcessBtn->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(pProcessBtn);


        pMainLayout->addLayout(horizontalLayout_4);

        line = new QFrame(verticalLayoutWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        pMainLayout->addWidget(line);

        pOutList = new QListWidget(verticalLayoutWidget);
        pOutList->setObjectName(QStringLiteral("pOutList"));
        pOutList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        pOutList->setProperty("showDropIndicator", QVariant(false));
        pOutList->setSelectionBehavior(QAbstractItemView::SelectRows);
        pOutList->setViewMode(QListView::ListMode);

        pMainLayout->addWidget(pOutList);

        pMainLayout->setStretch(2, 1);
        QWidget::setTabOrder(pIfBox, pHostEdit);
        QWidget::setTabOrder(pHostEdit, pPortEdit);
        QWidget::setTabOrder(pPortEdit, pShortCheck);
        QWidget::setTabOrder(pShortCheck, pWriteCheck);
        QWidget::setTabOrder(pWriteCheck, pPathEdit);
        QWidget::setTabOrder(pPathEdit, pSelectPathBtn);
        QWidget::setTabOrder(pSelectPathBtn, pProcessBtn);
        QWidget::setTabOrder(pProcessBtn, pOutList);

        retranslateUi(QRecvUdpClass);

        QMetaObject::connectSlotsByName(QRecvUdpClass);
    } // setupUi

    void retranslateUi(QWidget *QRecvUdpClass)
    {
        QRecvUdpClass->setWindowTitle(QApplication::translate("QRecvUdpClass", "QRecvUdp", nullptr));
        label_3->setText(QApplication::translate("QRecvUdpClass", "Interface : ", nullptr));
        label->setText(QApplication::translate("QRecvUdpClass", "Host :", nullptr));
        label_2->setText(QApplication::translate("QRecvUdpClass", "Port :", nullptr));
        pShortCheck->setText(QApplication::translate("QRecvUdpClass", "short log", nullptr));
        pWriteCheck->setText(QApplication::translate("QRecvUdpClass", "write to :", nullptr));
#ifndef QT_NO_TOOLTIP
        pSelectPathBtn->setToolTip(QApplication::translate("QRecvUdpClass", "select write path", nullptr));
#endif // QT_NO_TOOLTIP
        pSelectPathBtn->setText(QApplication::translate("QRecvUdpClass", "...", nullptr));
        pProcessBtn->setText(QApplication::translate("QRecvUdpClass", "Start", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QRecvUdpClass: public Ui_QRecvUdpClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RECVUDP_H

#ifndef SIGNALPROPERTYDIALOG_H
#define SIGNALPROPERTYDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>

#include "../lib/Hash.h"

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const               SignalPropertyGroup[] =
{
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Signal ID"),
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Position"),
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Input physical range: "),
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Input electric range: "),
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Output physical range: "),
                                QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Output electric range: "),
};

const int                       SIGNAL_PROPERTY_GROUP_COUNT         = sizeof(SignalPropertyGroup)/sizeof(SignalPropertyGroup[0]);

const int                       SIGNAL_PROPERTY_GROUP_ID            = 0,
                                SIGNAL_PROPERTY_GROUP_POSITION      = 1,
                                SIGNAL_PROPERTY_GROUP_IN_PH_RANGE   = 2,
                                SIGNAL_PROPERTY_GROUP_IN_EL_RANGE   = 3,
                                SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE  = 4,
                                SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE  = 5;

// ==============================================================================================

const int                       SIGNAL_PROPERTY_ITEM_CUSTOM_ID              = 0,
                                SIGNAL_PROPERTY_ITEM_CAPTION                = 1,
                                SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_LOW        = 2,
                                SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_HIGH       = 3,
                                SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_UNIT       = 4,
                                SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_PRECISION  = 5,
                                SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_LOW        = 6,
                                SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_HIGH       = 7,
                                SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_UNIT       = 8,
                                SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_SENSOR     = 9,
                                SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_PRECISION  = 10,
                                SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_LOW       = 11,
                                SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_HIGH      = 12,
                                SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_UNIT      = 13,
                                SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_PRECISION = 14,
                                SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_LOW       = 15,
                                SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_HIGH      = 16,
                                SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_UNIT      = 17,
                                SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_SENSOR    = 18,
                                SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_PRECISION = 19;

const int                       SIGNAL_PROPERTY_ITEM_COUNT                  = 20;


// ==============================================================================================

class SignalPropertyDialog : public QDialog
{
    Q_OBJECT

public:
	explicit                    SignalPropertyDialog(const Metrology::SignalParam& param, QWidget *parent = 0);
                                ~SignalPropertyDialog();

	Metrology::SignalParam		param() const { return m_param; }

private:

	Metrology::SignalParam      m_param;

    // Property list
    //
    QtVariantPropertyManager*   m_pManager = nullptr;
    QtVariantEditorFactory*     m_pFactory = nullptr;
    QtTreePropertyBrowser*      m_pEditor = nullptr;

    // buttons
    //
    QDialogButtonBox*           m_buttonBox = nullptr;

    static bool                 m_showGroupHeader[SIGNAL_PROPERTY_GROUP_COUNT];
    QtBrowserItem*              m_browserItemList[SIGNAL_PROPERTY_GROUP_COUNT];

    QMap<QtProperty*,int>       m_propertyMap;

    QtProperty*                 m_propertyGroupList[SIGNAL_PROPERTY_GROUP_COUNT];

    void                        createPropertyList();

    void                        updateGroupHeader(int index);

signals:

private slots:

    void                        onPropertyValueChanged(QtProperty *property, const QVariant &value);
    void                        onPropertyExpanded(QtBrowserItem *item);

    void                        onOk();
};

// ==============================================================================================

#endif // SIGNALPROPERTYDIALOG_H

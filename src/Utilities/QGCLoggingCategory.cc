/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCLoggingCategory.h"

#include <QtCore/QGlobalStatic>
#include <QtCore/QSettings>

static constexpr const char* kVideoAllLogCategory = "VideoAllLog";

// Add Global logging categories (not class specific) here using QGC_LOGGING_CATEGORY
QGC_LOGGING_CATEGORY(FirmwareUpgradeLog, "FirmwareUpgradeLog")
QGC_LOGGING_CATEGORY(FirmwareUpgradeVerboseLog, "FirmwareUpgradeVerboseLog")
QGC_LOGGING_CATEGORY(MissionCommandsLog, "MissionCommandsLog")
QGC_LOGGING_CATEGORY(GuidedActionsControllerLog, "GuidedActionsControllerLog")
QGC_LOGGING_CATEGORY(VideoAllLog, kVideoAllLogCategory)

Q_GLOBAL_STATIC(QGCLoggingCategoryRegister, _QGCLoggingCategoryRegisterInstance);

QGCLoggingCategoryRegister *QGCLoggingCategoryRegister::instance()
{
    return _QGCLoggingCategoryRegisterInstance();
}

QStringList QGCLoggingCategoryRegister::registeredCategories()
{
    _registeredCategories.sort();
    return _registeredCategories;
}

void QGCLoggingCategoryRegister::setCategoryLoggingOn(const QString &category, bool enable) const
{
    QSettings settings;

    settings.beginGroup(_filterRulesSettingsGroup);
    settings.setValue(category, enable);

    settings.endGroup();
}

bool QGCLoggingCategoryRegister::categoryLoggingOn(const QString &category) const
{
    QSettings settings;

    settings.beginGroup(_filterRulesSettingsGroup);
    return settings.value(category, false).toBool();
}

void QGCLoggingCategoryRegister::setFilterRulesFromSettings(const QString &commandLineLoggingOptions)
{
    QString filterRules;
    QString filterRuleFormat("%1.debug=true\n");
    bool videoAllLogSet = false;

    if (!commandLineLoggingOptions.isEmpty()) {
        _commandLineLoggingOptions = commandLineLoggingOptions;
    }

    filterRules += "*Log.debug=false\n";
    filterRules += "qgc.*.debug=false\n";

    // Set up filters defined in settings
    for (const QString &category : _registeredCategories) {
        if (categoryLoggingOn(category)) {
            filterRules += filterRuleFormat.arg(category);
            if (category == kVideoAllLogCategory) {
                videoAllLogSet = true;
            }
        }
    }

    // Command line rules take precedence, so they go last in the list
    if (!_commandLineLoggingOptions.isEmpty()) {
        const QStringList logList = _commandLineLoggingOptions.split(",");

        if (logList[0] == "full") {
            filterRules += "*Log.debug=true\n";
            for (const QString &log : logList) {
                filterRules += filterRuleFormat.arg(log);
            }
        } else {
            for (const QString &category: logList) {
                filterRules += filterRuleFormat.arg(category);
                if (category == kVideoAllLogCategory) {
                    videoAllLogSet = true;
                }
            }
        }
    }

    if (videoAllLogSet) {
        filterRules += filterRuleFormat.arg("qgc.videomanager.videomanager");
        filterRules += filterRuleFormat.arg("qgc.videomanager.subtitlewriter");
        filterRules += filterRuleFormat.arg("qgc.videomanager.videoreceiver.gstreamer");
        filterRules += filterRuleFormat.arg("qgc.videomanager.videoreceiver.gstreamer.gstvideoreceiver");
        filterRules += filterRuleFormat.arg("qgc.videomanager.videoreceiver.qtmultimedia.qtmultimediareceiver");
        filterRules += filterRuleFormat.arg("qgc.videomanager.videoreceiver.qtmultimedia.uvcreceiver");
    }

    // Logging from GStreamer library itself controlled by gstreamer debug levels is always turned on
    filterRules += filterRuleFormat.arg("qgc.videomanager.videoreceiver.gstreamer.api");

    filterRules += QStringLiteral("qt.qml.connections=false");

    qDebug() << "Filter rules" << filterRules;
    QLoggingCategory::setFilterRules(filterRules);
}

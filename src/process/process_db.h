/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:      maojj <maojunjie@uniontech.com>
* Maintainer:  maojj <maojunjie@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PROCESS_DB_H
#define PROCESS_DB_H

#include "common/error_context.h"
#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "system/system_monitor.h"
#include "process_set.h"

#include <QReadWriteLock>
#include <QObject>

#include <memory>

#include <unistd.h>
#include <signal.h>

namespace core {
namespace wm {
class WMWindowList;
}
} // namespace core

using namespace core::wm;

namespace core {
namespace process {

class GuiAppsCache;
class TrayAppsCache;
class DesktopEntryCache;
class ProcessSet;

class ProcessDB : public QObject
{
    Q_OBJECT

public:
    explicit ProcessDB(QObject *parent = nullptr);
    virtual ~ProcessDB();

    static ProcessDB *instance();

    WMWindowList *windowList();
    GuiAppsCache *guiAppsCache();
    TrayAppsCache *trayAppsCache();
    DesktopEntryCache *desktopEntryCache();

    static bool isCurrentProcess(pid_t pid);

public slots:
    void setFilterType(FilterType filter);
    void endProcess(pid_t pid);
    void pauseProcess(pid_t pid);
    void resumeProcess(pid_t pid);
    void killProcess(pid_t pid);
    ErrorContext setProcessPriority(pid_t pid, int priority);

Q_SIGNALS:
    void processEnded(pid_t pid);
    void processPaused(pid_t pid, char state);
    void processResumed(pid_t pid, char state);
    void processKilled(pid_t pid);
    void processPriorityChanged(pid_t pid, int priority);
    void priorityPromoteResultReady(const ErrorContext &ec);
    void processControlResultReady(const ErrorContext &ec);
    void filterTypeChanged(FilterType filter);

public:
    void update();

private:
    void sendSignalToProcess(pid_t pid, int signal);
    void filterProcess(const QList<Process> &procList);

private:
    QReadWriteLock m_rwlock;

    WMWindowList *m_windowList;
    GuiAppsCache *m_guiAppsCache;
    TrayAppsCache *m_trayAppsCache;
    DesktopEntryCache *m_desktopEntryCache;

    ProcessSet *m_procSet;
};

inline ProcessDB *ProcessDB::instance()
{
    auto *thread = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread);
    return thread->systemMonitorInstance()->processDB();
}

inline GuiAppsCache *ProcessDB::guiAppsCache()
{
    QReadLocker lock(&m_rwlock);
    return m_guiAppsCache;
}

inline TrayAppsCache *ProcessDB::trayAppsCache()
{
    QReadLocker lock(&m_rwlock);
    return m_trayAppsCache;
}

inline WMWindowList *ProcessDB::windowList()
{
    QReadLocker lock(&m_rwlock);
    return m_windowList;
}

inline DesktopEntryCache *ProcessDB::desktopEntryCache()
{
    QReadLocker lock(&m_rwlock);
    return m_desktopEntryCache;
}

inline bool ProcessDB::isCurrentProcess(pid_t pid)
{
    pid_t cur = getpid();
    return pid == cur;
}

} // namespace process
} // namespace core

#endif // PROCESS_DB_H
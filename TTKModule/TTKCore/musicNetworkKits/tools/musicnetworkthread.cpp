#include "musicnetworkthread.h"
#include "musicconnectionpool.h"
#include "musicsettingmanager.h"
#include "ttkconcurrent.h"

#include <QHostInfo>

#define NETWORK_DETECT_INTERVAL     5000             // second
#define NETWORK_REQUEST_ADDRESS     "www.baidu.com"  // ip

MusicNetworkThread::MusicNetworkThread()
    : QObject(nullptr),
      m_networkState(true)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(networkStateChanged()));

    G_CONNECTION_PTR->setValue(className(), this);
}

MusicNetworkThread::~MusicNetworkThread()
{
    m_timer.stop();
}

void MusicNetworkThread::start()
{
    TTK_INFO_STREAM("Load NetworkThread");
    m_timer.start(NETWORK_DETECT_INTERVAL);
    networkStateChanged();
}

void MusicNetworkThread::setBlockNetWork(int block)
{
    G_SETTING_PTR->setValue(MusicSettingManager::CloseNetWorkMode, block);
}

void MusicNetworkThread::networkStateChanged()
{
    TTKConcurrent(
    {
        const bool block = G_SETTING_PTR->value(MusicSettingManager::CloseNetWorkMode).toBool();
        const QHostInfo &info = QHostInfo::fromName(NETWORK_REQUEST_ADDRESS);
        m_networkState = !info.addresses().isEmpty();
        m_networkState = block ? false : m_networkState;
        Q_EMIT networkConnectionStateChanged(m_networkState);
    });
}

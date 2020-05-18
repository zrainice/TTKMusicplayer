#include "musicradiochannelthread.h"
#///QJson import
#include "qjson/parser.h"
#///Sync import
#include "qsync/qsyncutils.h"

#include <QNetworkRequest>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>

MusicRadioChannelThread::MusicRadioChannelThread(QObject *parent, QNetworkCookieJar *cookie)
    : MusicRadioThreadAbstract(parent, cookie)
{

}

MusicRadioChannelThread::~MusicRadioChannelThread()
{
    deleteAll();
}

void MusicRadioChannelThread::startToDownload(const QString &id)
{
    Q_UNUSED(id);
    m_manager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    request.setUrl(QUrl("https://api.douban.com/v2/fm/app_channels"));
#ifndef QT_NO_SSL
    connect(m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    MusicObject::setSslConfiguration(&request);
#endif
    if(m_cookJar)
    {
        m_manager->setCookieJar(m_cookJar);
        m_cookJar->setParent(nullptr);
    }

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
}

void MusicRadioChannelThread::downLoadFinished()
{
    if(!m_reply)
    {
        deleteAll();
        return;
    }

    if(m_reply->error() == QNetworkReply::NoError)
    {
        const QByteArray &bytes = m_reply->readAll();
        m_channels.clear();

        QJson::Parser parser;
        bool ok;
        const QVariant &data = parser.parse(bytes, &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            const QVariantList &groups = value["groups"].toList();
            for(int i=0; i<groups.count(); ++i)
            {
                value = groups[i].toMap();
                const int group = value["group_id"].toInt();
                if(group != 4 && group != 5 && group != 6)
                {
                    continue;
                }

                const QVariantList &channels = value["chls"].toList();
                for(int i=0; i<channels.count(); ++i)
                {
                    value = channels[i].toMap();
                    MusicRadioChannelInfo channel;
                    channel.m_id = value["id"].toString();
                    channel.m_name = value["name"].toString();
                    channel.m_coverUrl = value["cover"].toString();
                    m_channels << channel;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged("query finished!");
    deleteAll();
}

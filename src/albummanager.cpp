// Copyright 2016 The PiCollectionSaver Authors. All rights reserved.
// Use of this source code is governed by a GNU V3.0 license that can be
// found in the LICENSE file.

#include "albummanager.h"
#include <QFile>

#include "errhlpr.h"
#include "CommonUtils.h"

QSharedPointer<IAlbmMngr> IAlbmMngrCtr(
        ISite *pSite, IMainLog *pLog, const QString &strFolder,
        const QString &strUserId,
        const std::shared_ptr<IHtmlPageElm> &htmlElmUserMain,
        HttpDownloader &pHttpDown)
{
    QSharedPointer<IAlbmMngr> retVal(new AlbumManager(pSite, pLog, strFolder,
                                                      strUserId,
                                                      htmlElmUserMain,
                                                      pHttpDown));
    return retVal;
}

AlbumManager::AlbumManager(ISite *pSite, IMainLog *pLog,
                           const QString &strFolder, const QString &strUserId,
                           const std::shared_ptr<IHtmlPageElm> &htmlElmUserMain,
                           HttpDownloader &pHttpDown)
    : m_pSite(pSite), m_pLog(pLog), m_strFolder(strFolder),
      m_strUserId(strUserId), m_htmlElmUsrMain(htmlElmUserMain),
      m_pHttpDown(pHttpDown)
{
    if (m_strFolder.lastIndexOf('/') != m_strFolder.size() - 1) {
        m_strFolder += '/';
    }
}

qListPairOf2Str AlbumManager::GetMissingPicPageUrlLst()
{
    qListPairOf2Str lstUrlFileName;

    LogOut(QString("Start to process album of user %1(%2) with total %3, hide"
                   " %4, limited %5 pics")
           .arg(m_strUserId).arg(QsFrWs(m_htmlElmUsrMain->GetUserName()))
           .arg(m_htmlElmUsrMain->GetTotalPhotoCount())
           .arg(m_htmlElmUsrMain->GetHidePhotoCount())
           .arg(m_htmlElmUsrMain->GetViewLimitedPhotoCount()));

    QString strAlbumLink = QsFrWs(m_htmlElmUsrMain->GetFirstCommonAlbumUrl());
    if (strAlbumLink.isEmpty()) {
        strAlbumLink = QsFrWs(m_pSite->UrlBldr()->GetCommonAlbumUrlById(
                                  m_strUserId.toStdWString()));

        LogOut(QString("WARNING - empty common album for user %1(%2). "
                       "Try direct common album link %3")
               .arg(m_strUserId).arg(QsFrWs(m_htmlElmUsrMain->GetUserName()))
               .arg(strAlbumLink));
    }
    for (;
        strAlbumLink != "" ;
        strAlbumLink = QsFrWs(m_htmlElmUsrMain->GetNextCommonAlbumUrl(
                                  strAlbumLink.toStdWString())))
    {
        QByteArray byteRep = m_pHttpDown.DownloadSync(QUrl(strAlbumLink));
        QString strRep = CommonUtils::Win1251ToQstring(byteRep);

        if (strRep.isEmpty()) {
            if (CErrHlpr::IgnMsgBox("Could not download album page for ID "
                                    + m_strUserId, "URL: " + strAlbumLink)) {
                continue;
            }
            return qListPairOf2Str();
        }

        auto htmlAlbumPage(m_pSite->HtmlPageElmCtr(strRep));
        auto lstUserAlbumPic(htmlAlbumPage->GetPicPageUrlsListByImageIdOnly());
        if (lstUserAlbumPic.empty()) {
            break;
        }

        int iSzBeforeForeach(lstUrlFileName.size());

        for(const std::wstring &wStrPicUrl: lstUserAlbumPic) {
            auto strUrl = QsFrWs(wStrPicUrl);
            auto strFile = QsFrWs(m_pSite->FileNameBldr()->GetPicFileName(
                                      m_strUserId.toStdWString(),
                                      m_pSite->UrlBldr()->
                                      GetPicIdFromUrl(strUrl.toStdWString())));

            QFile file(m_strFolder + strFile);
            if (!file.exists()) {
                Q_ASSERT(!strUrl.isEmpty() && !strFile.isEmpty());
                lstUrlFileName.append(qMakePair(strUrl, strFile));
            } else {
                LogOut("The " + file.fileName()
                       + " exist, break parsing album pages");
                break;
            }
        }
        int iSize = lstUserAlbumPic.size();
        if (iSize != lstUrlFileName.size() - iSzBeforeForeach){
            break;
        } else {
            LogOut("User id " + m_strUserId +
                   " switch to next album page(or end)");
        }
    }

    LogOut(QString("Finished(ID %1). %2 from %3 pictures are missing local")
           .arg(m_strUserId).arg(lstUrlFileName.size())
           .arg(m_htmlElmUsrMain->GetTotalPhotoCount()));

    return lstUrlFileName;
}

void AlbumManager::LogOut(const QString &strMessage)
{
    if (m_pLog) {
        m_pLog->LogOut("---[AlbumManager] " + strMessage);
    }
}

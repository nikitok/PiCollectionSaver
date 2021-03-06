// Copyright 2016 The PiCollectionSaver Authors. All rights reserved.
// Use of this source code is governed by a GNU V3.0 license that can be
// found in the LICENSE file.

#pragma once
#include <QMainWindow>
#include "ui_picollectionsaver.h"

#include <atomic>
#include "topleveiInterfaces.h"
#include "previewpictable.h"


class PiCollectionSaver
        : public QMainWindow, IMainLog, IWorkDir, IFileSavedCallback
{
    Q_OBJECT

public:
    explicit PiCollectionSaver(QWidget *parent = 0);
    ~PiCollectionSaver();
    void LogOut(const QString &strMessage);
    const QString &GetWD() const;
    void FileSaved(const QString &strPath);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::PiCollectionSaverClass ui;

    QSharedPointer<ISqLitePicPreview> m_previewSqLiteCache;
    QSharedPointer<PreviewPicTable> m_previewDownload;
    QSharedPointer<PreviewPicTable> m_previewBrowse;

    QString m_strSiteType;
    // 0-stopped, 1-processing, 2-request to stop:
    std::atomic<int> m_iProccessing;
    void StartProcessing();
    void StopProcessing();
    void ProcessingStopped();
    bool IsStopProcessing();
    bool IsProcessing();

    void ProcessAllUsers(bool bFavorite, bool bEmptyActivityTime);

private slots:
    void slotStartStopAll(void);
    void on_actionAdd_new_user_s_triggered();
    void on_actionDownload_photo_by_ID_triggered();

    void slotPicViewerItemSelectionChanged();
    void slotPicViewerReturnPressed();

    void slotTabChanged(int iIndex);
    void slotTextBrowserPicAnchorClicked(const QUrl &link);
};

#include "ExtractArchiveManager.h"
#include <QCoreApplication>
#include <QThreadPool>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDebug>

ExtractArchiveManager::ExtractArchiveManager(QObject *parent) : QObject(parent)
{
    m_p7ZExtract = nullptr;
    m_type = FILE_TYPE_NOT_SUPPORT;
    m_tempPath = QCoreApplication::applicationDirPath() + "/temp";

    QDir desDir = QDir(m_tempPath);

    if(!desDir.exists()){
        desDir.mkpath(m_tempPath);
    }
}

ExtractArchiveManager::~ExtractArchiveManager()
{
    QDir desDir(m_tempPath);
    desDir.removeRecursively();
}

void ExtractArchiveManager::startExtractArchive(const QString &path)
{
    m_srcPath = path;
    QFileInfo info(path);

    if(m_srcPath.isEmpty()) {
        qDebug() << "src path is empty";
        return;
    }
    else {
        m_desPath = m_tempPath + "/" + info.baseName();
        QDir desDir = QDir(m_desPath);

        if(!desDir.exists()){
            desDir.mkpath(m_desPath);
        }
    }

    if("7z" == info.suffix()) {
        m_type = FILE_TYPE_7Z;
        extractArchive7z();
    }
    else if("zip" == info.suffix()) {
        m_type = FILE_TYPE_ZIP;
        extractArchiveZip();
    }
    else if("rar" == info.suffix()) {
        m_type = FILE_TYPE_RAR;
        extractArchiveRar();
    }
    else {
        m_type = FILE_TYPE_NOT_SUPPORT;
        qDebug() << "file type is not support extract";
    }
}

QString ExtractArchiveManager::GetDesPath()
{
    return m_desPath;
}

void ExtractArchiveManager::onDealProgress(qint64 completed, qint64 total)
{
    int count = 100 * completed / total;
    qDebug() << "progress:" << count << "%";
}

void ExtractArchiveManager::onFinished()
{
    qDebug() << "finished";

    m_pFile->deleteLater();
}

void ExtractArchiveManager::extractArchive7z()
{
//    Lib7z::init();

//    QString filePath = QCoreApplication::applicationDirPath() + "/BiuBiuBiu.7z";
//    qDebug() << filePath;

//    qDebug() << Lib7z::isSupportedArchive(QCoreApplication::applicationDirPath() + "/invaild.7z");
//    qDebug() << Lib7z::isSupportedArchive(filePath);

//    QFile file(filePath);
//    file.open(QFile::ReadOnly);
//    Lib7z::extractArchive(&file,QDir::currentPath());

    Lib7z::init();

    m_p7ZExtract = new Lib7z::ExtractItemJob();
    connect(m_p7ZExtract, &Lib7z::ExtractItemJob::progress, this, &ExtractArchiveManager::onDealProgress);
    connect(m_p7ZExtract, &Lib7z::ExtractItemJob::finished, this, &ExtractArchiveManager::onFinished);

    qDebug() << Lib7z::isSupportedArchive(m_srcPath);

//    QFile srcFile(m_srcPath);
//    srcFile.open(QFile::ReadOnly);
    m_pFile = new QFile(m_srcPath, this);
    m_pFile->open(QFile::ReadOnly);
    m_p7ZExtract->setArchive(m_pFile);
    m_p7ZExtract->setTargetDirectory(m_desPath);

    QThreadPool::globalInstance()->start(m_p7ZExtract);
}

void ExtractArchiveManager::extractArchiveZip()
{
    JlCompress::extractDir(m_srcPath, m_desPath);
}

void ExtractArchiveManager::extractArchiveRar()
{
    QDir dir(m_desPath);
    if(!dir.exists()) {
        dir.mkpath(m_desPath);
    }

    QFileInfo exeInfo("UnRaR.exe");
    if(!exeInfo.exists()) {
        qDebug() << "Can't find UnRaR.exe, please add UnRaR.exe to current application dir path";
        return;
    }

    QProcess pro;
    QString t1 ="UnRaR.exe";
    QStringList t2;
    t2.append("x");
    t2.append("-ibck");
    t2.append("-y");
    t2.append("-o+");
    t2.append(m_srcPath);
    t2.append(m_desPath);

    int ret = pro.execute(t1,t2);
    qDebug() << "process return ret: " << ret;
}

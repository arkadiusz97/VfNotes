#include "QFilesContainer.h"
QStringList QFilesContainer::getFilesList()
{
    if(!isOpenDirectory) throw "Directory is not open.";
    dir.setPath(patch);
    filesList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    return filesList;
}

QString QFilesContainer::openFile(QString fileName)
{
    lastFile = patch+"/"+fileName;
    currnetFile.setFileName(lastFile);
    currnetFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = currnetFile.readAll();
    currnetFile.close();
    return content.toUtf8();
}

void QFilesContainer::closeFile()
{
    currnetFile.close();
}

bool QFilesContainer::saveCurrentFile(QString content)
{
    QTextStream stream(&currnetFile);
    stream.setCodec("UTF-8");
    currnetFile.setFileName(lastFile);
    currnetFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    stream<<content.toUtf8();
    currnetFile.close();
    return true;
}

QStringList QFilesContainer::newFile(QString fileName)
{
    QFile file(patch+"/"+fileName);
    file.open(QIODevice::WriteOnly | QIODevice::ReadOnly | QIODevice::Text);
    file.close();
    return getFilesList();
}

QStringList QFilesContainer::removeFile()
{
    currnetFile.remove();
    return getFilesList();
}

QStringList QFilesContainer::renameFile(QString newName)
{
    lastFile = patch+"/"+newName;
    currnetFile.rename(lastFile);
    return getFilesList();
}

QStringList QFilesContainer::openDirectory(QString location)
{
    patch = location;
    dir.mkpath(location);
    isOpenDirectory = true;
    currnetFile.close();
    return getFilesList();
}

bool QFilesContainer::checkOpenDirectory()
{
    return isOpenDirectory;
}

bool QFilesContainer::checkOpenFile()
{
    return currnetFile.isOpen();
}

QStringList QFilesContainer::closeDirectory()
{
    isOpenDirectory = false;
    currnetFile.close();
    filesList.clear();
    return filesList;
}

QFilesContainer::QFilesContainer():isOpenDirectory(false){}

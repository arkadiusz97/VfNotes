#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), pushButtonRenameEnabledFirstTime(false),
    isModified(false),
    change(false),
    startThemeFileName("start_theme.css"),
    versionString("1.1")
{
    ui->setupUi(this);
    ui->listWidgetNotes->addItems(notes.openDirectory(QDir::currentPath()+"/notes/"));
    setWindowTitle("VfNotes " + versionString);
    save = new QShortcut(QKeySequence(QKeySequence::Save), this);
    connect(save, SIGNAL(activated()), this, SLOT(on_pushButtonSave_clicked()));
    remove = new QShortcut(QKeySequence(QKeySequence::Delete), this);
    connect(remove, SIGNAL(activated()), this, SLOT(on_pushButtonRemove_clicked()));
    connect(ui->actionsettings, SIGNAL(triggered(bool)), this, SLOT(showSettingsWindow()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAboutWindow()));
    connect(ui->actionSetTheme, SIGNAL(triggered(bool)), this, SLOT(setTheme()));
    connect(ui->actionSetDefaultTheme, SIGNAL(triggered(bool)), this, SLOT(setDefaultTheme()));
    connect(&s, &settingsWindow::notesFontSize, this, &MainWindow::setNotesFontSize);
    connect(&s, &settingsWindow::noteFontSize, this, &MainWindow::setNoteFontSize);
    connect(&s, &settingsWindow::noteFontSize, this, &MainWindow::saveConfig);
    int fontId = QFontDatabase::addApplicationFont(":/OpenSans-Light.ttf");
    if(fontId != -1)
    {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont font(family, -1, QFont::Light, true);
        noteFont = font;
        notesFont = font;
        QApplication::setFont(font);
    }
    loadConfig();
    QFile startThemeFile(startThemeFileName);
    startThemeFile.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(startThemeFile.readAll());
    startThemeFile.close();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete save;
    delete remove;
}

void MainWindow::on_pushButtonSave_clicked()
{
    if(notes.checkOpenFile())
    {
        QMessageBox::information(this, "VfNotes", "File is not open!");
        return;
    }
    notes.saveCurrentFile(ui->plainTextEditContent->toPlainText().toUtf8());
    isModified = false;
    ui->pushButtonSave->setEnabled(false);
    ui->plainTextEditContent->setFocus();
}

void MainWindow::on_pushButtonNew_clicked()
{
    if(ui->lineEditNew->text().isEmpty())
    {
        QMessageBox::information(this, "VfNotes", "Name is empty!");
        return;
    }
    change = false;
    if(isModified)
    {
        auto reply = QMessageBox::question(this, "VfNotes", "Do you want save changes?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
        {
            on_pushButtonSave_clicked();
            notes.closeFile();
            ui->plainTextEditContent->clear();
        }
        else if(reply == QMessageBox::No) notes.closeFile();
        else return;
    }
    isModified = false;
    const QString newFileName = ui->lineEditNew->text();
    ui->listWidgetNotes->clear();
    ui->listWidgetNotes->addItems(notes.newFile(newFileName));
    ui->lineEditNew->clear();
    auto matchItems = ui->listWidgetNotes->findItems(newFileName, Qt::MatchExactly | Qt::MatchFixedString);
    ui->listWidgetNotes->setCurrentItem(matchItems[0]);
    if(ui->listWidgetNotes->count() == 1)
        ui->listWidgetNotes->setCurrentRow(0);
    ui->plainTextEditContent->setFocus();
}

void MainWindow::on_pushButtonRemove_clicked()
{
    ui->listWidgetNotes->setEnabled(false);
    if(notes.checkOpenFile())
    {
        QMessageBox::information(this, "VfNotes", "File is not open!");
        return;
    }
    auto reply = QMessageBox::question(this, "VfNotes", "Do you want remove this note?", QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        ui->listWidgetNotes->clear();
        ui->listWidgetNotes->addItems(notes.removeFile());
        this->setWindowTitle("VfNotes " + versionString);
        ui->plainTextEditContent->clear();
        ui->plainTextEditContent->setEnabled(false);
        ui->pushButtonRemove->setEnabled(false);
        ui->pushButtonRename->setEnabled(false);
        ui->pushButtonSave->setEnabled(false);
        isModified = change = false;
    }
    ui->listWidgetNotes->setEnabled(true);
}

void MainWindow::on_plainTextEditContent_textChanged()
{
    ui->pushButtonSave->setEnabled(true);
    if(!notes.checkOpenFile())
    {
        if(!change) change = true;
        else isModified = true;
    }
}

void MainWindow::on_pushButtonRename_clicked()
{
    if(ui->lineEditNew->text().isEmpty())
    {
        QMessageBox::information(this, "VfNotes", "Name is empty!");
        ui->plainTextEditContent->setFocus();
        return;
    }
    QStringList filesList = notes.getFilesList();
    if(filesList.indexOf(ui->lineEditNew->text()) != -1)
    {
        QMessageBox::information(this, "VfNotes", "Note with this name already exists.");
        return;
    }
    notes.renameFile(ui->lineEditNew->text());
    this->setWindowTitle(ui->lineEditNew->text()+" - VfNotes " + versionString);
    ui->lineEditNew->clear();
    ui->listWidgetNotes->clear();
    ui->listWidgetNotes->addItems(filesList);
    ui->plainTextEditContent->setFocus();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    handleSizeChangeInSplitterNotesAndNote();
    if(isModified)
    {
        QMessageBox::StandardButton exitButton = QMessageBox::question(this, "VfNotes", tr("Do you want save changes?"), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
        switch(exitButton)
        {
            case QMessageBox::Yes:
                on_pushButtonSave_clicked();
                QCoreApplication::quit();
                s.close();
                break;
            case QMessageBox::No:
                QCoreApplication::quit();
                s.close();
                break;
            case QMessageBox::Cancel:
                event->ignore();
                break;
        }
    }
    else
    {
        s.close();
    }
}

void MainWindow::showSettingsWindow()
{
    s.fontsInWindow(ui->plainTextEditContent->fontInfo().pointSize(), ui->listWidgetNotes->fontInfo().pointSize());
    s.show();
}

void MainWindow::setNotesFontSize(int fSize)
{
    jsonSettings["notesFontSize"] = QJsonValue(fSize);
    notesFont.setPointSize(fSize);
    ui->listWidgetNotes->setFont(notesFont);
}

void MainWindow::setNoteFontSize(int fSize)
{
    jsonSettings["noteFontSize"] = QJsonValue(fSize);
    noteFont.setPointSize(fSize);
    ui->plainTextEditContent->setFont(noteFont);
}

void MainWindow::showAboutWindow()
{
    QMessageBox::information(this, "About VfNotes.", "VfNotes " + versionString + ". Written by Arkadiusz97.");
}

void MainWindow::loadConfig()
{
    QFile settingsFile("VfNotes_settings.txt");
    settingsFile.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray settingsFileContent = settingsFile.readAll();
    jsonSettings = QJsonDocument::fromJson(settingsFileContent).object();
    if(jsonSettings.contains("notesFontSize"))
        setNotesFontSize(jsonSettings["notesFontSize"].toInt());
    if(jsonSettings.contains("noteFontSize"))
        setNoteFontSize(jsonSettings["noteFontSize"].toInt());
    if(jsonSettings.contains("mainWindowWidth") && jsonSettings.contains("mainWindowHeight"))
        resize(jsonSettings["mainWindowWidth"].toInt(), jsonSettings["mainWindowHeight"].toInt());

    if(jsonSettings.contains("sizesInSplitterNotesAndNote"))
    {
        QList<int>sizesToSet;
        for(QJsonValueRef i : jsonSettings["sizesInSplitterNotesAndNote"].toArray())
            sizesToSet.push_back(i.toInt());
        ui->splitterNotesAndNote->setSizes(sizesToSet);
    }
    settingsFile.close();
}

void MainWindow::saveConfig()
{
    QFile settingsFile("VfNotes_settings.txt");
    settingsFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QJsonDocument toSave(jsonSettings);
    settingsFile.write(toSave.toJson());
    settingsFile.close();
}

void MainWindow::on_listWidgetNotes_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current != NULL && pushButtonRenameEnabledFirstTime)
    {
        ui->plainTextEditContent->setEnabled(true);
        ui->pushButtonRemove->setEnabled(true);
        ui->pushButtonRename->setEnabled(true);
        ui->pushButtonSave->setEnabled(true);
        change = false;
        if(isModified)
        {
            auto reply = QMessageBox::question(this, "VfNotes", "Do you want save changes?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
            if(reply == QMessageBox::Yes) on_pushButtonSave_clicked();
            else if(reply == QMessageBox::No) notes.closeFile();
            else
            {
                QTimer::singleShot(0, qApp, [this, previous]
                {
                    ui->listWidgetNotes->blockSignals(true);
                    ui->listWidgetNotes->setCurrentItem(previous);
                    ui->listWidgetNotes->blockSignals(false);
                });
                return;
            }
        }
        isModified = false;
        this->setWindowTitle(current->text()+" - VfNotes " + versionString);
        ui->plainTextEditContent->setPlainText(notes.openFile(current->text()));
        ui->plainTextEditContent->setFocus();
        QTextCursor cursor(ui->plainTextEditContent->textCursor());
        cursor.movePosition(QTextCursor::End);
        ui->plainTextEditContent->setTextCursor(cursor);
    }
    if(!pushButtonRenameEnabledFirstTime)
    {
        pushButtonRenameEnabledFirstTime = true;
        ui->listWidgetNotes->setCurrentIndex(QModelIndex());
    }
    ui->pushButtonSave->setEnabled(false);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   jsonSettings["mainWindowWidth"] = QJsonValue(width());
   jsonSettings["mainWindowHeight"] = QJsonValue(height());
   saveConfig();
}

void MainWindow::handleSizeChangeInSplitterNotesAndNote()
{
    QList<int>sizesArray = ui->splitterNotesAndNote->sizes();
    QJsonArray sizesArrayToSave;
    for(int i : sizesArray)
        sizesArrayToSave.push_back(i);
    jsonSettings["sizesInSplitterNotesAndNote"] = QJsonValue(sizesArrayToSave);
    saveConfig();
}

void MainWindow::setTheme()
{
    QString themeFileName = QFileDialog::getOpenFileName(this, tr("Open theme"), "", tr("Css Files (*.css)"));
    if(themeFileName.isEmpty())
        return;
    QFile themeFile(themeFileName), startThemeFile(startThemeFileName);
    themeFile.open(QIODevice::ReadOnly);
    startThemeFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QByteArray themeFileContent = themeFile.readAll();
    startThemeFile.write(themeFileContent);
    qApp->setStyleSheet(themeFileContent);
    themeFile.close();
    startThemeFile.close();
}

void MainWindow::setDefaultTheme()
{
    int currentNotesFontSize = jsonSettings["notesFontSize"].toInt();
    int currentNoteFontSize = jsonSettings["noteFontSize"].toInt();
    qApp->setStyleSheet("");
    QFile startThemeFile(startThemeFileName);
    startThemeFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    startThemeFile.close();
    setNotesFontSize(currentNotesFontSize);
    setNoteFontSize(currentNoteFontSize);
}

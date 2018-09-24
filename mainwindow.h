#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QListWidgetItem>
#include "QFilesContainer.h"
#include "settingswindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void setNotesFontSize(int fSize);

    void setNoteFontSize(int fSize);
private slots:

    void on_pushButtonSave_clicked();

    void on_pushButtonNew_clicked();

    void on_pushButtonRemove_clicked();

    void on_plainTextEditContent_textChanged();

    void on_pushButtonRename_clicked();

    void closeEvent(QCloseEvent *event);

    void showSettingsWindow();

    void showAboutWindow();

    void loadConfig();

    void saveConfig();

    void on_listWidgetNotes_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MainWindow *ui;
    QFilesContainer notes;
    bool isModified, change, pushButtonRenameEnabledFirstTiem;
    QShortcut *save;
    QShortcut *remove;
    settingsWindow s;
    int iNotesFontSize, iNoteFontSize;
};

#endif // MAINWINDOW_H

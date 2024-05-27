#pragma once

#ifndef QTEDITOR_H
#define QTEDITOR_H

#include <QtWidgets/QMainWindow>
#include <QTextEdit>
#include <QLabel> 

class QT_editor : public QMainWindow
{
    Q_OBJECT

public:
    QT_editor(QWidget *parent = nullptr);
    ~QT_editor() {}

public slots:
    void setCurrentFile(const QString &file);

private slots:

    void newFile();
    void newWindow();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exit();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void Delete();

private:

    void Actions();
    void CreateMenu();
    void CreateToolbar();
    void CreateStatusBar();
    void updateStatusBar();

    QTextEdit* textEdit;
    QMenu* menuFile;
    QMenu* menuEdit;
    QMenu* menuHelp;

    QAction* actionNew;
    QAction* actionNewWindow;
    QAction* actionOpen;
    QAction* actionSave;
    QAction* actionSaveAs;
    QAction* actionExit;

    QAction* actionUndo;
    QAction* actionRedo;
    QAction* actionCut;
    QAction* actionCopy;
    QAction* actionPaste;
    QAction* actionDelete;

    QAction* actionAbout;

    QToolBar* fileToolBar;
    QToolBar* editToolBar;

    QString currentFile;
    QString selectedEncoding;

    QLabel* statusBarLabel; 
    QLabel* encodingLabel;
    QLabel* positionLabel;
    QLabel* selectedLengthLabel;

};

#endif QTEDITOR_H
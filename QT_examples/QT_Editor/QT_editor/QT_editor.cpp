
#include "QT_editor.h"
#include <QShortcut>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QGuiApplication>
#include <QSaveFile>
#include <QMenuBar>
#include <QToolBar>
#include <QLabel> 
#include <QStatusBar>
#include <QTextCodec> // QT 5 compatibility module needed
#include <QInputDialog>

QT_editor::QT_editor(QWidget *parent)
    : QMainWindow(parent)
{
	textEdit = new QTextEdit(this);
	setCentralWidget(textEdit);
	statusBarLabel = new QLabel("Ready", this);
	statusBar()->addWidget(statusBarLabel);

	Actions();
	CreateMenu();
	CreateToolbar();
	CreateStatusBar();

	setWindowTitle("Text Editor");
	setMinimumSize(800, 600);

	QIcon appIcon(":/icons/editor.png");
	setWindowIcon(appIcon);
}

void QT_editor::Actions()
{

	// File
	actionNew = new QAction(QIcon(":/icons/new.png"), "New", this);
	actionNew->setShortcut(QKeySequence::New);
	connect(actionNew, &QAction::triggered, this, &QT_editor::newFile);

	actionNewWindow = new QAction("New Window");
	actionNewWindow->setShortcut(tr("Ctrl+n"));
	connect(actionNewWindow, &QAction::triggered, this, &QT_editor::newWindow);

	actionOpen = new QAction(QIcon(":/icons/open.png"), "Open", this);
	actionOpen->setShortcut(QKeySequence::Open);
	connect(actionOpen, &QAction::triggered, this, &QT_editor::openFile);

	actionSave = new QAction(QIcon(":/icons/save.png"), "Save", this);
	actionSave->setShortcut(QKeySequence::Save);
	connect(actionSave, &QAction::triggered, this, &QT_editor::saveFile);

	actionSaveAs= new QAction(QIcon(":/icons/save_as.png"), "Save As", this);
	actionSaveAs->setShortcut(tr("Ctrl+Shift+S"));
	connect(actionSaveAs, &QAction::triggered, this, &QT_editor::saveFileAs);

	actionExit = new QAction(QIcon(":/icons/exit.png"), "Exit", this);
	actionExit->setShortcut(tr("Ctrl+Q"));
	connect(actionExit, &QAction::triggered, this, &QT_editor::exit);

	// Edit
	actionUndo = new QAction("Undo", this);
	actionUndo->setShortcut(QKeySequence::Undo);
	connect(actionUndo, &QAction::triggered, this, &QT_editor::undo);

	actionRedo = new QAction("Redo", this);
	actionRedo->setShortcut(QKeySequence::Redo);
	connect(actionRedo, &QAction::triggered, this, &QT_editor::redo);

	actionCut = new QAction(QIcon(":/icons/cut.png"), "Cut", this);
	actionCut->setShortcut(QKeySequence::Cut);
	connect(actionCut, &QAction::triggered, this, &QT_editor::cut);

	actionCopy = new QAction(QIcon(":/icons/copy.png"), "Copy", this);
	actionCopy->setShortcut(QKeySequence::Copy);
	connect(actionCopy, &QAction::triggered, this, &QT_editor::copy);

	actionPaste = new QAction(QIcon(":/icons/paste.png"), "Paste", this);
	actionPaste->setShortcut(QKeySequence::Paste);
	connect(actionPaste, &QAction::triggered, this, &QT_editor::paste);

	actionDelete = new QAction("Delete");
	actionDelete->setShortcut(QKeySequence::Delete);
	connect(actionDelete, &QAction::triggered, this, &QT_editor::Delete);

	// Help
	actionAbout = new QAction(QIcon(":/icons/about.png"), "help");
}

void QT_editor::CreateMenu()
{

	menuFile = menuBar()->addMenu("File");
	{
		
		actionNew->setStatusTip("Create new File");
		menuFile->addAction(actionNew);

		actionNewWindow->setStatusTip("Open new Window");
		menuFile->addAction(actionNewWindow);

		actionOpen->setStatusTip("open exist File");
		menuFile->addAction(actionOpen);

		
		actionSave->setStatusTip("save File");
		menuFile->addAction(actionSave);

		
		actionSaveAs->setStatusTip("save file as new encoding");
		menuFile->addAction(actionSaveAs);

		menuFile->addSeparator();
		
		actionExit->setStatusTip("exit program");
		menuFile->addAction(actionExit);
	}

	menuEdit = menuBar()->addMenu("Edit");
	{
		
		actionUndo->setStatusTip("Undo");
		menuEdit->addAction(actionUndo);

		actionRedo->setStatusTip("Redo");
		menuEdit->addAction(actionRedo);

		actionCut->setStatusTip("Cut");
		menuEdit->addAction(actionCut);

		actionCopy->setStatusTip("Copy");
		menuEdit->addAction(actionCopy);

		actionPaste->setStatusTip("Paste");
		menuEdit->addAction(actionPaste);

		actionDelete->setStatusTip("Delete");
		menuEdit->addAction(actionDelete);
	}

	menuHelp = menuBar()->addMenu("Help");
	{
		
		actionAbout->setStatusTip("about us");
		menuHelp->addAction(actionAbout);
	}

}

void QT_editor::CreateToolbar()
{
	fileToolBar = addToolBar("File");
	fileToolBar->addAction(actionNew);
	fileToolBar->addAction(actionOpen);
	fileToolBar->addAction(actionSave);

	editToolBar = addToolBar("Edit");
	editToolBar->addAction(actionCut);
	editToolBar->addAction(actionCopy);
	editToolBar->addAction(actionPaste);
}

void QT_editor::CreateStatusBar()
{
	statusBarLabel = new QLabel("Ready", this);
	statusBar()->addWidget(statusBarLabel);

	encodingLabel = new QLabel("Encoding: UTF-8", this);
	statusBar()->addWidget(encodingLabel);  // Display default encoding

	positionLabel = new QLabel("Line: 1, Col: 1", this);
	statusBar()->addWidget(positionLabel);

	selectedLengthLabel = new QLabel("Selected: 0", this);
	statusBar()->addWidget(selectedLengthLabel);
}

void QT_editor::updateStatusBar()
{
	// Example: Update position label
	QTextCursor cursor = textEdit->textCursor();
	int line = cursor.blockNumber() + 1; // Lines are 1-based
	int col = cursor.columnNumber() + 1; // Columns are 1-based

	positionLabel->setText(QString("Line: %1, Col: %2").arg(line).arg(col));

	// Update encoding label using the stored encoding
	encodingLabel->setText("Encoding: " + selectedEncoding);

}

void QT_editor::newFile()
{
	// Check if the current content is modified
	if (textEdit->document()->isModified()) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "Save Changes", "Do you want to save changes to the current file?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			saveFile(); // Save changes before creating a new file
		} else if (reply == QMessageBox::Cancel) {
			return; // Cancel the new file operation
		}
	}

	textEdit->clear();

	// Reset the current file information
	setCurrentFile(""); 

	// Optionally, you can perform additional setup for a new file
	statusBarLabel->setText("Ready");
	textEdit->document()->setModified(false);
}

void QT_editor::newWindow()
{
	// Create a new instance of the QT_editor class
	QT_editor *newEditor = new QT_editor;

	newEditor->show();
}

void QT_editor::setCurrentFile(const QString &file)
{
	currentFile = file;
	setWindowTitle("Text Editor - " + currentFile);
	statusBarLabel->setText("Current File: " + currentFile);
}

void QT_editor::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), "Text Files (*.txt);;All Files (*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Display an error message if the file cannot be opened
            QMessageBox::critical(this, "Error", "Could not open the file.");
            return;
        }

        // Read the content of the file
        QTextStream in(&file);
        QString fileContent = in.readAll();

        file.close();

        // Set the content to the text editor
        textEdit->setPlainText(fileContent);

        // Set currentFile to the opened file
        setCurrentFile(fileName);
    }
}

void QT_editor::saveFile()
{
	if (currentFile.isEmpty()) {
		// If the file hasn't been saved yet, prompt the user for a file name
		QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), "Text Files (*.txt);;All Files (*)");

		if (fileName.isEmpty()) {
			// User canceled the save operation
			return;
		}

		currentFile = fileName;
	}

	QSaveFile file(currentFile);

	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		out << textEdit->toPlainText();
		file.commit();
		statusBar()->showMessage("File saved", 2000); // Show a success message in the status bar
	} else {
		QMessageBox::critical(this, "Error", "Could not save the file.");
	}
}

void QT_editor::saveFileAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"), QDir::homePath(), tr("Text Files (*.txt);;All Files (*)"));

	if (fileName.isNull()) {
		// User canceled the save operation
		return;
	}

	// Convert available codecs to QStringList
	QStringList availableCodecs;
	foreach (const QByteArray &codec, QTextCodec::availableCodecs()) {
		availableCodecs << QString::fromUtf8(codec);
	}

	// Display a dialog for selecting the encoding
	bool ok;
	QString selectedEncoding = QInputDialog::getItem(this, tr("Select Encoding"), tr("Choose Encoding:"), availableCodecs, 0, false, &ok);

	if (!ok) {
		// User canceled the encoding selection
		return;
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("Error"), tr("Could not save the file."));
		return;
	}

	QTextStream out(&file);
	out << textEdit->toPlainText();

	file.close();
}

void QT_editor::exit()
{
	// Check if the current content is modified
	if (textEdit->document()->isModified()) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "Save Changes", "Do you want to save changes to the current file?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (reply == QMessageBox::Yes) {
			saveFile(); // Save changes before exiting
		} else if (reply == QMessageBox::Cancel) {
			return; // Cancel the exit operation
		}
	}

	QCoreApplication::quit();
}

void QT_editor::undo()
{
	textEdit->undo();
}

void QT_editor::redo()
{
	textEdit->redo();
}

void QT_editor::cut()
{
	textEdit->cut();
}

void QT_editor::copy()
{
	textEdit->copy();
}

void QT_editor::paste()
{
	textEdit->paste();
}

void QT_editor::Delete() 
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.removeSelectedText();
}
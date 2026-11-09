#include "MainWindow.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollBar>
#include <QSize>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QVariant>

#include <exception>
#include <vector>

#include "Utilities.h"

namespace
{
    const char *const kAdditionExample =
        "; Add 3 + 7 + 9 and display the result (19 = 13h) on the screen.\n"
        "2003\t; R0 = 3\n"
        "2107\t; R1 = 7\n"
        "2209\t; R2 = 9\n"
        "5301\t; R3 = R0 + R1\n"
        "5423\t; R4 = R2 + R3\n"
        "34FF\t; Store R4 in FF (screen)\n"
        "C000\t; Halt\n";

    const char *const kComparisonExample =
        "; Compare two numbers and display 1 if they are equal, 0 otherwise.\n"
        "2005\t; R0 = 5\n"
        "2105\t; R1 = 5\n"
        "B10C\t; If R1 == R0, jump to 0C\n"
        "2000\t; R0 = 0 (not equal)\n"
        "30FF\t; Display 0\n"
        "C000\t; Halt\n"
        "2001\t; R0 = 1 (equal)\n"
        "30FF\t; Display 1\n"
        "C000\t; Halt\n";

    QFont monospaceFont()
    {
        QFont font(QStringLiteral("Consolas"), 10);
        font.setStyleHint(QFont::Monospace);
        font.setFixedPitch(true);
        return font;
    }

    QLabel *makeCaption(const QString &text, QWidget *parent)
    {
        auto *label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("stateCaption"));
        return label;
    }

    QLabel *makeValueLabel(const QString &text, QWidget *parent)
    {
        auto *label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("stateValue"));
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return label;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      machine(),
      runTimer(new QTimer(this))
{
    setWindowTitle(QStringLiteral("Vole Machine Simulator"));
    setMinimumSize(980, 560);

    runTimer->setTimerType(Qt::PreciseTimer);

    buildUi();
    buildActions();
    applyTheme();
    connectSignals();

    // The combo starts on Hex, which is what most course programs expect.
    machine.setScreenFormat(ScreenFormat::Hex);

    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("Load a program or type instructions, then press \"Load to memory\"."));
}

// ---------------------------------------------------------------------------
// User interface construction
// ---------------------------------------------------------------------------

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(10, 8, 10, 8);
    rootLayout->setSpacing(8);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(8);

    // ----- Left column: program editor and screen output --------------------
    auto *left = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    auto *programGroup = new QGroupBox(QStringLiteral("Program"), left);
    auto *programLayout = new QVBoxLayout(programGroup);
    programLayout->setContentsMargins(10, 14, 10, 10);
    programLayout->setSpacing(6);

    editor = new QPlainTextEdit(programGroup);
    editor->setObjectName(QStringLiteral("programEditor"));
    editor->setFont(monospaceFont());
    editor->setMinimumHeight(80);
    editor->setTabStopDistance(32);
    editor->setPlaceholderText(
        QStringLiteral("; Enter one 4-digit hexadecimal instruction per line.\n"
                       "; Everything after ';' is a comment.\n"
                       "2003   ; R0 = 3\n"
                       "2107   ; R1 = 7\n"
                       "5301   ; R3 = R0 + R1\n"
                       "34FF   ; Display R3\n"
                       "C000   ; Halt"));
    programLayout->addWidget(editor, 1);

    auto *addressRow = new QHBoxLayout();
    addressRow->setSpacing(8);

    auto *addressLabel = new QLabel(QStringLiteral("Start address"), programGroup);
    startAddressEdit = new QLineEdit(QStringLiteral("00"), programGroup);
    startAddressEdit->setAlignment(Qt::AlignCenter);
    startAddressEdit->setFixedWidth(64);
    startAddressEdit->setMaxLength(2);
    startAddressEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9A-Fa-f]{0,2}")), startAddressEdit));
    startAddressEdit->setToolTip(
        QStringLiteral("Even hexadecimal address from 00 to FE (00, 02, 04, ... FE)."));

    loadButton = new QPushButton(QStringLiteral("Load to memory"), programGroup);
    loadButton->setCursor(Qt::PointingHandCursor);

    addressRow->addWidget(addressLabel);
    addressRow->addWidget(startAddressEdit);
    addressRow->addStretch(1);
    addressRow->addWidget(loadButton);
    programLayout->addLayout(addressRow);

    editorHint = new QLabel(
        QStringLiteral("Instructions are stored high byte first, two memory cells per instruction."),
        programGroup);
    editorHint->setObjectName(QStringLiteral("hintLabel"));
    editorHint->setWordWrap(true);
    programLayout->addWidget(editorHint);

    leftLayout->addWidget(programGroup, 1);

    auto *screenGroup = new QGroupBox(QStringLiteral("Screen output"), left);
    auto *screenLayout = new QVBoxLayout(screenGroup);
    screenLayout->setContentsMargins(10, 14, 10, 10);
    screenLayout->setSpacing(6);

    auto *screenHeader = new QHBoxLayout();
    screenHeader->setSpacing(8);

    auto *screenFormatLabel = new QLabel(QStringLiteral("Format"), screenGroup);
    screenFormatCombo = new QComboBox(screenGroup);
    screenFormatCombo->setObjectName(QStringLiteral("screenFormatCombo"));
    screenFormatCombo->addItems({ QStringLiteral("Hex"),
                                  QStringLiteral("Decimal"),
                                  QStringLiteral("ASCII") });
    screenFormatCombo->setCurrentIndex(0);
    screenFormatCombo->setMinimumWidth(96);
    screenFormatCombo->setToolTip(
        QStringLiteral("How values written to FF are shown.\n"
                       "Hex: 0F, FC, 2A\n"
                       "Decimal: 15, 252, 42\n"
                       "ASCII: H, i, 6"));

    screenHeader->addWidget(screenFormatLabel);
    screenHeader->addWidget(screenFormatCombo);
    screenHeader->addStretch(1);
    screenLayout->addLayout(screenHeader);

    screenView = new QPlainTextEdit(screenGroup);
    screenView->setObjectName(QStringLiteral("screenView"));
    screenView->setFont(monospaceFont());
    screenView->setReadOnly(true);
    screenView->setPlaceholderText(QStringLiteral("Output written to address FF appears here."));
    screenView->setMinimumHeight(70);
    screenLayout->addWidget(screenView);

    leftLayout->addWidget(screenGroup, 0);

    // ----- Right column: registers, processor state, memory -----------------
    auto *right = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    auto *registerGroup = new QGroupBox(QStringLiteral("Registers"), right);
    auto *registerLayout = new QVBoxLayout(registerGroup);
    registerLayout->setContentsMargins(10, 14, 10, 10);

    // Two blocks side by side: R0-R7 on the left and R8-RF on the right.
    registerTable = new QTableWidget(8, 4, registerGroup);
    registerTable->setObjectName(QStringLiteral("registerTable"));
    registerTable->setFont(monospaceFont());
    registerTable->setHorizontalHeaderLabels(
        { QStringLiteral("Reg"), QStringLiteral("Hex"),
          QStringLiteral("Reg"), QStringLiteral("Hex") });
    registerTable->verticalHeader()->setVisible(false);
    registerTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    registerTable->verticalHeader()->setDefaultSectionSize(19);
    registerTable->verticalHeader()->setMinimumSectionSize(14);
    registerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    registerTable->horizontalHeader()->setHighlightSections(false);
    registerTable->horizontalHeader()->setMinimumSectionSize(32);
    registerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    registerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    registerTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    registerTable->setShowGrid(true);
    registerTable->setAlternatingRowColors(false);

    for (int row = 0; row < 8; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            auto *item = new QTableWidgetItem(QString());
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            registerTable->setItem(row, column, item);
        }
    }

    registerLayout->addWidget(registerTable);
    topRow->addWidget(registerGroup, 1);

    auto *processorGroup = new QGroupBox(QStringLiteral("Processor"), right);
    auto *processorLayout = new QVBoxLayout(processorGroup);
    processorLayout->setContentsMargins(10, 14, 10, 10);
    processorLayout->setSpacing(8);

    auto *stateGrid = new QGridLayout();
    stateGrid->setHorizontalSpacing(14);
    stateGrid->setVerticalSpacing(8);

    auto *pcCaption = makeCaption(QStringLiteral("PC"), processorGroup);
    pcCaption->setToolTip(QStringLiteral("Program counter"));
    auto *irCaption = makeCaption(QStringLiteral("IR"), processorGroup);
    irCaption->setToolTip(QStringLiteral("Instruction register"));

    stateGrid->addWidget(pcCaption, 0, 0);
    stateGrid->addWidget(irCaption, 0, 1);

    pcValue = makeValueLabel(QStringLiteral("0000"), processorGroup);
    irValue = makeValueLabel(QStringLiteral("0000"), processorGroup);
    stateGrid->addWidget(pcValue, 1, 0);
    stateGrid->addWidget(irValue, 1, 1);

    stateGrid->addWidget(makeCaption(QStringLiteral("STATE"), processorGroup), 2, 0);
    stateGrid->addWidget(makeCaption(QStringLiteral("INSTRUCTION"), processorGroup), 2, 1);

    stateValue = makeValueLabel(QStringLiteral("IDLE"), processorGroup);
    instructionValue = new QLabel(QStringLiteral("—"), processorGroup);
    instructionValue->setObjectName(QStringLiteral("instructionValue"));
    instructionValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    instructionValue->setWordWrap(true);
    instructionValue->setMinimumWidth(0);
    instructionValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    stateGrid->addWidget(stateValue, 3, 0);
    stateGrid->addWidget(instructionValue, 3, 1);

    stateGrid->setColumnStretch(0, 0);
    stateGrid->setColumnStretch(1, 1);
    processorLayout->addLayout(stateGrid);

    processorLayout->addStretch(1);

    topRow->addWidget(processorGroup, 1);
    rightLayout->addLayout(topRow, 0);

    auto *memoryGroup = new QGroupBox(QStringLiteral("Memory (256 bytes)"), right);
    auto *memoryLayout = new QVBoxLayout(memoryGroup);
    memoryLayout->setContentsMargins(10, 14, 10, 10);

    memoryTable = new QTableWidget(16, 16, memoryGroup);
    memoryTable->setObjectName(QStringLiteral("memoryTable"));
    memoryTable->setFont(monospaceFont());
    memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    memoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    memoryTable->setAlternatingRowColors(false);
    memoryTable->setShowGrid(true);
    memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->horizontalHeader()->setHighlightSections(false);
    memoryTable->verticalHeader()->setHighlightSections(false);
    memoryTable->horizontalHeader()->setMinimumSectionSize(22);
    memoryTable->verticalHeader()->setMinimumSectionSize(14);
    memoryTable->verticalHeader()->setDefaultSectionSize(18);
    memoryTable->setMinimumHeight(100);
    memoryTable->setToolTip(
        QStringLiteral("Rows are the high nibble and columns the low nibble of the address.\n"
                       "Double-click a cell to edit that byte."));

    QStringList columnLabels;
    QStringList rowLabels;
    for (int index = 0; index < 16; ++index)
    {
        columnLabels << hexDigit(index);
        rowLabels << hexDigit(index) + QStringLiteral("0");
    }
    memoryTable->setHorizontalHeaderLabels(columnLabels);
    memoryTable->setVerticalHeaderLabels(rowLabels);

    for (int row = 0; row < 16; ++row)
    {
        for (int column = 0; column < 16; ++column)
        {
            auto *item = new QTableWidgetItem(QStringLiteral("00"));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            memoryTable->setItem(row, column, item);
        }
    }

    memoryLayout->addWidget(memoryTable);
    rightLayout->addWidget(memoryGroup, 1);

    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    splitter->setSizes({ 400, 760 });

    rootLayout->addWidget(splitter, 1);
    setCentralWidget(central);
}

void MainWindow::buildActions()
{
    newAction = new QAction(style()->standardIcon(QStyle::SP_FileIcon),
                            QStringLiteral("&New"), this);
    newAction->setShortcut(QKeySequence::New);

    openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton),
                             QStringLiteral("&Open..."), this);
    openAction->setShortcut(QKeySequence::Open);

    saveAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton),
                             QStringLiteral("&Save..."), this);
    saveAction->setShortcut(QKeySequence::Save);

    loadAction = new QAction(style()->standardIcon(QStyle::SP_ArrowDown),
                             QStringLiteral("Load to memory"), this);
    loadAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));

    runAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay),
                            QStringLiteral("&Run"), this);
    runAction->setShortcut(QKeySequence(QStringLiteral("F5")));
    runAction->setToolTip(QStringLiteral("Run the program (F5). Press again to pause."));

    stepAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward),
                             QStringLiteral("&Step"), this);
    stepAction->setShortcut(QKeySequence(QStringLiteral("F10")));
    stepAction->setToolTip(QStringLiteral("Execute one instruction (F10)."));

    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop),
                             QStringLiteral("S&top"), this);
    stopAction->setShortcut(QKeySequence(QStringLiteral("Shift+F5")));
    stopAction->setToolTip(QStringLiteral("Halt the machine (Shift+F5)."));

    resetAction = new QAction(style()->standardIcon(QStyle::SP_BrowserReload),
                              QStringLiteral("&Reset"), this);
    resetAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    resetAction->setToolTip(QStringLiteral("Clear memory, registers and the screen (Ctrl+R)."));

    auto *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    auto *examplesMenu = fileMenu->addMenu(QStringLiteral("Load &example"));
    auto *additionAction = examplesMenu->addAction(QStringLiteral("Addition (3 + 7 + 9)"));
    auto *comparisonAction = examplesMenu->addAction(QStringLiteral("Comparison (equal or not)"));

    connect(additionAction, &QAction::triggered, this, [this]() {
        editor->setPlainText(QString::fromLatin1(kAdditionExample));
        currentFile.clear();
        loadEditorProgram();
    });
    connect(comparisonAction, &QAction::triggered, this, [this]() {
        editor->setPlainText(QString::fromLatin1(kComparisonExample));
        currentFile.clear();
        loadEditorProgram();
    });

    fileMenu->addSeparator();
    auto *exitAction = fileMenu->addAction(QStringLiteral("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *machineMenu = menuBar()->addMenu(QStringLiteral("&Machine"));
    machineMenu->addAction(loadAction);
    machineMenu->addSeparator();
    machineMenu->addAction(runAction);
    machineMenu->addAction(stepAction);
    machineMenu->addAction(stopAction);
    machineMenu->addSeparator();
    machineMenu->addAction(resetAction);

    auto *helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    auto *aboutAction = helpMenu->addAction(QStringLiteral("&About"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    auto *toolbar = addToolBar(QStringLiteral("Main"));
    toolbar->setObjectName(QStringLiteral("mainToolBar"));
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->setIconSize(QSize(18, 18));

    toolbar->addAction(newAction);
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addSeparator();
    toolbar->addAction(loadAction);
    toolbar->addSeparator();
    toolbar->addAction(runAction);
    toolbar->addAction(stepAction);
    toolbar->addAction(stopAction);
    toolbar->addAction(resetAction);
    toolbar->addSeparator();

    auto *toolbarSpeedLabel = new QLabel(QStringLiteral("Speed"), toolbar);
    toolbarSpeedLabel->setObjectName(QStringLiteral("toolbarLabel"));
    toolbar->addWidget(toolbarSpeedLabel);

    speedCombo = new QComboBox(toolbar);
    speedCombo->addItems({ QStringLiteral("Slow"),
                           QStringLiteral("Normal"),
                           QStringLiteral("Fast"),
                           QStringLiteral("Turbo") });
    speedCombo->setCurrentIndex(1);
    speedCombo->setMinimumWidth(100);
    toolbar->addWidget(speedCombo);
}

void MainWindow::applyTheme()
{
    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

void MainWindow::connectSignals()
{
    connect(newAction, &QAction::triggered, this, &MainWindow::newProgram);
    connect(openAction, &QAction::triggered, this, &MainWindow::openProgram);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveProgram);
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadEditorProgram);
    connect(runAction, &QAction::triggered, this, &MainWindow::toggleRun);
    connect(stepAction, &QAction::triggered, this, &MainWindow::stepProgram);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stopProgram);
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetMachine);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadEditorProgram);
    connect(runTimer, &QTimer::timeout, this, &MainWindow::onRunTimeout);
    connect(speedCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onSpeedChanged);
    connect(screenFormatCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::onScreenFormatChanged);
    connect(memoryTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::editMemoryCell);
}

// ---------------------------------------------------------------------------
// Program loading and execution
// ---------------------------------------------------------------------------

void MainWindow::newProgram()
{
    runTimer->stop();
    machine.reset();

    programLoaded = false;
    programStartAddress = 0;
    currentFile.clear();
    lastStepFailed = false;
    haveSnapshot = false;

    editor->clear();
    startAddressEdit->setText(QStringLiteral("00"));

    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("New program. Type instructions or open a file."));
}

void MainWindow::openProgram()
{
    const QString startDirectory =
        currentFile.isEmpty() ? QString() : QFileInfo(currentFile).absolutePath();

    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Open Vole program"),
        startDirectory,
        QStringLiteral("Vole programs (*.txt *.vole *.asm);;All files (*)"));

    if (!path.isEmpty())
    {
        openProgramFile(path);
    }
}

void MainWindow::openProgramFile(const QString &path, bool runAfterLoad)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showError(QStringLiteral("Cannot open %1:\n%2")
                      .arg(QDir::toNativeSeparators(path), file.errorString()));
        return;
    }

    const QString text = QString::fromUtf8(file.readAll());
    file.close();

    editor->setPlainText(text);
    currentFile = path;

    int startAddress = 0;
    QString error;
    if (!readStartAddress(startAddress, error))
    {
        showError(error);
        return;
    }

    try
    {
        loadProgramText(text, startAddress, QFileInfo(path).fileName());
    }
    catch (const std::exception &exception)
    {
        showError(QString::fromUtf8(exception.what()));
        return;
    }

    if (runAfterLoad)
    {
        toggleRun();
    }
}

bool MainWindow::setScreenFormat(const QString &formatName)
{
    const QString name = formatName.trimmed().toLower();

    int index = -1;
    if (name == QStringLiteral("hex"))
    {
        index = 0;
    }
    else if (name == QStringLiteral("dec") || name == QStringLiteral("decimal"))
    {
        index = 1;
    }
    else if (name == QStringLiteral("ascii") || name == QStringLiteral("text"))
    {
        index = 2;
    }

    if (index < 0)
    {
        return false;
    }

    // Changing the index emits currentIndexChanged, which updates the machine
    // and refreshes the screen view.
    screenFormatCombo->setCurrentIndex(index);
    return true;
}

void MainWindow::saveProgram()
{
    QString path = currentFile;
    if (path.isEmpty())
    {
        path = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("Save Vole program"),
            QStringLiteral("program.txt"),
            QStringLiteral("Vole programs (*.txt);;All files (*)"));
    }

    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        showError(QStringLiteral("Cannot save %1:\n%2")
                      .arg(QDir::toNativeSeparators(path), file.errorString()));
        return;
    }

    file.write(editor->toPlainText().toUtf8());
    file.close();

    currentFile = path;
    setStatus(QStringLiteral("Saved %1.").arg(QDir::toNativeSeparators(path)));
}

void MainWindow::loadEditorProgram()
{
    int startAddress = 0;
    QString error;
    if (!readStartAddress(startAddress, error))
    {
        showError(error);
        return;
    }

    const QString sourceName =
        currentFile.isEmpty() ? QStringLiteral("editor") : QFileInfo(currentFile).fileName();

    try
    {
        loadProgramText(editor->toPlainText(), startAddress, sourceName);
    }
    catch (const std::exception &exception)
    {
        showError(QString::fromUtf8(exception.what()));
    }
}

void MainWindow::loadProgramText(const QString &text, int startAddress, const QString &sourceName)
{
    ProgramLoader loader;
    const std::vector<uint8_t> programBytes = loader.parseProgramText(text.toStdString());
    loader.validateProgram(programBytes, startAddress);

    runTimer->stop();
    machine.reset();
    loader.loadProgram(machine.getMemory(), programBytes, startAddress);

    machine.getControlUnit().setPC(startAddress);
    machine.getControlUnit().setIR(0);

    programLoaded = true;
    programStartAddress = startAddress;
    lastStepFailed = false;
    haveSnapshot = false;

    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("Loaded %1 instruction(s) from %2 at address %3.")
                  .arg(static_cast<int>(programBytes.size() / 2))
                  .arg(sourceName, hexWord(startAddress)));
}

bool MainWindow::readStartAddress(int &address, QString &error) const
{
    QString text = startAddressEdit->text().trimmed();
    if (text.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
    {
        text = text.mid(2);
    }

    if (text.isEmpty())
    {
        address = 0;
        return true;
    }

    bool ok = false;
    const uint value = text.toUInt(&ok, 16);

    if (!ok || value > 254 || value % 2 != 0)
    {
        error = QStringLiteral("Start address must be an even hexadecimal value from 00 to FE.");
        return false;
    }

    address = static_cast<int>(value);
    return true;
}

void MainWindow::restartAtProgramStart()
{
    machine.getControlUnit().setPC(programStartAddress);
    machine.getControlUnit().setIR(0);
    machine.start();
    lastStepFailed = false;
}

bool MainWindow::stepMachine()
{
    try
    {
        machine.step();
        return true;
    }
    catch (const std::exception &exception)
    {
        lastStepFailed = true;
        runTimer->stop();
        refreshViews();
        showError(QString::fromUtf8(exception.what()));
        return false;
    }
}

void MainWindow::toggleRun()
{
    if (!programLoaded)
    {
        setStatus(QStringLiteral("No program loaded. Load one first."), true);
        return;
    }

    if (runTimer->isActive())
    {
        runTimer->stop();
        refreshViews(false);
        updateControlStates();
        setStatus(QStringLiteral("Paused at PC = %1.")
                      .arg(hexWord(machine.getControlUnit().getPC())));
        return;
    }

    if (!machine.isRunning())
    {
        const int pc = machine.getControlUnit().getPC();
        if (pc < 0 || pc >= 256 || lastStepFailed)
        {
            restartAtProgramStart();
        }
        else
        {
            machine.start();
        }
    }

    lastStepFailed = false;
    runTimer->setInterval(speedInterval());
    runTimer->start();

    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("Running..."));
}

void MainWindow::stepProgram()
{
    if (!programLoaded)
    {
        setStatus(QStringLiteral("No program loaded. Load one first."), true);
        return;
    }

    if (runTimer->isActive())
    {
        return;
    }

    if (!machine.isRunning())
    {
        const int pc = machine.getControlUnit().getPC();
        if (pc < 0 || pc >= 256 || lastStepFailed)
        {
            restartAtProgramStart();
        }
        else
        {
            machine.start();
        }
    }

    if (!stepMachine())
    {
        refreshViews();
        updateControlStates();
        return;
    }

    refreshViews();

    if (!machine.isRunning())
    {
        setStatus(QStringLiteral("Program halted."));
    }

    updateControlStates();
}

void MainWindow::stopProgram()
{
    runTimer->stop();

    if (programLoaded)
    {
        machine.halt();
    }

    lastStepFailed = false;
    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("Stopped. Press Run or Step to restart from the beginning."));
}

void MainWindow::resetMachine()
{
    runTimer->stop();
    machine.reset();

    programLoaded = false;
    programStartAddress = 0;
    lastStepFailed = false;
    haveSnapshot = false;

    refreshViews(false);
    updateControlStates();
    setStatus(QStringLiteral("Machine reset. Memory, registers and the screen are clear."));
}

void MainWindow::onRunTimeout()
{
    if (!machine.isRunning())
    {
        runTimer->stop();
        updateControlStates();
        return;
    }

    const int batch = stepsPerTick();
    for (int index = 0; index < batch; ++index)
    {
        if (!machine.isRunning())
        {
            break;
        }

        if (!stepMachine())
        {
            updateControlStates();
            return;
        }
    }

    if (!machine.isRunning())
    {
        runTimer->stop();
    }

    // Refresh after stopping the timer so the state label reports HALTED
    // rather than RUNNING.
    refreshViews();

    if (!machine.isRunning())
    {
        updateControlStates();

        if (!lastStepFailed)
        {
            setStatus(QStringLiteral("Program halted."));
        }
    }
}

void MainWindow::onSpeedChanged()
{
    if (runTimer->isActive())
    {
        runTimer->setInterval(speedInterval());
    }
}

void MainWindow::onScreenFormatChanged()
{
    ScreenFormat format = ScreenFormat::Hex;

    switch (screenFormatCombo->currentIndex())
    {
        case 1:
            format = ScreenFormat::Decimal;
            break;
        case 2:
            format = ScreenFormat::Ascii;
            break;
        default:
            format = ScreenFormat::Hex;
            break;
    }

    machine.setScreenFormat(format);

    // Re-render the existing output immediately; changing the format must not
    // require re-running the program.
    refreshViews(false);

    setStatus(QStringLiteral("Screen format: %1.").arg(screenFormatCombo->currentText()));
}

void MainWindow::editMemoryCell(int row, int column)
{
    if (runTimer->isActive())
    {
        setStatus(QStringLiteral("Pause the machine before editing memory."), true);
        return;
    }

    const int address = row * 16 + column;
    bool ok = false;

    const QString text = QInputDialog::getText(
        this,
        QStringLiteral("Edit memory"),
        QStringLiteral("Address %1 — hexadecimal byte (00-FF):").arg(hexWord(address)),
        QLineEdit::Normal,
        hexByte(machine.getMemory().read(address)),
        &ok);

    if (!ok)
    {
        return;
    }

    QString valueText = text.trimmed();
    if (valueText.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
    {
        valueText = valueText.mid(2);
    }

    bool valid = false;
    const uint value = valueText.toUInt(&valid, 16);

    if (valueText.isEmpty() || !valid || value > 0xFF)
    {
        showError(QStringLiteral("Enter a hexadecimal byte from 00 to FF."));
        return;
    }

    machine.getMemory().write(address, static_cast<uint8_t>(value));
    refreshViews();
    setStatus(QStringLiteral("Memory[%1] = %2.").arg(hexWord(address), hexByte(value)));
}

void MainWindow::showAbout()
{
    QMessageBox::about(
        this,
        QStringLiteral("About Vole Machine Simulator"),
        QStringLiteral(
            "<h3>Vole Machine Simulator</h3>"
            "<p>A Qt 6 front end for the Vole machine simulator.</p>"
            "<p>Load a hexadecimal program, then run it as a whole or step through it "
            "one instruction at a time. The registers, memory, program counter, "
            "instruction register and screen are updated after every step.</p>"
            "<p><b>Shortcuts:</b> F5 run/pause, F10 step, Shift+F5 stop, Ctrl+R reset.</p>"));
}

// ---------------------------------------------------------------------------
// View refresh
// ---------------------------------------------------------------------------

void MainWindow::refreshViews(bool highlightChanges)
{
    Memory &memory = machine.getMemory();
    Register &registers = machine.getRegisterFile();
    ControlUnit &controlUnit = machine.getControlUnit();

    const int pc = controlUnit.getPC();
    const int ir = controlUnit.getIR();

    const QColor pcBackground(QStringLiteral("#2dd4bf"));
    const QColor pcForeground(QStringLiteral("#06212b"));
    const QColor changedBackground(245, 158, 11, 120);
    const QColor normalForeground(QStringLiteral("#dbe5f1"));
    const QColor zeroForeground(QStringLiteral("#5c6b80"));

    for (int row = 0; row < 16; ++row)
    {
        for (int column = 0; column < 16; ++column)
        {
            const int address = row * 16 + column;
            const uint8_t value = memory.read(address);

            QTableWidgetItem *item = memoryTable->item(row, column);
            if (item == nullptr)
            {
                continue;
            }

            item->setText(hexByte(value));

            const bool isPc =
                pc >= 0 && (address == pc || address == pc + 1);
            const bool changed =
                highlightChanges && haveSnapshot && previousMemory[address] != value;

            if (isPc)
            {
                item->setBackground(QBrush(pcBackground));
                item->setForeground(QBrush(pcForeground));
            }
            else if (changed)
            {
                item->setBackground(QBrush(changedBackground));
                item->setForeground(QBrush(normalForeground));
            }
            else
            {
                item->setData(Qt::BackgroundRole, QVariant());
                item->setForeground(QBrush(value == 0 ? zeroForeground : normalForeground));
            }

            item->setToolTip(QStringLiteral("Address %1: %2 (decimal %3)")
                                 .arg(hexWord(address), hexByte(value))
                                 .arg(value));
        }
    }

    for (int index = 0; index < 16; ++index)
    {
        const uint8_t value = registers.read(index);
        const bool changed =
            highlightChanges && haveSnapshot && previousRegisters[index] != value;

        const int row = index % 8;
        const int baseColumn = (index / 8) * 2;

        QTableWidgetItem *nameItem = registerTable->item(row, baseColumn);
        QTableWidgetItem *hexItem = registerTable->item(row, baseColumn + 1);

        nameItem->setText(QStringLiteral("R") + hexDigit(index));
        hexItem->setText(hexValue(value));

        for (QTableWidgetItem *item : { nameItem, hexItem })
        {
            if (changed)
            {
                item->setBackground(QBrush(changedBackground));
            }
            else
            {
                item->setData(Qt::BackgroundRole, QVariant());
            }
            item->setForeground(QBrush(normalForeground));
        }

        const QString toolTip =
            QStringLiteral("R%1 = %2 (unsigned %3, signed %4)")
                .arg(hexDigit(index), hexValue(value))
                .arg(value)
                .arg(static_cast<int>(static_cast<int8_t>(value)));
        nameItem->setToolTip(toolTip);
        hexItem->setToolTip(toolTip);
    }

    pcValue->setText(pc < 0 ? QStringLiteral("----") : hexWord(pc));
    irValue->setText(hexWord(ir));

    QString stateText;
    QColor stateColor;

    if (lastStepFailed)
    {
        stateText = QStringLiteral("ERROR");
        stateColor = QColor(QStringLiteral("#f87171"));
    }
    else if (runTimer->isActive())
    {
        stateText = QStringLiteral("RUNNING");
        stateColor = QColor(QStringLiteral("#2dd4bf"));
    }
    else if (machine.isRunning())
    {
        stateText = QStringLiteral("PAUSED");
        stateColor = QColor(QStringLiteral("#fbbf24"));
    }
    else if (programLoaded)
    {
        const bool halted = (pc < 0 || pc >= 256);
        stateText = halted ? QStringLiteral("HALTED") : QStringLiteral("READY");
        stateColor = halted ? QColor(QStringLiteral("#94a3b8"))
                            : QColor(QStringLiteral("#60a5fa"));
    }
    else
    {
        stateText = QStringLiteral("IDLE");
        stateColor = QColor(QStringLiteral("#64748b"));
    }

    stateValue->setText(stateText);
    stateValue->setStyleSheet(QStringLiteral("color: %1;").arg(stateColor.name()));

    if (!programLoaded)
    {
        instructionValue->setText(QStringLiteral("—"));
    }
    else if (pc >= 0 && pc <= 254)
    {
        const int rawInstruction =
            (static_cast<int>(memory.read(pc)) << 8) | static_cast<int>(memory.read(pc + 1));
        instructionValue->setText(describeInstruction(rawInstruction));
    }
    else if (pc < 0)
    {
        // The machine halted: show the last instruction that was fetched.
        instructionValue->setText(ir != 0 ? describeInstruction(ir)
                                          : QStringLiteral("—"));
    }
    else
    {
        instructionValue->setText(QStringLiteral("End of memory"));
    }

    const QString screen = QString::fromStdString(machine.getScreen());
    if (screenView->toPlainText() != screen)
    {
        screenView->setPlainText(screen);
        QScrollBar *scrollBar = screenView->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }

    for (int address = 0; address < 256; ++address)
    {
        previousMemory[address] = memory.read(address);
    }
    for (int index = 0; index < 16; ++index)
    {
        previousRegisters[index] = registers.read(index);
    }
    haveSnapshot = true;
}

void MainWindow::updateControlStates()
{
    const bool running = runTimer->isActive();

    runAction->setText(running ? QStringLiteral("&Pause") : QStringLiteral("&Run"));
    runAction->setIcon(style()->standardIcon(
        running ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
    runAction->setEnabled(programLoaded);

    stepAction->setEnabled(programLoaded && !running);
    stopAction->setEnabled(programLoaded && (machine.isRunning() || running));
    resetAction->setEnabled(true);
    loadAction->setEnabled(!running);
    loadButton->setEnabled(!running);
    startAddressEdit->setEnabled(!running);
    editor->setReadOnly(running);
}

void MainWindow::setStatus(const QString &text, bool error)
{
    statusBar()->showMessage(text, 0);
    statusBar()->setStyleSheet(error
                                   ? QStringLiteral("color: #f87171;")
                                   : QString());
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

int MainWindow::speedInterval() const
{
    switch (speedCombo->currentIndex())
    {
        case 0:
            return 500;
        case 1:
            return 250;
        case 2:
            return 80;
        default:
            return 10;
    }
}

int MainWindow::stepsPerTick() const
{
    switch (speedCombo->currentIndex())
    {
        case 0:
        case 1:
            return 1;
        case 2:
            return 2;
        default:
            return 10;
    }
}

QString MainWindow::describeInstruction(int rawInstruction) const
{
    const int opcode = (rawInstruction >> 12) & 0x0F;
    const int operand1 = (rawInstruction >> 8) & 0x0F;
    const int operand2 = (rawInstruction >> 4) & 0x0F;
    const int operand3 = rawInstruction & 0x0F;
    const int address = (operand2 << 4) | operand3;

    switch (opcode)
    {
        case 0x1:
            return QStringLiteral("LOAD   R%1, [%2]").arg(hexDigit(operand1), hexByte(address));
        case 0x2:
            return QStringLiteral("LOAD   R%1, #%2").arg(hexDigit(operand1), hexByte(address));
        case 0x3:
            return QStringLiteral("STORE  R%1, [%2]").arg(hexDigit(operand1), hexByte(address));
        case 0x4:
            if (operand1 != 0)
            {
                break;
            }
            return QStringLiteral("MOVE   R%1, R%2").arg(hexDigit(operand2), hexDigit(operand3));
        case 0x5:
            return QStringLiteral("ADD    R%1, R%2, R%3")
                .arg(hexDigit(operand1), hexDigit(operand2), hexDigit(operand3));
        case 0x6:
            return QStringLiteral("FADD   R%1, R%2, R%3")
                .arg(hexDigit(operand1), hexDigit(operand2), hexDigit(operand3));
        case 0x7:
            return QStringLiteral("OR     R%1, R%2, R%3")
                .arg(hexDigit(operand1), hexDigit(operand2), hexDigit(operand3));
        case 0x8:
            return QStringLiteral("AND    R%1, R%2, R%3")
                .arg(hexDigit(operand1), hexDigit(operand2), hexDigit(operand3));
        case 0x9:
            return QStringLiteral("XOR    R%1, R%2, R%3")
                .arg(hexDigit(operand1), hexDigit(operand2), hexDigit(operand3));
        case 0xA:
            if (operand2 != 0)
            {
                break;
            }
            return QStringLiteral("ROR    R%1, %2").arg(hexDigit(operand1), hexDigit(operand3));
        case 0xB:
            return QStringLiteral("JUMP   %1 if R%2 == R0")
                .arg(hexByte(address), hexDigit(operand1));
        case 0xC:
            if (rawInstruction == 0xC000)
            {
                return QStringLiteral("HALT");
            }
            break;
        default:
            break;
    }

    return QStringLiteral("Invalid instruction %1").arg(hexWord(rawInstruction));
}

QString MainWindow::hexDigit(int value)
{
    return QString::number(value & 0x0F, 16).toUpper();
}

QString MainWindow::hexByte(int value)
{
    return QStringLiteral("%1").arg(value & 0xFF, 2, 16, QLatin1Char('0')).toUpper();
}

QString MainWindow::hexValue(int value)
{
    // Shortest hex form, so 10..15 render as A..F instead of 0A..0F.
    return QString::number(value & 0xFF, 16).toUpper();
}

QString MainWindow::hexWord(int value)
{
    if (value < 0)
    {
        return QStringLiteral("----");
    }

    return QStringLiteral("%1").arg(value & 0xFFFF, 4, 16, QLatin1Char('0')).toUpper();
}

void MainWindow::showError(const QString &message)
{
    setStatus(QStringLiteral("Error: %1").arg(message.simplified()), true);
    QMessageBox::critical(this, QStringLiteral("Vole Machine Simulator"), message);
}

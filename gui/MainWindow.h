#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <array>
#include <cstdint>

#include "VoleMachine.h"

class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;
class QTimer;

// Qt Widgets front end for the Vole machine.
//
// The window owns a VoleMachine and only drives it through its public API.
// Long programs are executed one step per timer tick so the UI stays
// responsive and the user can watch the machine change state.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    // Loads a program from disk into the editor and into memory. Used by the
    // File > Open action and by the --program command-line option.
    void openProgramFile(const QString &path, bool runAfterLoad = false);

    // Selects the screen renderer: "hex", "dec"/"decimal" or "ascii".
    // Returns false when the name is not recognised.
    bool setScreenFormat(const QString &formatName);

private slots:
    void newProgram();
    void openProgram();
    void saveProgram();
    void loadEditorProgram();
    void toggleRun();
    void stepProgram();
    void stopProgram();
    void resetMachine();
    void onRunTimeout();
    void onSpeedChanged();
    void onScreenFormatChanged();
    void editMemoryCell(int row, int column);
    void showAbout();

private:
    void buildUi();
    void buildActions();
    void applyTheme();
    void connectSignals();

    void loadProgramText(const QString &text, int startAddress, const QString &sourceName);
    bool readStartAddress(int &address, QString &error) const;
    void restartAtProgramStart();
    bool stepMachine();
    void showError(const QString &message);

    void refreshViews(bool highlightChanges = true);
    void updateControlStates();
    void setStatus(const QString &text, bool error = false);

    int speedInterval() const;
    int stepsPerTick() const;

    QString describeInstruction(int rawInstruction) const;
    static QString hexDigit(int value);
    static QString hexByte(int value);
    static QString hexValue(int value);
    static QString hexWord(int value);

    VoleMachine machine;
    QTimer *runTimer = nullptr;

    QPlainTextEdit *editor = nullptr;
    QLineEdit *startAddressEdit = nullptr;
    QPlainTextEdit *screenView = nullptr;
    QTableWidget *memoryTable = nullptr;
    QTableWidget *registerTable = nullptr;

    QLabel *pcValue = nullptr;
    QLabel *irValue = nullptr;
    QLabel *stateValue = nullptr;
    QLabel *instructionValue = nullptr;
    QLabel *editorHint = nullptr;

    QPushButton *loadButton = nullptr;
    QComboBox *speedCombo = nullptr;
    QComboBox *screenFormatCombo = nullptr;

    QAction *newAction = nullptr;
    QAction *openAction = nullptr;
    QAction *saveAction = nullptr;
    QAction *loadAction = nullptr;
    QAction *runAction = nullptr;
    QAction *stepAction = nullptr;
    QAction *stopAction = nullptr;
    QAction *resetAction = nullptr;

    bool programLoaded = false;
    int programStartAddress = 0;
    QString currentFile;
    bool lastStepFailed = false;

    std::array<uint8_t, 256> previousMemory{};
    std::array<uint8_t, 16> previousRegisters{};
    bool haveSnapshot = false;
};

#endif // MAINWINDOW_H

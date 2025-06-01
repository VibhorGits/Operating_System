#include "jamshellwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QRegularExpression>
#include <QProcess>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLineEdit>

// --- ANSI escape code stripper ---
static QString stripAnsi(const QString &in) {
    static QRegularExpression re("\x1B\\[[0-9;]*[A-Za-z]");
    QString out = in;
    out.remove(re);
    return out;
}

// --- Constructor ---
JamShellWindow::JamShellWindow(QWidget *parent)
    : QMainWindow(parent),
      terminalProcess(nullptr)
{
    setWindowTitle("JAM Shell GUI");
    resize(1200, 800);
    showFullScreen();

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    banner = new QLabel("=========== JAM Shell ===========", this);
    banner->setAlignment(Qt::AlignCenter);
    banner->setStyleSheet("font-weight: bold; font-size: 16px;");
    mainLayout->addWidget(banner);

    outputPane = new QPlainTextEdit(this);
    outputPane->setReadOnly(true);
    outputPane->setStyleSheet("background-color: black; color: lightgreen; font-family: monospace; font-size: 14px;");
    mainLayout->addWidget(outputPane, 1);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLine = new QLineEdit(this);
    submitBtn = new QPushButton("Submit", this);
    closeJamboBtn = new QPushButton("Close Jambo", this);
    closeJamboBtn->setEnabled(false);

    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(submitBtn);
    inputLayout->addWidget(closeJamboBtn);
    mainLayout->addLayout(inputLayout);

    setCentralWidget(central);

    createMenus();

    connect(submitBtn, &QPushButton::clicked, this, &JamShellWindow::onSubmit);
    connect(inputLine, &QLineEdit::returnPressed, this, &JamShellWindow::onSubmit);
    connect(closeJamboBtn, &QPushButton::clicked, this, &JamShellWindow::onCloseJambo);

    // Embedded REPL process
    mainProcess = new QProcess(this);
    mainProcess->setProgram("./jam");
    mainProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(mainProcess, &QProcess::readyReadStandardOutput, this, &JamShellWindow::readProcessOutput);
    connect(mainProcess, &QProcess::readyReadStandardError,  this, &JamShellWindow::readProcessError);
    connect(mainProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int code, QProcess::ExitStatus) {
                outputPane->appendPlainText(QString("\n[Process finished with code %1]").arg(code));
            });
    mainProcess->start();

    // Check or start tmux session
    int tmuxStatus = QProcess::execute("tmux", {"has-session", "-t", "jamsession"});
    if (tmuxStatus != 0) {
        QProcess::startDetached("tmux", {"new-session", "-d", "-s", "jamsession", "./jam"});
        outputPane->appendPlainText("[tmux] Created jamsession");
    } else {
        outputPane->appendPlainText("[tmux] jamsession already running");
    }
}

// --- Destructor ---
JamShellWindow::~JamShellWindow()
{
    if (terminalProcess) {
        terminalProcess->kill();
        terminalProcess->deleteLater();
        terminalProcess = nullptr;
    }
    if (mainProcess) {
        mainProcess->kill();
        mainProcess->waitForFinished(2000);
        mainProcess->deleteLater();
        mainProcess = nullptr;
    }
}

// --- Create menus helper ---
void JamShellWindow::createMenus()
{
    addCommandMenu("General", {"help", "exit", "history", "alias name=command"});
    addCommandMenu("File Operations", {"jcreate <filename>", "jsave <filename>", "jedit <filename>", "jmodify <filename>", "jrename <old> <new>", "jexecute <filename>"});
    addCommandMenu("Search & Navigation", {"sgown <term>", "locate <term>", "cd <path>"});
    addCommandMenu("Scheduling", {"jschedule <file> [priority]", "jschedulexecute", "jscheduleview", "jschedulesave <filename>", "jschedulecancel <task_id>", "jschedulemodify <id> <cmd>"});
    addCommandMenu("Jambo", {"jambo", "jambo -l <filename>", "jambo -p <filename>", "jambo -s <filename>"});
}

// --- Add menu with commands ---
void JamShellWindow::addCommandMenu(const QString &title, const QStringList &commands)
{
    QMenu *menu = menuBar()->addMenu(title);
    for (const QString &cmd : commands) {
        QAction *act = menu->addAction(cmd);
        connect(act, &QAction::triggered, this, [this, cmd]() {
            inputLine->setText(cmd);
            inputLine->setFocus();
        });
    }
}

// --- onSubmit slot ---
void JamShellWindow::onSubmit()
{
    const QString cmd = inputLine->text().trimmed();
    if (cmd.isEmpty()) return;

    if (cmd.startsWith("jedit") || cmd.startsWith("jambo")) {
        // Always kill previous session first
        QProcess::execute("tmux", {"kill-session", "-t", "jamsession"});

        // Create new session running ./jam (or just a bash shell if you prefer)
        QProcess::execute("tmux", {"new-session", "-d", "-s", "jamsession", "./jam"});
        outputPane->appendPlainText("[tmux] Created fresh jamsession");

        // Open new gnome-terminal attached to it
        QProcess::startDetached("gnome-terminal", {"--", "tmux", "attach-session", "-t", "jamsession"});
        outputPane->appendPlainText("[INFO] Attached new terminal to jamsession");

        // Send typed command to tmux session
        QProcess::execute("tmux", {"send-keys", "-t", "jamsession", cmd, "Enter"});
        outputPane->appendPlainText(QString("[tmux] Sent command: %1").arg(cmd));

        closeJamboBtn->setEnabled(true);
    }
    else {
        if (mainProcess->state() == QProcess::Running) {
            mainProcess->write((cmd + "\n").toUtf8());
            outputPane->appendPlainText(QString("[REPL] Sent command: %1").arg(cmd));
        }
        closeJamboBtn->setEnabled(false);
    }

    inputLine->clear();
}

// --- onCloseJambo slot ---
void JamShellWindow::onCloseJambo()
{
    // Optionally kill the tmux session here if desired:
    // QProcess::execute("tmux", {"kill-session", "-t", "jamsession"});
    closeJamboBtn->setEnabled(false);
}

// --- readProcessOutput slot ---
void JamShellWindow::readProcessOutput()
{
    QByteArray data = mainProcess->readAllStandardOutput();
    QString out = stripAnsi(QString::fromUtf8(data));
    outputPane->appendPlainText(out.trimmed());
}

// --- readProcessError slot ---
void JamShellWindow::readProcessError()
{
    QByteArray data = mainProcess->readAllStandardError();
    QString out = stripAnsi(QString::fromUtf8(data));
    outputPane->appendPlainText("[ERROR] " + out.trimmed());
}

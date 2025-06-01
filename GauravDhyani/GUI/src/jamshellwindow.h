#ifndef JAMSHELLWINDOW_H
#define JAMSHELLWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>

class JamShellWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit JamShellWindow(QWidget *parent = nullptr);
    ~JamShellWindow();

private slots:
    void onSubmit();
    void onCloseJambo();

    void readProcessOutput();
    void readProcessError();

private:
    void createMenus();
    void addCommandMenu(const QString &title, const QStringList &commands);

    void ensureTmuxSessionRunning();
    void sendCommandToJamTerminal(const QString &cmd);

    void closeExternalTerminals();

    QString stripAnsiEscapes(const QString &input);

private:
    QProcess *mainProcess = nullptr;
    QProcess *terminalProcess = nullptr; // Not really needed in this version but kept for cleanup

    QLabel *banner = nullptr;
    QPlainTextEdit *outputPane = nullptr;
    QLineEdit *inputLine = nullptr;
    QPushButton *submitBtn = nullptr;
    QPushButton *closeJamboBtn = nullptr;
};

#endif // JAMSHELLWINDOW_H

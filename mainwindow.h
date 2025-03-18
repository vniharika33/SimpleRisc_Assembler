#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Helper function to convert int to binary string with 'bits' length
    QString toBinary(int value, int bits);

    // Process a single operand (Handles all addressing modes)
    void processOperand(
        const string &operand,
        vector<string> &current,
        int programCounter,
        const string &modifierBits,
        bool &isImmediate
        );

    // Generalized handler for instructions (MOV, NOT, CMP explicitly handled)
    void handleInstruction(
        const vector<string> &v,
        vector<string> &current,
        int programCounter,
        const string &opcode,
        const unordered_map<string, int> &labels
        );

    // Main function to assemble code
    void assembleCode(const QString &inputCode);

private slots:
    void on_btnLoad_clicked();  // Button handler for Compile button

private:
    Ui::MainWindow *ui;
private:
    void updateHexOutput(const QString &binaryResult);  // Function to generate HEX output

};

#endif  // MAINWINDOW_H

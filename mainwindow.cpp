#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <bitset>
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Connect the Compile button to the slot
    connect(ui->on_btnLoad_clicked, &QPushButton::clicked, this, &MainWindow::on_btnLoad_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}


// Helper function to convert int to binary string with 'bits' length
QString MainWindow::toBinary(int value, int bits) {
    QString binaryString;
    for (int i = bits - 1; i >= 0; --i) {
        binaryString.append((value & (1 << i)) ? '1' : '0');
    }
    return binaryString;
}

// Process a single operand (Handles all addressing modes)
void MainWindow::processOperand(
    const string &operand,
    vector<string> &current,
    int programCounter,
    const string &modifierBits,
    bool &isImmediate
    ) {
    if (operand[0] == '#') {  // Immediate Addressing
        isImmediate = true;
        current.push_back(modifierBits);
        int immediate = stoi(operand.substr(1));
        current.push_back(toBinary(immediate, 16).toStdString());
    }
    else if (operand[0] == 'R') {  // Register Direct Addressing
        isImmediate = false;
        if (operand.size() < 2 || !isdigit(operand[1])) { // Check for valid register format
            cerr << "Error: Invalid register operand '" << operand << "'." << endl;
            exit(1);
        }
        int reg = stoi(operand.substr(1));
        current.push_back(toBinary(reg, 4).toStdString());
    }
    else if (operand[0] == '[' && operand.back() == ']') {  // Register Indirect Addressing
        isImmediate = false;
        int reg = stoi(operand.substr(2, operand.size() - 3));
        current.push_back(toBinary(reg, 4).toStdString());
    }
    else if (operand.find("[") != string::npos && operand.find(",") == string::npos) {  // Base Offset Addressing
        isImmediate = false;
        size_t leftParen = operand.find("[");
        int offset = stoi(operand.substr(0, leftParen));
        int baseReg = stoi(operand.substr(leftParen + 2, operand.size() - leftParen - 3));
        current.push_back(toBinary(offset, 8).toStdString());
        current.push_back(toBinary(baseReg, 4).toStdString());
    }
    else if (operand.find("[") != string::npos && operand.find(",") != string::npos) {  // Base Index Offset Addressing
        isImmediate = false;
        size_t leftParen = operand.find("[");
        size_t comma = operand.find(",");
        int offset = stoi(operand.substr(0, leftParen));
        int baseReg = stoi(operand.substr(leftParen + 2, comma - leftParen - 2));
        int indexReg = stoi(operand.substr(comma + 2, operand.size() - comma - 3));
        current.push_back(toBinary(offset, 8).toStdString());
        current.push_back(toBinary(baseReg, 4).toStdString());
        current.push_back(toBinary(indexReg, 4).toStdString());
    }
    else if (operand.find("0x") == 0) {  // Memory Direct Addressing
        isImmediate = false;
        int memoryAddress = stoi(operand, nullptr, 16);
        current.push_back(toBinary(memoryAddress, 16).toStdString());
    }
    else {  // PC-Relative Addressing
        isImmediate = false;
        int targetAddress = stoi(operand);
        int pcRelativeOffset = targetAddress - programCounter;
        current.push_back(toBinary(pcRelativeOffset, 16).toStdString());
    }
}

// Generalized handler for instructions
void MainWindow::handleInstruction(const vector<string> &v,vector<string> &current,int programCounter,const string &opcode,const unordered_map<string, int> &labels
    ) {
    bool isImmediate = false; // Track addressing mode
    string modifierBits = "00"; // Default modifier bits for immediate values
    if (opcode == "CMP") {
        // Process operands for instruction encoding
        processOperand(v[1], current, programCounter, "00", isImmediate);
        if(v[2][0]=='#')isImmediate=true;
        current.insert(current.begin() + 1, isImmediate ? "1" : "0");// Insert addressing mode bit
        current.insert(current.begin() + 2, "0000");                 // Add unused bits
        processOperand(v[2], current, programCounter, "00", isImmediate);
    }

    else if (opcode == "MOV"||opcode == "NOT") { // MOV and NOT  instructions


        processOperand(v[1], current, programCounter, modifierBits, isImmediate); // Destination register
        current.push_back("0000"); // Add unused bits (4 zeros for MOV and NOT)
        processOperand(v[2], current, programCounter, modifierBits, isImmediate); // Source operand
        current.insert(current.begin() + 1, isImmediate ? "1" : "0"); // Insert addressing mode bit
    }

    else if ( opcode == "B"||opcode=="CALL") {
        // Determine the PC-relative offset
        int targetAddress = stoi(v[1]); // Target address
        int pcRelativeOffset = targetAddress - (programCounter + 4); // Calculate PC-relative offset

        // Convert the PC-relative offset to a 27-bit binary string
        string offsetBits = bitset<27>(pcRelativeOffset).to_string(); // Convert offset to 27-bit binary

        // Add only the 27-bit offset to the current vector (opcode is already handled in main)
        current.push_back(offsetBits);
    }
    else if (opcode == "BGT" ||opcode=="BEQ") {
        // Extract the target label from the instruction
        const string &label = v[1];

        // Check if the label exists in the labels map
        if (labels.find(label) == labels.end()) {
            cerr << "Error: Undefined label '" << label << "' in BGT instruction." << endl;
            exit(1);
        }

        // Resolve the target address of the label
        int targetAddress = labels.at(label);

        // Calculate the PC-relative offset
        int pcRelativeOffset = targetAddress - (programCounter + 4);

        string binaryValue= bitset<32>(pcRelativeOffset).to_string();
        current.push_back(binaryValue.substr(5));


    }

    else if (opcode == "LD" || opcode == "ST") {
        // Extract destination/source register (RD)
        int rd = stoi(v[1].substr(1)); // Extract R1 as an integer

        // Determine if the offset is a register or immediate
        size_t leftBracket = v[2].find('[');
        size_t rightBracket = v[2].find(']');
        string offset = v[2].substr(0, leftBracket); // Extract offset (could be a register or immediate)
        int rs1 = stoi(v[2].substr(leftBracket + 2, rightBracket - leftBracket - 2)); // Extract base register (RS1)

        string addressingBit; // Define the 6th bit

        if (offset[0] == 'R') {
            // R-type (Register offset)

            addressingBit = "0"; // 6th bit is 0 for R-type
            current.push_back(addressingBit);    // Add 6th bit (R-type)
            int rs2 = stoi(offset.substr(1)); // Extract offset register (RS2)
            current.push_back(toBinary(rd, 4).toStdString());   // Add RD (4 bits)
            current.push_back(toBinary(rs1, 4).toStdString()); // Add RS1 (4 bits)
            current.push_back(toBinary(rs2, 4).toStdString()); // Add RS2 (4 bits)


            current.push_back("00000000");       // Add 8 unused bits
        } else {
            // I-type (Immediate offset)
            addressingBit = "1"; // 6th bit is 1 for I-type
            current.push_back(addressingBit);    // Add 6th bit (I-type)
            int imm = stoi(offset); // Convert immediate value
            current.push_back(toBinary(rd, 4).toStdString());  // Add RD (4 bits)
            current.push_back(toBinary(rs1, 4).toStdString()); // Add RS1 (4 bits)
            current.push_back(toBinary(imm, 18).toStdString()); // Add Immediate (8 bits)

        }
    }
    else{
        for (int i = 1; i < v.size(); ++i) {
            processOperand(v[i], current, programCounter, modifierBits, isImmediate); // Process each operand
        }
        current.insert(current.begin() + 1, isImmediate ? "1" : "0"); // Insert addressing mode bit
    }
}

// Main function to assemble code
void MainWindow::assembleCode(const QString &inputCode) {
    vector<vector<string>> v; // Store parsed input instructions
    unordered_map<string, int> labels; // Map to store label addresses

    stringstream ss(inputCode.toStdString());
    string line;
    int programCounter = 0;

    // **First Pass: Record Label Addresses**
    while (getline(ss, line)) {
        // Remove comments starting with `//`
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos); // Keep only the part before `//`
        }
        istringstream ssLine(line);
        vector<string> temp;
        string word;

        while (ssLine >> word) {
            if (word.back() == ':') {
                string label = word.substr(0, word.size() - 1);
                labels[label] = programCounter;
            } else {
                temp.push_back(word);
            }
        }

        if (!temp.empty()) {
            v.push_back(temp);
            programCounter += 4;
        }
    }

    // ✅ Add OPCODES and EXPECTED_OPERANDS here in assembleCode() only!
    unordered_map<string, string> OPCODES = {
        {"ADD", "00000"}, {"SUB", "00001"}, {"MUL", "00010"}, {"DIV", "00011"},
        {"MOD", "00100"}, {"CMP", "00101"}, {"AND", "00110"}, {"OR", "00111"},
        {"NOT", "01000"}, {"MOV", "01001"}, {"LSL", "01010"}, {"LSR", "01011"},
        {"ASR", "01100"}, {"LD", "01110"}, {"ST", "01111"},
        {"BEQ", "10000"}, {"BGT", "10001"}, {"B", "10010"}, {"CALL", "10011"},
        {"RET", "10100"}, {"HLT", "11111"}, {"NOP", "11111"}
    };

    // ✅ Define the expected operand counts
    unordered_map<string, int> EXPECTED_OPERANDS = {
        {"ADD", 4}, {"SUB", 4}, {"MUL", 4}, {"DIV", 4},
        {"MOD", 4}, {"CMP", 3}, {"AND", 4}, {"OR", 4},
        {"NOT", 3}, {"MOV", 3}, {"LSL", 4}, {"LSR", 4},
        {"ASR", 4}, {"LD", 3}, {"ST", 3}, {"BEQ", 2},
        {"BGT", 2}, {"B", 2}, {"CALL", 2}, {"RET", 1},
        {"HLT", 1}, {"NOP", 1}
    };

    // Reset PC for the second pass
    programCounter = 0;

    // **Second Pass: Parse and Process Instructions**
    QString result;
    QString hexResult;
    for (const auto &instruction : v) {
        if (instruction.empty()) continue;

        const string &opCode = instruction[0];
        if (OPCODES.find(opCode) == OPCODES.end()) {
            result += QString::fromStdString("Error: Invalid instruction " + opCode + "\n");
            continue;
        }

        int expectedCount = EXPECTED_OPERANDS[opCode];
        if (instruction.size() != expectedCount) {
            result += QString::fromStdString(
                "Error: Invalid number of operands for " + opCode +
                ". Expected " + to_string(expectedCount - 1) +
                ", found " + to_string(instruction.size() - 1) + "\n");
            continue;
        }

        vector<string> current;
        current.push_back(OPCODES[opCode]);

        // Process the instruction and its operands
        handleInstruction(instruction, current, programCounter, opCode, labels);

        // Combine binary parts into a single 32-bit instruction
        string binaryInstruction;
        for (const auto &part : current) {
            binaryInstruction += part;
        }
        while (binaryInstruction.size() < 32) {
            binaryInstruction += "0";
        }

        result += QString::fromStdString(binaryInstruction) + "\n";
        // Convert binary to hexadecimal



        programCounter += 4; // Increment PC by 4 (32-bit instruction)
    }
    // Generate hex output from binary output

    QStringList binaryLines = result.split('\n', Qt::SkipEmptyParts);  // Split binary into lines

    for (const QString &binaryLine : binaryLines) {
        bool ok;
        // Convert binary to a 32-bit unsigned integer
        quint32 decimalValue = binaryLine.toULongLong(&ok, 2);
        if (ok) {
            // Convert to 8-character hexadecimal representation
            QString hexLine = QString::number(decimalValue, 16).toUpper().rightJustified(8, '0');
            hexResult +="0x"+ hexLine + "\n";  // Add hex value for each binary line
        } else {
            hexResult += "ERROR\n";  // Handle error if binary is invalid
        }
    }


    // Display the output in the txtOutput field
    ui->txtOutput->setText(result);
    ui->txtHexOutput->setText(hexResult);
}

// Slot triggered when Compile button is clicked
void MainWindow::on_btnLoad_clicked() {
    QString inputCode = ui->textEdit->toPlainText();
    if (inputCode.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Input is empty! Please enter code.");
        return;
    }
    assembleCode(inputCode);
}








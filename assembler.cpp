#include <bits/stdc++.h>
using namespace std;


// Helper function to convert to binary
string toBinary(int value, int bits) {
    return bitset<16>(value).to_string().substr(16 - bits, bits);
}

// Process a single operand (Handles all addressing modes)
void processOperand(const string &operand, vector<string> &current, int programCounter, const string &modifierBits, bool &isImmediate) {
    if (operand[0] == '#') { // Immediate Addressing
        isImmediate = true; // Set 6th bit to 1 for immediate
        current.push_back(modifierBits); // Add modifier bits (default "00")
        int immediate = stoi(operand.substr(1)); // Strip # and convert to integer
        current.push_back(toBinary(immediate, 16)); // Add 16-bit binary immediate value
    } else if (operand[0] == 'R') { // Register Direct Addressing
        isImmediate = false; // Set 6th bit to 0 for register
        if (operand.size() < 2 || !isdigit(operand[1])) { // Check for valid register format
            cerr << "Error: Invalid register operand '" << operand << "'." << endl;
            exit(1);
        }
        int reg = stoi(operand.substr(1)); // Extract register number
        current.push_back(toBinary(reg, 4)); // Add 4-bit register binary
    } else if (operand[0] == '[' && operand.back() == ']') { // Register Indirect Addressing
        isImmediate = false; // Set 6th bit to 0 for register indirect
        int reg = stoi(operand.substr(2, operand.size() - 3)); // Extract register number
        current.push_back(toBinary(reg, 4)); // Add 4-bit register binary
    } else if (operand.find("[") != string::npos && operand.find(",") == string::npos) { // Base Offset Addressing
        isImmediate = false; // Set 6th bit to 0 for base-offset
        size_t leftParen = operand.find("[");
        int offset = stoi(operand.substr(0, leftParen)); // Extract offset
        int baseReg = stoi(operand.substr(leftParen + 2, operand.size() - leftParen - 3)); // Extract base register
        current.push_back(toBinary(offset, 8)); // Add 8-bit offset
        current.push_back(toBinary(baseReg, 4)); // Add 4-bit base register
    } else if (operand.find("[") != string::npos && operand.find(",") != string::npos) { // Base Index Offset Addressing
        isImmediate = false; // Set 6th bit to 0 for base-index-offset
        size_t leftParen = operand.find("[");
        size_t comma = operand.find(",");
        int offset = stoi(operand.substr(0, leftParen)); // Extract offset
        int baseReg = stoi(operand.substr(leftParen + 2, comma - leftParen - 2)); // Extract base register
        int indexReg = stoi(operand.substr(comma + 2, operand.size() - comma - 3)); // Extract index register
        current.push_back(toBinary(offset, 8)); // Add 8-bit offset
        current.push_back(toBinary(baseReg, 4)); // Add 4-bit base register
        current.push_back(toBinary(indexReg, 4)); // Add 4-bit index register
    } else if (operand.find("0x") == 0) { // Memory Direct Addressing
        isImmediate = false; // Set 6th bit to 0 for memory direct
        int memoryAddress = stoi(operand, nullptr, 16); // Convert hex address
        current.push_back(toBinary(memoryAddress, 16)); // Add 16-bit memory address
    } else { // PC-Relative Addressing
        isImmediate = false; // Set 6th bit to 0 for PC-relative
        int targetAddress = stoi(operand); // Target address
        int pcRelativeOffset = targetAddress - programCounter; // Calculate PC-relative offset
        current.push_back(toBinary(pcRelativeOffset, 16)); // Add 16-bit signed offset
    }
}

// Generalized handler for instructions (MOV, NOT, CMP explicitly handled)
void handleInstruction(const vector<string> &v, vector<string> &current, int programCounter, const string &opcode, const unordered_map<string, int> &labels) {
    bool isImmediate = false; // Track addressing mode
    string modifierBits = "01"; // Default modifier bits for immediate values
    if (opcode == "CMP") {
    // Process operands for instruction encoding
    processOperand(v[1], current, programCounter, "00", isImmediate);
    if(v[2][0]=='#')isImmediate=true;
        current.insert(current.begin() + 1, isImmediate ? "1" : "0");// Insert addressing mode bit
    current.insert(current.begin() + 2, "0000");                 // Add unused bits
    processOperand(v[2], current, programCounter, "00", isImmediate);
}

    else if (opcode == "MOV") { // MOV and NOT  instructions
        
        
        processOperand(v[1], current, programCounter, modifierBits, isImmediate); // Destination register
        current.push_back("0000"); // Add unused bits (4 zeros for MOV and NOT)
        processOperand(v[2], current, programCounter, modifierBits, isImmediate); // Source operand
        current.insert(current.begin() + 1, isImmediate ? "1" : "0"); // Insert addressing mode bit
    }
    else if (opcode == "BGT" ||opcode=="BEQ"||opcode == "B"||opcode=="CALL") {
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
        current.push_back(toBinary(rd, 4));   // Add RD (4 bits)
        current.push_back(toBinary(rs1, 4)); // Add RS1 (4 bits)
        current.push_back(toBinary(rs2, 4)); // Add RS2 (4 bits)
        
        
        current.push_back("00000000");       // Add 8 unused bits
    } else {
        // I-type (Immediate offset)
        addressingBit = "1"; // 6th bit is 1 for I-type
        current.push_back(addressingBit);    // Add 6th bit (I-type)
        int imm = stoi(offset); // Convert immediate value
        current.push_back(toBinary(rd, 4));  // Add RD (4 bits)
        current.push_back(toBinary(rs1, 4)); // Add RS1 (4 bits)
        current.push_back(toBinary(imm, 18)); // Add Immediate (8 bits)
        
    }
   } 
   else{
    for (int i = 1; i < v.size(); ++i) {
            processOperand(v[i], current, programCounter, modifierBits, isImmediate); // Process each operand
    }    
        current.insert(current.begin() + 1, isImmediate ? "1" : "0"); // Insert addressing mode bit
    }
  }

int main() {
    vector<vector<string>> v; // Store parsed input instructions
    unordered_map<string, int> labels; // Map to store label addresses
    ifstream file("input.asm"); // Open input file containing assembly instructions

    if (!file) {
        cerr << "Error: File not found!" << endl; // Error message if file cannot be opened
        return 1; // Return with an error code
    }

    string line;
    int programCounter = 0; // Initialize the program counter

    // **First Pass: Record Label Addresses**
    while (getline(file, line)) {
         // Remove comments starting with `//`
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos); // Keep only the part before `//`
        }
        istringstream ss(line);
        vector<string> temp;
        string word;

        while (ss >> word) {
            if (word.back() == ':') { // Detect labels
                string label = word.substr(0, word.size() - 1); // Remove ':' from the label
                labels[label] = programCounter; // Record the label address
            } else {
                temp.push_back(word); // Add instruction/operand to temporary vector
            }
        }

        if (!temp.empty()) {
            v.push_back(temp); // Add the parsed instruction to the list
            programCounter += 4; // Increment PC (assuming 32-bit instructions)
        }
    }
    file.close(); // Close the input file after the first pass

    // Define the opcode mappings
    unordered_map<string, string> OPCODES = {
        {"ADD", "00000"}, {"SUB", "00001"}, {"MUL", "00010"}, {"DIV", "00011"},
        {"MOD", "00100"}, {"CMP", "00101"}, {"AND", "00110"}, {"OR", "00111"},
        {"NOT", "01000"}, {"MOV", "01001"}, {"LSL", "01010"}, {"LSR", "01011"},
        {"ASR", "01100"}, {"LD", "01110"}, {"ST", "01111"},
        {"BEQ", "10000"}, {"BGT", "10001"}, {"B", "10010"}, {"CALL", "10011"},
        {"RET", "10100"}, {"HLT", "11111"}, {"NOP", "11111"}
    };

    // Define expected operand counts for each instruction
    unordered_map<string, int> EXPECTED_OPERANDS = {
        {"ADD", 4}, {"SUB", 4}, {"MUL", 4}, {"DIV", 4},
        {"MOD", 4}, {"CMP", 3}, {"AND", 4}, {"OR", 4},
        {"NOT", 3}, {"MOV", 3}, {"LSL", 4}, {"LSR", 4},
        {"ASR", 4}, {"LD", 3}, {"ST", 3}, {"BEQ", 2},
        {"BGT", 2}, {"B", 2}, {"CALL", 2}, {"RET", 1},
        {"HLT", 1}, {"NOP", 1}
    };

    programCounter = 0; // Reset PC for the second pass

    ofstream outputBinary("output.bin");
    ofstream outputHex("output.hex");
    if (!outputBinary || !outputHex) {
    cerr << "Error: Could not create output files!" << endl;
    return 1;
    }

    // **Second Pass: Parse and Process Instructions**
    for (const auto &instruction : v) {
        if (instruction.empty()) continue; // Skip empty instructions

        const string &opCode = instruction[0];
        if (OPCODES.find(opCode) == OPCODES.end()) {
            cerr << "Error: Invalid instruction " << opCode << endl; // Handle invalid opcodes
            continue;
        }

        int expectedCount = EXPECTED_OPERANDS[opCode];
        if (instruction.size() != expectedCount) {
            cerr << "Error: Invalid number of operands for instruction " << opCode
                 << ". Expected " << expectedCount - 1 << ", found " << instruction.size() - 1 << endl;
            continue;
        }

        vector<string> current;
        current.push_back(OPCODES[opCode]); // Add the opcode to the binary representation

        // Process the instruction and its operands
        handleInstruction(instruction, current, programCounter, opCode, labels);

        // Combine the parts of the binary instruction into a single 32-bit binary string
        string binaryInstruction;
        for (const auto &part : current) {
            binaryInstruction += part;
        }
        while (binaryInstruction.size() < 32) {
            binaryInstruction += "0"; // Add padding to make it 32 bits
        }

        // Convert binary instruction to hexadecimal
        stringstream hexStream;
        hexStream << hex << uppercase << stoull(binaryInstruction, nullptr, 2);
        string hexInstruction ="0x"+ hexStream.str();

        // Write binary and hex values to their respective files
        outputBinary << binaryInstruction << endl;
        outputHex << hexInstruction << endl;

        programCounter += 4; // Increment the program counter (assuming 32-bit instruction length)
        
        

        programCounter += 4; // Increment the program counter (assuming 32-bit instruction length)
    }

    outputBinary.close(); // Close the output file
    outputHex.close(); // Close the hex file
    return 0; // Return successful exit code
}
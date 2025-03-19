
# **SimpleRISC Assembler**

This project is a **custom assembler(in C++)** that converts assembly code into **32-bit binary** machine code. It supports multiple addressing modes, handles labels, and provides robust error checking..

## **Project Overview**

The assembler reads the input, processes it through a **two-pass** approach, and outputs the equivalent **binary machine code**.

* **First Pass:** Records label addresses.
* **Second Pass:** Encodes each instruction into **32-bit binary**.

We have provided two versions of our project.

1.**Qt version** - in master branch of the repository.

We have used Qt(C++) for the front-end part(for UI).After installation of Qt and running it we can give the input in the UI and output in different forms will be displayed on the screen in output boxes.

2.Normal version which can be run in any IDE like **VSCode –** in main branch

In this branch there are three files :-

**input.asm : The assembly source code.**

**output.bin : The assembled binary output.**

**output.hex : The assembled hexadecimal output.**

**assembler.cpp : Our main code is there in**

**Features**

### **Instruction Set Support**

Supports various instructions including:

* **Arithmetic Operations**: ADD, SUB, MUL, DIV, MOD
* **Logical Operations**: AND, OR, NOT, CMP
* **Data Transfer**: MOV, LD, ST
* **Shift Operations**: LSL, LSR, ASR
* **Branching**: BEQ, BGT, B, CALL, RET
* **Miscellaneous**: HLT, NOP

###

###

###

### **Addressing Modes**

Supports multiple operand formats:

* **Immediate Addressing**: #10
* **Register Direct Addressing**: R1
* **Register Indirect Addressing**: [R2]
* **Base-Offset Addressing**: 4[R3]
* **Base-Index Offset Addressing**: 4[R4,R5]
* **Memory Direct Addressing**: 0x1000
* **PC-Relative Addressing**: Labels (e.g., loop:)

### **Label Support**

The assembler supports **labels** for jumps and subroutine calls. It resolves label addresses during the **first pass**.

### **Error Handling**

* **Invalid Opcode**: Detects unknown instructions.
* **Operand Mismatch**: Validates correct operand counts.
* **Undefined Labels**: Checks for missing labels during branch instructions. Incase of undefined labels the program is not going to be executed and error will be shown in the terminal.

Errors are displayed and logged during assembly.

 **How to Use**

 **Compile the Assembler**

Ensure you have Qt creator and MinGW in order to view the UI part. Else we have also provided normal code in main branch which can be used in VSCode or any online compiler.

Incase of Qt creator after clicking on run you will be directed to the front-end in which there will be an input area where you can provide the code and then click on assemble.The output will be shown in the output area .

Ex:- ![](data:image/png;base64...)

###

 **Example input.asm:**

MOV R1 #10

ADD R2 R1 #5

CMP R2 #15

BGT TARGET1

B TARGET2

LD R2 10[R3]

ST R2 R3[10]

TARGET1: HLT

TARGET2: MOV R3 R4

Rules-

1. ‘#’ Must be written before immediate values.
2. Only 1 space must be used between all operands.(No commas must be used).
3. Instructions must be written in capital letters only.
4. For label ,write the name of the label then colon and then give some space and then give the rest of the code..as shown in the example.
5. Formatting for LD and ST instruction is shown in the example above and also we have included all the addressing modes through which we can address in the prev page.
6. For comments use //.
7. Incase of branch instruction offset=targetaddress-(pc+4) will be shown.

### **Output**

The output will display the hex code as well as the binary code .

If you are not using Qt (i.e,if using **VSCode**)then output will be saved in 2 files output.bin and output.hex .

## **Internals of the Assembler**

### **First Pass: Label Recording**

1. Reads the input assembly file line by line.
2. Identifies and **records** label addresses.

Example:

LOOP: ADD R1 R2 R3

B LOOP

Records:

labels["LOOP"] = 0 //records the pc value of the loop in the map.

### **Second Pass: Binary Generation**

1. Parses instructions and operands.
2. Encodes each instruction into a **32-bit** binary format.

## **Limitations**

1. We have taken default modifier bits as 01.

2. U and H operations such as MOVU and MOVH have not been added.

**Difficulties Faced**

1.Installation of Qt for the front-end part was very difficult for us as we did not have any background of development and after installation also it was showing many errors.It was a very tedious task.

2.We did not understand how to change the modifier bit and how to give input for that hence We kept default as 01(unsigned).

3.Lot of time was wasted for making bgt and beq instructions as at first we thought that we have to make flag registers and an array of registers and then store the actual immediate data that is being given and compare that data and then update the flag registers but afterwards we got to know that we just directly have to calculate the offset regardless of the value.

4.We also did not know many inbuilt functions like bitset,etc in cpp which we discovered while doing this project which made it somewhat easier for us. Actually we made a function toBinary in order to convert it into binary which was not required.

5.Writing of the code is easy only ,but it is somewhat long as each and every instruction had to be handled properly



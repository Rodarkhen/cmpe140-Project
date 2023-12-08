/*  Group Names: David Wei, Matthew Malvini, Michelle Fang, Rodrigo Chen, Sammy Cuaderno
    Course: CMPE 140
    Assignment: RISCV Project
    Date: 12/08/2023
    Descrption: This program emulates a five stage RISCV CPU.
*/
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstring>
#include <iomanip>
#include "instructions.h"

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        std::cout << "Wrong USAGE" << std::endl;
        exit(1);
    }
    instructionMemory in_instructions[50]; //  the 32 bits instruction place in an array
    Reg registers[32];                     // 32 registers
    Dmem mem[128];                         // 128 mem

    int32_t initialAddr = 268500992;
    for (int i = 0; i < 128; i++)
    {
        mem[i].address = initialAddr;
        mem[i].data = 0;
        initialAddr += 4;
    }

    int PC = 0;     // Keeping track of PC
    int toJump = 0; // For jump instructions

    std::string filename = argv[1];
    // Opening file
    std::ifstream inputFile;
    inputFile.open(filename);
    if (!inputFile.is_open()) // if fail to open
    {
        std::cout << "Unable to open file" << std::endl;
        exit(1);
    }

    // Get instructions from files
    int lineCount = 0;
    std::string line;
    std::string subInstruction = "";
    int idx = 0; // index of mem

    // If second file param entered -> dmem file
    if (argc == 3) //./test line.dat dmem.dat
    {
        std::string dmemfilename = argv[2];
        std::ifstream dmemFile;
        dmemFile.open(dmemfilename);
        if (!dmemFile.is_open())
        {
            std::cout << "Unable to open file" << std::endl;
            exit(1);
        }
        while (getline(dmemFile, line))
        {
            subInstruction = line + subInstruction;
            lineCount++;
            if (lineCount % 4 == 0)
            {
                int32_t data = 0;
                lineCount = 0;

                data = (subInstruction[0] == '1')              // data is determined by index 0 the in order to be place as data
                           ? twosComplement(subInstruction)    // determines if it is negative or positive
                           : stol(subInstruction, nullptr, 2); // converts to decimals

                mem[idx].data = data;
                subInstruction = "";
                idx++;
            }
        }
    }
    // Variables for user inputs
    std::string currentInput = "";
    std::string prevInput = "";
    bool r = false; // runs everything
    bool brk = false;
    bool printMem = false;
    // Variables for decoding instructions
    int32_t immed = 0;
    int32_t rs1 = 0;
    int32_t rd = 0;
    int32_t opcode = 0;
    int32_t func3 = 0;
    int32_t R_immed = 0;
    int32_t rs2 = 0;
    int32_t StoreImmed = 0;
    int32_t UI_Immed = 0;
    int32_t Branch_Immed = 0;
    int32_t UJ_Immed = 0;
    bool branched = false;
    bool jumped = false;

    int index = 0;         // instruction number
    bool dmem_wen = false; // Write enable for data memory (1 for Store, 0 for Load)
    int count = 0;         // index for going through imem
    subInstruction = "";
    // Reads a file with instructions
    while (getline(inputFile, line))
    {
        if (line.size() == 32) // (32'b instruction per line)
        {
            in_instructions[index].instruction = line;
            index++;
        }
        else if (line.size() == 8) //'(8'b instruction per line * 4 )
        {
            subInstruction = line + subInstruction;
            lineCount++;
            if (lineCount % 4 == 0)
            {
                in_instructions[index].instruction = subInstruction;
                lineCount = 0;
                subInstruction = "";
                index++;
            }
        }
        else
        {
            std::cout << "wrong format for instruction. Please Check your instruction file" << std::endl;
            exit(1);
        }
    }

    // Shows available options fo the user
    printOptions();

    while (true)
    {
        //=============//
        // FETCH STAGE //
        //=============//
        if ((toJump == 0))
        {
            /*prevInput is not equal to "s" || prevInput is not equal to "pc" && brk is false && r is false.*/
            while ((prevInput.compare("s") || prevInput.compare("pc")) && !brk && !r)
            {
                std::cout << "-------------------------" << std::endl;
                std::cout << "Enter command: ";
                std::cin >> currentInput;
                std::cout << "- - - - - - - - - - - - -" << std::endl;
                std::string hex = currentInput.substr(0, 2);
                std::string reg = currentInput.substr(0, 1);

                if (currentInput.compare("r") == 0)
                { // Runs everything
                    prevInput = "r";
                    brk = true;
                    r = true;
                }
                else if (currentInput.compare("s") == 0)
                { // Runs next instruction + then stop
                    prevInput = "s";
                    brk = true;
                }
                else if (reg.compare("x") == 0)
                { // Prints reg value based on reg num
                    prevInput = "reg";
                    if (currentInput.size() == 1)
                    { // just input x->give error or else stops the prog
                        std::cout << "not valid reg" << std::endl;
                        brk = false;
                    }
                    else
                    {
                        std::string num = currentInput.substr(1, 2);
                        int n = stol(num, nullptr, 10);
                        std::cout << "contents for register " << n << ": 0x" << std::hex << std::setw(8) << std::setfill('0')
                                  << registers[n].value << std::dec << std::endl;
                        brk = false;
                    }
                }
                else if (hex.compare("0x") == 0)
                { // Prints mem data based on mem addr
                    prevInput = "hex";
                    std::string addr = currentInput.substr(2, 9);
                    int32_t a = stol(addr, nullptr, 10);
                    // std::cout << a << " | " << addr << std::endl;
                    for (int i = 0; i < 128; i++)
                    {
                        // std::cout << "mem Addr: " << mem[i].address << std::endl;
                        if (mem[i].address == a)
                        {
                            std::cout << "Contents from addr " << addr << ": " << std::dec << mem[i].data << std::endl;
                        }
                    }
                    brk = false;
                }
                else if (currentInput.compare("pc") == 0)
                { // prints pc
                    prevInput = "pc";
                    std::cout << "PC: " << PC << std::endl;
                    brk = false;
                }
                else
                    brk = false;
            }
            brk = false;
            //====================================================//

            //==============//
            // Decode STAGE //
            //==============//

            // Sort into I-type format //
            // imm[11:0]
            in_instructions[count].immed = in_instructions[count].instruction.substr(0, 12);
            immed = (in_instructions[count].immed[0] == '1') ? twosComplement(in_instructions[count].immed) : stol(in_instructions[count].immed, nullptr, 2);
            // rs1
            in_instructions[count].rs1 = in_instructions[count].instruction.substr(12, 5);
            rs1 = binToDec(stol(in_instructions[count].rs1, nullptr, 10));
            // func3
            in_instructions[count].func3 = in_instructions[count].instruction.substr(17, 3);
            func3 = stoi(in_instructions[count].func3, nullptr, 10);
            // rd
            in_instructions[count].rd = in_instructions[count].instruction.substr(20, 5);
            rd = binToDec(stol(in_instructions[count].rd, nullptr, 10));
            // opcode
            in_instructions[count].opcode = in_instructions[count].instruction.substr(25, 7);
            opcode = stoi(in_instructions[count].opcode, nullptr, 10);

            // R-type format //
            // R_immed
            in_instructions[count].R_immed = in_instructions[count].instruction.substr(0, 7);
            R_immed = binToDec(stol(in_instructions[count].R_immed, nullptr, 10));
            // rs2
            in_instructions[count].rs2 = in_instructions[count].instruction.substr(7, 5);
            rs2 = binToDec(stol(in_instructions[count].rs2, nullptr, 10));
            // rs1, func3, rd, opcode are similar to the I-Type format

            // Store format //
            in_instructions[count].StoreImmed = in_instructions[count].R_immed + in_instructions[count].rd;
            StoreImmed = binToDec(stol(in_instructions[count].StoreImmed, nullptr, 10));

            // U format //
            in_instructions[count].UI_Immed = in_instructions[count].instruction.substr(0, 20);
            UI_Immed = (in_instructions[count].UI_Immed[0] == '1') ? twosComplement(in_instructions[count].UI_Immed) : stol(in_instructions[count].UI_Immed, nullptr, 2);

            // UJ format //
            in_instructions[count].UJ_Immed = in_instructions[count].instruction.substr(0, 1) + in_instructions[count].instruction.substr(12, 8) + in_instructions[count].instruction.substr(11, 1) + in_instructions[count].instruction.substr(1, 10) + "0";
            UJ_Immed = (in_instructions[count].UJ_Immed[0] == '1') ? twosComplement(in_instructions[count].UJ_Immed) : stol(in_instructions[count].UJ_Immed, nullptr, 2);

            // Branch format --lowest bit offest is always 0
            in_instructions[count].Branch_Immed = in_instructions[count].R_immed.substr(0, 1) + in_instructions[count].rd.substr(4, 1) + in_instructions[count].R_immed.substr(1, 6) + in_instructions[count].rd.substr(0, 4) + '0';
            Branch_Immed = binToDec(stol(in_instructions[count].Branch_Immed, nullptr, 10));
            Branch_Immed = (in_instructions[count].Branch_Immed[0] == '1') ? twosComplement(in_instructions[count].Branch_Immed) : stol(in_instructions[count].Branch_Immed, nullptr, 2);
            //====================================================//

            //===============//
            // Execute STAGE //
            //===============//
            switch (opcode)
            {
            case I_type_format:
            {
                switch (func3)
                {
                case ADDI: // Addition with immediate
                {
                    // to add immediate value, we take the value from immediate and add it to the rs1.
                    // After we do this we store to the results from adding these two to the destination register rd.
                    std::cout << "ADDI " << registerNames[rd] << "," << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 + immed;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case STLI: // Pick the value of rs1 or registers[rs1].value
                {
                    std::cout << "STLI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    if (temp_rs1 < immed)
                        registers[rd].value = 1;
                    else
                        registers[rd].value = 0;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLTIU: // Set less than unsigned
                {           // set the value if temp_rs1 is less than imm value
                    std::cout << "SLTIU " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    if ((unsigned int)temp_rs1 < immed)
                        registers[rd].value = 1;
                    else
                        registers[rd].value = 0;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case XORI: // Exclusive or
                {
                    std::cout << "XORI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    registers[rd].value = temp_rs1 ^ immed;                          //
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case ORI: // Bitwise Or with immediate
                {
                    std::cout << "ORI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;                                      // d_write[rs1].value : rs1
                    registers[rd].value = temp_rs1 | immed;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case ANDI: // Bitwise And with immediate
                {
                    std::cout << "ANDI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 & immed;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLLI: // Shift left logical with immediate
                {
                    std::cout << "SLLI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 << immed;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SRLISRAI: // Arithmetic and logical right shift with immediate
                {
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;

                    // Immediate value contains the shift amount (lower 5 bits)
                    int shift_amount = immed & 0b00011111;
                    if (R_immed == 0b0100000)
                    {
                        std::cout << "SRAI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                        registers[rd].value = temp_rs1 >> shift_amount;
                    }
                    else
                    {
                        std::cout << "SRLI " << registerNames[rd] << ", " << registerNames[rs1] << ", " << immed << std::endl;
                        registers[rd].value = (unsigned)temp_rs1 >> shift_amount;
                    }
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                default:
                {
                    // std::cout << "not valid I-TYPE instruction" << std::endl;
                    break;
                }
                }
                break;
            } // End of I_type_format
            case R_type_format:
            {
                switch (func3)
                {
                case ADDSUB: // Addition and Subtraction
                {
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (R_immed == 0b0100000)
                    { // func7 code (0100000 = sub)
                        std::cout << "SUB " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                        registers[rd].value = temp_rs1 - temp_rs2;
                    }
                    else
                    {
                        std::cout << "ADD " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                        registers[rd].value = temp_rs1 + temp_rs2;
                    }
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLL: // Shift Left Logical
                {
                    std::cout << "SLL " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 << temp_rs2;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLT: //
                {
                    std::cout << "SLT " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;

                    if (temp_rs1 < temp_rs2)
                        registers[rd].value = 1;
                    else
                        registers[rd].value = 0;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLTU: // Set Less Than Imme (unsigned)
                {
                    std::cout << "SLTU " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 < temp_rs2)
                        registers[rd].value = 1;
                    else
                        registers[rd].value = 0;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case XOR: // Exclusion Or
                {
                    std::cout << "XOR " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 ^ temp_rs2;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SRLSRA: // Arithmetic and Logical Right Shift
                {
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    // Bitmask to extract last 5 bits of the shift amount
                    int bitmask = 0b00011111;
                    temp_rs2 = temp_rs2 & bitmask;
                    if (R_immed == 0b0100000)
                    { // shift amount is the last 5 bits of registers[rs2]
                        std::cout << "SRA " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                        registers[rd].value = temp_rs1 >> temp_rs2;
                    }
                    else
                    {
                        std::cout << "SRL " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                        registers[rd].value = (unsigned int)temp_rs1 >> temp_rs2;
                    }
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case OR: // Bitwise Or
                {
                    std::cout << "OR " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 | temp_rs2;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case AND: // Bitwise And
                {
                    std::cout << "AND " << registerNames[rd] << ", " << registerNames[rs1] << ", " << registerNames[rs2] << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 & temp_rs2;
                    registers[rd].used = true;
                    std::cout << "Result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                default:
                {
                    break;
                }
                }
                break;
            } // End of R_type_format
            case Branch_format:
            {
                switch (func3)
                {
                case BEQ: // Branch if equal
                {
                    std::cout << "BEQ " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 == temp_rs2)
                    {
                        std::cout << "Equal to" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }

                    break;
                }
                case BNE: // Branch if not equal
                {
                    std::cout << "BNE " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 != temp_rs2)
                    {
                        std::cout << "Not equal to" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BLT: // Branch if Less than
                {
                    std::cout << "BLT " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 < temp_rs2)
                    {
                        std::cout << "Less than" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BGE: // Branch if greater than or equal
                {
                    std::cout << "BGE " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 >= temp_rs2)
                    {
                        std::cout << "Greater than or equal" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BLTU: // Branch if Less Than (unsigned)
                {
                    std::cout << "BLTU " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if ((unsigned)temp_rs1 < (unsigned)temp_rs2)
                    {
                        std::cout << "Less than" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BGEU: // Branch if Greater or Equal to (unsigned)
                {
                    std::cout << "BGEU " << registerNames[rs1] << ", " << registerNames[rs2] << ", " << Branch_Immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if ((unsigned)temp_rs1 >= (unsigned)temp_rs2)
                    {
                        std::cout << "greater or equal to" << std::endl
                                  << std::endl;
                        toJump = Branch_Immed;
                        PC += Branch_Immed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "Skip branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                default:
                {
                    // std::cout<<"Invalid branch instruction" << std::endl;
                    break;
                }
                }
                break;
            }
            case LUI: // Load Upper Immediate
            {
                std::cout << "LUI " << registerNames[rd] << ", " << UI_Immed << std::endl;
                registers[rd].value = UI_Immed << 12;
                registers[rd].used = true;
                std::cout << "Result: " << registers[rd].value << std::endl
                          << std::endl;
                break;
            }         // End fof Branch Format
            case JAL: // Jump and Link
            {
                std::cout << "JAL " << registerNames[rd] << ", " << UJ_Immed << std::endl;
                // cannot change value of x0
                if (rd != 0)
                {
                    registers[rd].value = PC + 4;
                    registers[rd].used = true;
                }
                toJump = UJ_Immed;
                jumped = true;
                PC += UJ_Immed;
                std::cout << "Reg/RA: " << registers[rd].value << " | PC: " << PC << std::endl
                          << std::endl;
                break;
            }
            case JALR: // Jump and Link with register
            {
                std::cout << "JALR " << registerNames[rd] << ", " << immed << "(" << registerNames[rs1] << ")" << std::endl;
                toJump = immed + rs1;
                PC = immed + rs1 + PC;
                jumped = true;
                break;
            }
            case AUIPC: // Add Upper Imm to PC
            {
                std::cout << "AUIPC " << registerNames[rd] << ", " << UI_Immed << std::endl;
                registers[rd].value = PC + (UI_Immed << 12);
                registers[rd].used = true;
                std::cout << "Result: " << registers[rd].value << std::endl
                          << std::endl;
                break;
            }
            default:
            {
                // std::cout<<"not valid instruction"<<std::endl;
                break;
            }
            }
            //============//
            // MEM ACCESS //
            //============//
            int temp_address = 0;
            switch (opcode)
            {
            case Load_format: // Load word
            {
                std::cout << "LW " << registerNames[rd] << ", " << immed << "(" << registerNames[rs1] << ")" << std::endl;
                dmem_wen = false;
                printMem = true;
                int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                int address = temp_rs1 + immed;
                temp_address = address;
                std::cout << "Address: " << address << std::endl;
                break;
            }                  // End of Load Word
            case Store_format: // Store word
            {
                dmem_wen = true;
                printMem = true;
                std::cout << "STORE " << registerNames[rs2] << ", " << StoreImmed << "(" << registerNames[rs1] << ")" << std::endl;
                // store word
                int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                int address = temp_rs1 + StoreImmed;
                bool found = false;
                for (int i = 0; i < 128; i++)
                {
                    if (mem[i].address == address)
                    {
                        mem[i].data = temp_rs2;
                        found = true;
                    }
                }
                std::cout << "Address: " << temp_rs1 + StoreImmed << std::endl;
                std::cout << "Result: " << temp_rs2 << std::endl
                          << std::endl;
                break;
            } // End of Store Word
            default:
            {
                // std::cout << "Ivalid in mem access" << std::endl;
                break;
            }
            } // End of MEM ACESS STAGE
            //==================//
            // WRITE BACK STAGE //
            //==================//
            // if store write it back
            if (!dmem_wen && opcode == Load_format)
            { // checking if loading write back data result to register
                for (int i = 0; i < 32; i++)
                {
                    if (mem[i].address == temp_address)
                    {
                        registers[rd].value = mem[i].data;
                        registers[rd].used = true;
                    }
                }
                std::cout << "Result: " << registers[rd].value << std::endl
                          << std::endl; // printing for load
            }

            if (!branched && !jumped)
            {            // no branch
                PC += 1; // add +4 to PC
            }
            else if (jumped)
            {
                jumped = false;
            }
            else
            {
                branched = false; // reseting bool
                jumped = false;
            }
            if (toJump == 0)
            {
                count++;
            }
            else if (toJump > 0)
            {
                count = count + (toJump / 4);
                toJump = 0;
            }
            else if (toJump < 0)
            {
                count = count - ((-toJump) / 4);
                toJump = 0;
            }
        }
        if (count == index) // reaches at the EOF
            break;
    } // End of While loop
    inputFile.close();

    std::cout << "   ____________________________________" << std::endl;
    std::cout << "  |             REGISTERS              |" << std::endl;
    std::cout << "  |____________________________________|" << std::endl;
    std::cout << "  |  Name   |  Number  |     Value     |" << std::endl;
    std::cout << "  |_________|__________|_______________|" << std::endl;
    for (int i = 0; i < 32; i++)
    {
        std::cout << "  | " << std::setw(7) << registerNames[i] << " | " << std::setw(8) << i << " | " << std::setw(13) << registers[i].value << " |" << std::endl;
    }
    std::cout << "  |_________|__________|_______________|" << std::endl
              << std::endl;

    // Printing mem
    if (printMem)
    {
        std::cout << "          ____________________________________________" << std::endl;
        std::cout << "         |                   MEMORY                   |" << std::endl;
        std::cout << "  _______|____________________________________________|_____" << std::endl;
        for (int i = 0; i < 128 && mem[i].data != 0; i++)
        {
            std::cout << " |Number: " << std::setw(4) << i << " | Address: " << std::setw(12) << mem[i].address << " | Data: " << std::setw(12) << mem[i].data << " |" << std::endl;
        }
        std::cout << " |_____________|_______________________|____________________|" << std::endl
                  << std::endl;

        std::cout << "~~~~ END OF PROGRAM ~~~~" << std::endl
                  << std::endl;
    }
    return 0;
}
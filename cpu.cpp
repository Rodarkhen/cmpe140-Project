#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstring>
#include <iomanip>
#include "instructions.h"

// Function declarations
void printOptions();
int binaryToDecimal(long n);
long twosComplement(std::string str);

int main(int argc, char **argv)
{
    instructionMemory in_instructions[50]; //  the 32 bits instruction place in an array
    reg registers[32];                     // 32 registers
    dmem mem[128];                         // 128 mem

    long initialAddr = 268500992;
    for (int i = 0; i < 128; i++)
    {
        mem[i].address = initialAddr;
        mem[i].data = 0;
        initialAddr += 4;
    }

    int PC = 0;      // keeping track of PC
    int jumping = 0; // for jumping instructions

    std::string filename = argv[1];
    // Opening file
    std::ifstream inputFile;
    inputFile.open(filename);
    if (!inputFile.is_open())
    {
        std::cout << "Unable to open file" << std::endl;
        exit(1);
    }

    // getline vars
    int lineCount = 0;
    std::string line;

    int idx = 0; // index of mem

    // if second file param entered->dmem file
    if (argc == 3) //./test line.dat dmem.dat
    {
        std::string subInstruction = "";
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
                long data = 0;
                lineCount = 0;

                data = (subInstruction[0] == '1')              // d is determined by index 0 the in order to be place as data
                           ? twosComplement(subInstruction)    // determines if it is negative or positive
                           : stol(subInstruction, nullptr, 2); // converts to decimals

                mem[idx].data = data;
                subInstruction = "";
                idx++;
            }
        }
    }

    int index = 0;         // instruction number
    bool dmem_wen = false; // Write enable for data memory (1 for Store, 0 for Load)

    // Variables for user inputs
    std::string currentInput = "";
    std::string prevInput = "";
    bool r = false;
    bool brk = false;
    bool printMem = false;
    // Variables for decoding instructions
    long immed = 0;
    long rs1 = 0;
    long rd = 0;
    long opcode = 0;
    long func3 = 0;
    long R_immed = 0;
    long rs2 = 0;
    long StoreImmed = 0;
    long UImmed = 0;
    long bImmed = 0;
    long UJImmed = 0;
    bool branched = false;
    bool jumped = false;

    // Reads a file with instructions (32'b instruction per line)
    while (getline(inputFile, line))
    {
        in_instructions[index].instruction = line;
        index++;
    }

    int count = 0; // index for going through imem

    // Shows available options fo the user
    printOptions();

    while (true)
    {
        //=============//
        // FETCH STAGE //
        //=============//
        if ((jumping == 0))
        {
            while ((prevInput.compare("s") || prevInput.compare("pc")) && !brk && !r)
            /*prevInput is not equal to "s" || prevInput is not equal to "pc" && brk is false && r is false.*/
            {
                std::cout << "-------------------------------" << std::endl;
                std::cout << "\e[1mEnter command:\e[0m "; // bolded
                std::cin >> currentInput;
                std::string hex = currentInput.substr(0, 2);
                std::string reg = currentInput.substr(0, 1);

                if (currentInput.compare("r") == 0)
                { // runs everything
                    prevInput = "r";
                    brk = true;
                    r = true;
                }
                else if (currentInput.compare("s") == 0)
                { // runs next instruction + then stop
                    prevInput = "s";
                    brk = true;
                }
                else if (reg.compare("x") == 0)
                { // prints reg value based on reg num
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
                        std::cout << "contents for register " << n << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << registers[n].value << std::dec << std::endl;
                        brk = false;
                    }
                }
                else if (hex.compare("0x") == 0)
                { // prints mem data based on mem addr
                    prevInput = "hex";
                    std::string addr = currentInput.substr(2, 8);
                    int a = stol(addr, nullptr, 16);
                    for (int i = 0; i < 128; i++)
                    {
                        if (mem[i].address == a)
                        {
                            std::cout << "contents from address 0x" << addr << ": 0x" << std::hex << mem[i].data << std::dec << std::endl;
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
                {
                    brk = false;
                }
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
            rs1 = binaryToDecimal(stol(in_instructions[count].rs1, nullptr, 10));
            // func3
            in_instructions[count].func3 = in_instructions[count].instruction.substr(17, 3);
            func3 = stoi(in_instructions[count].func3, nullptr, 10);
            // rd
            in_instructions[count].rd = in_instructions[count].instruction.substr(20, 5);
            rd = binaryToDecimal(stol(in_instructions[count].rd, nullptr, 10));
            // opcode
            in_instructions[count].opcode = in_instructions[count].instruction.substr(25, 7);
            opcode = stoi(in_instructions[count].opcode, nullptr, 10);

            // R-type format //
            // R_immed
            in_instructions[count].R_immed = in_instructions[count].instruction.substr(0, 7);
            R_immed = binaryToDecimal(stol(in_instructions[count].R_immed, nullptr, 10));
            // rs2
            in_instructions[count].rs2 = in_instructions[count].instruction.substr(7, 5);
            rs2 = binaryToDecimal(stol(in_instructions[count].rs2, nullptr, 10));
            // rs1, func3, rd, opcode are similar to the I-Type format

            // Store format
            in_instructions[count].StoreImmed = in_instructions[count].R_immed + in_instructions[count].rd;
            StoreImmed = binaryToDecimal(stol(in_instructions[count].StoreImmed, nullptr, 10));

            // U format
            in_instructions[count].UImmed = in_instructions[count].instruction.substr(0, 20);
            UImmed = (in_instructions[count].UImmed[0] == '1') ? twosComplement(in_instructions[count].UImmed) : stol(in_instructions[count].UImmed, nullptr, 2);

            // UJ format
            in_instructions[count].UJImmed = in_instructions[count].instruction.substr(0, 1) + in_instructions[count].instruction.substr(12, 8) + in_instructions[count].instruction.substr(11, 1) + in_instructions[count].instruction.substr(1, 10) + "0";
            UJImmed = (in_instructions[count].UJImmed[0] == '1') ? twosComplement(in_instructions[count].UJImmed) : stol(in_instructions[count].UJImmed, nullptr, 2);

            // branch format --lowest bit offest is always 0
            in_instructions[count].bImmed = in_instructions[count].R_immed.substr(0, 1) + in_instructions[count].rd.substr(4, 1) + in_instructions[count].R_immed.substr(1, 6) + in_instructions[count].rd.substr(0, 4) + '0';
            bImmed = binaryToDecimal(stol(in_instructions[count].bImmed, nullptr, 10));
            bImmed = (in_instructions[count].bImmed[0] == '1') ? twosComplement(in_instructions[count].bImmed) : stol(in_instructions[count].bImmed, nullptr, 2);
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
                    std::cout << "ADDI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 + immed;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case STLI: // Pick the value of rs1 or registers[rs1].value
                {
                    std::cout << "STLI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    if (temp_rs1 < immed)
                    {
                        registers[rd].value = 1;
                    }
                    else
                    {
                        registers[rd].value = 0;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLTIU: // Set less than unsigned
                {           // set the value if temp_rs1 is less than imm value
                    std::cout << "SLTIU x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    if ((unsigned int)temp_rs1 < immed)
                    {
                        registers[rd].value = 1;
                    }
                    else
                    {
                        registers[rd].value = 0;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case XORI: // Exclusive or
                {
                    std::cout << "XORI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    registers[rd].value = temp_rs1 ^ immed;                          //
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case ORI: // Bitwise Or with immediate
                {
                    std::cout << "ORI x" << rd << ", x" << rs1 << ", " << immed << std::endl; // check if temp_rs1 is less that imm value, then set registers[rd] to 1
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;          // d_write[rs1].value : rs1
                    registers[rd].value = temp_rs1 ^ immed;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case ANDI: // Bitwise And with immediate
                {
                    std::cout << "ANDI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 & immed;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLLI: // Shift left logical with immediate
                {
                    std::cout << "SLLI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    registers[rd].value = temp_rs1 << immed;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
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
                        std::cout << "SRAI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                        registers[rd].value = temp_rs1 >> shift_amount;
                    }
                    else
                    {
                        std::cout << "SRLI x" << rd << ", x" << rs1 << ", " << immed << std::endl;
                        registers[rd].value = (unsigned)temp_rs1 >> shift_amount;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                default:
                {
                    std::cout << "not valid I-TYPE instruction" << std::endl;
                    break;
                }
                }
                break;
            }
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
                        std::cout << "SUB x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                        registers[rd].value = temp_rs1 - temp_rs2;
                    }
                    else
                    {
                        std::cout << "ADD x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                        registers[rd].value = temp_rs1 + temp_rs2;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLL: // Shift Left Logical
                {
                    std::cout << "SLL x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 << temp_rs2;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLT: //
                {
                    std::cout << "SLT x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;

                    if (temp_rs1 < temp_rs2)
                    {
                        registers[rd].value = 1;
                    }
                    else
                    {
                        registers[rd].value = 0;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case SLTU: // Set Less Than Imme (unnsigne)
                {
                    std::cout << "SLTU x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 < temp_rs2)
                    {
                        registers[rd].value = 1;
                    }
                    else
                    {
                        registers[rd].value = 0;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case XOR: // Exclusion Or
                {
                    std::cout << "XOR x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 ^ temp_rs2;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
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
                        std::cout << "SRA x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                        registers[rd].value = temp_rs1 >> temp_rs2;
                    }
                    else
                    {
                        std::cout << "SRL x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                        registers[rd].value = (unsigned int)temp_rs1 >> temp_rs2;
                    }
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case OR: // Bitwise Or
                {
                    std::cout << "OR x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 | temp_rs2;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                case AND: // Bitwise And
                {
                    std::cout << "AND x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    registers[rd].value = temp_rs1 & temp_rs2;
                    registers[rd].used = true;
                    std::cout << "result: " << registers[rd].value << std::endl
                              << std::endl;
                    break;
                }
                default:
                {
                    // std::cout<<"Invalid R-TYPE instruction" << std::endl;
                    break;
                }
                }
                break;
            }
            case LUI: // Load Upper Immediate
            {
                std::cout << "LUI x" << rd << ", " << UImmed << std::endl;
                registers[rd].value = UImmed << 12;
                registers[rd].used = true;
                std::cout << "result: " << registers[rd].value << std::endl
                          << std::endl;
                break;
            }
            case AUIPC: // Add Upper Imm to PC
            {
                std::cout << "AUIPC x" << rd << ", " << UImmed << std::endl;
                registers[rd].value = PC + (UImmed << 12);
                registers[rd].used = true;
                std::cout << "result: " << registers[rd].value << std::endl
                          << std::endl;
                break;
            }
            case JAL: // Jump and Link
            {
                std::cout << "JAL x" << rd << ", " << UJImmed << std::endl;
                // cannot change value of x0
                if (rd != 0)
                {
                    registers[rd].value = PC + 4;
                    registers[rd].used = true;
                }
                jumping = UJImmed;
                jumped = true;
                PC += UJImmed;
                std::cout << "reg/return address: " << registers[rd].value << " PC: " << PC << std::endl
                          << std::endl;
                break;
            }
            case JALR: // Jump and Link with register
            {
                std::cout << "JALR x" << rd << ", " << immed << "(x" << rs1 << ")" << std::endl;
                jumping = immed + rs1;
                PC = immed + rs1 + PC;
                jumped = true;
                break;
            }
            case Branch_format:
            {
                switch (func3)
                {
                case BEQ: // Branch if equal
                {
                    std::cout << "BEQ x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 == temp_rs2)
                    {
                        std::cout << "equal to" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }

                    break;
                }
                case BNE: // Branch if not equal
                {
                    std::cout << "BNE x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 != temp_rs2)
                    {
                        std::cout << "not equal to" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BLT: // Branch if Less than
                {
                    std::cout << "BLT x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 < temp_rs2)
                    {
                        std::cout << "less than" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BGE: // Branch if greater than or equal
                {
                    std::cout << "BGE x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if (temp_rs1 >= temp_rs2)
                    {
                        std::cout << "greater than or equal" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BLTU: // Branch if Less Than (unsigned)
                {
                    std::cout << "BLTU x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if ((unsigned)temp_rs1 < (unsigned)temp_rs2)
                    {
                        std::cout << "less than" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                case BGEU: // Branch if Greater or Equal to (unsigned)
                {
                    std::cout << "BGEU x" << rs1 << ", x" << rs2 << ", " << bImmed << std::endl;
                    int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                    int temp_rs2 = registers[rs2].used ? registers[rs2].value : rs2;
                    if ((unsigned)temp_rs1 >= (unsigned)temp_rs2)
                    {
                        std::cout << "greater or equal to" << std::endl
                                  << std::endl;
                        jumping = bImmed;
                        PC += bImmed;
                        branched = true;
                    }
                    else
                    {
                        std::cout << "not branching" << std::endl
                                  << std::endl;
                    }
                    break;
                }
                default:
                {
                    // std::cout<<"not valid branch instruction"<<std::endl;
                    break;
                }
                }
                break;
            }
            default:
            {
                // std::cout<<"not valid instruction"<<std::endl;
                break;
            }
            }
            //====================================================//
            // MEM ACCESS
            int temp_address = 0;
            switch (opcode)
            {
            case Load_format: // Load word
            {
                std::cout << "LW x" << rd << ", " << immed << "(x" << rs1 << ")" << std::endl;
                dmem_wen = false;
                printMem = true;
                // load word
                int temp_rs1 = registers[rs1].used ? registers[rs1].value : rs1;
                int address = temp_rs1 + immed;
                temp_address = address;
                std::cout << "address: " << address << std::endl;
                break;
            }
            case Store_format: // Store word
            {
                dmem_wen = true;
                printMem = true;
                std::cout << "STORE x" << rs2 << ", " << StoreImmed << "(x" << rs1 << ")" << std::endl;
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
                std::cout << "addr: " << temp_rs1 + StoreImmed << std::endl;
                std::cout << "result: " << temp_rs2 << std::endl
                          << std::endl;
                break;
            }
            default:
            {
                // std::cout << "not valid in mem access" << std::endl;
                break;
            }
            }
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
                std::cout << "result: " << registers[rd].value << std::endl
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
            if (jumping == 0)
            {
                count++;
            }
            else if (jumping > 0)
            {
                count = count + (jumping / 4);
                jumping = 0;
            }
            else if (jumping < 0)
            {
                count = count - ((-jumping) / 4);
                jumping = 0;
            }
        }
        if (count == index) // reaches at the EOF
            break;
    } // End of While loop

    inputFile.close();
    std::cout << "   _____________________________" << std::endl;
    std::cout << "  |         REGISTERS           |" << std::endl;
    std::cout << "  |_____________________________|" << std::endl;
    std::cout << "  |    Number   |     Value     |" << std::endl;
    std::cout << "  |_____________|_______________|" << std::endl;
    for (int i = 0; i < 32; i++)
    {
        std::cout << "  | " << std::setw(11) << i << " |  " << std::setw(12) << registers[i].value << " |" << std::endl;
    }
    std::cout << "  |_____________|_______________|" << std::endl;

    // printing mem
    if (printMem)
    {
        std::cout << "     ____________________________________________" << std::endl;
        std::cout << "    |                   MEMORY                   |" << std::endl;
        std::cout << "  __|____________________________________________|__" << std::endl;
        for (int i = 0; i < 128 && mem[i].data != 0; i++)
        {
            std::cout << " |Number: " << std::setw(4) << i << " | Address: " << std::setw(12) << mem[i].address << " | Data: " << std::setw(4) << mem[i].data << " |" << std::endl;
        }
        std::cout << " |_____________|_______________________|____________|" << std::endl;
    }
    return 0;
}

void printOptions()
{
    std::cout << "Options:" << std::endl
              << "    - 'r' runs the entire program in one go till it hits a breakpoint or exits." << std::endl
              << "    - 's' runs the next instruction and then stops and waits for next command." << std::endl
              << "    - 'x0' to 'x31' return the contents of the register from the register file (x0 must always stay 0)." << std::endl
              << "    - '0x12345678' returns the contents from the address 0x12345678 in the data memory. " << std::endl
              << "       This should work for all 32 bit addresses, the value shown above is an example." << std::endl
              << "    - 'pc' returns the value of the PC" << std::endl;
}

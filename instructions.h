#include <string>
#include <iostream>

// Instruction OPCODE Format
#define I_type_format 10011   // 0b0010011
#define R_type_format 110011  // 0b0110011
#define Load_format 11        // 0b0000011
#define Store_format 100011   // 0b0100011
#define Branch_format 1100011 // 0b1100011
#define JAL 1101111           // 0b1101111
#define JALR 1100111          // 0b1100111
#define LUI 110111            // 0b0110111
#define AUIPC 10111           // 0b0010111

// Instruction Type --Func3 dependent
//----------------------------------------
// For I-type instructions
#define ADDI 0       // 0b000   |   0x0
#define SLLI 1       // 0b001   |   0x1
#define STLI 10      // 0b010   |   0x2
#define SLTIU 11     // 0b011   |   0x3
#define XORI 100     // 0b100   |   0x4
#define SRLISRAI 101 // 0b101   |   0x5
#define ORI 110      // 0b110   |   0x6
#define ANDI 111     // 0b111   |   0x7
// For R-type instructions
#define ADDSUB 0   // 0b000     |   0x0
#define SLL 1      // 0b001     |   0x1
#define SLT 10     // 0b010     |   0x2
#define SLTU 11    // 0b011     |   0x3
#define XOR 100    // 0b100     |   0x4
#define SRLSRA 101 // 0b101     |   0x5
#define OR 110     // 0b110     |   0x6
#define AND 111    // 0b111     |   0x7
// For Jump and Branch instructions
#define BEQ 0    // 0b000       |   0x0
#define BNE 1    // 0b001       |   0x1
#define BLT 100  // 0b100       |   0x4
#define BGE 101  // 0b101       |   0x5
#define BLTU 110 // 0b110       |   0x6
#define BGEU 111 // 0b111       |   0x7
//---------------------------------------

// Created a class to hold all the pieces of a of each format of the instructions
class instructionMemory
{
public:
    std::string instruction;  // 32'b instruction
    std::string opcode;       // 7'b [6:0]
    std::string rd;           // 5'b [11:7]
    std::string func3;        // 3'b [14:12]
    std::string rs1;          // 5'b [19:15]
    std::string immed;        // 12'b [31:20]
    std::string R_immed;      // 7'b [31:25] func7 - (First seven bits of immed)
    std::string rs2;          // 5'b [24:20]
    std::string StoreImmed;   // 7'b + 5'b = 12'b [31:25][11:7] (First 7'b of immed and 5 bits from rd)
    std::string UI_Immed;     // 21 bit immed for lui + auipc
    std::string Branch_Immed; // [31:12] branch immed
    std::string UJ_Immed;     // jal instruction immed

    //  Constructor
    instructionMemory()
    {
        instruction = "";
        opcode = "";
        rd = "";
        func3 = "";
        rs1 = "";
        immed = "";
        R_immed = "";
        rs2 = "";
        StoreImmed = "";
        UI_Immed = "";
        Branch_Immed = "";
        UJ_Immed = "";
    }
};

// Keep track of reg values
class Reg
{
public:
    int32_t value;
    bool used;
    // Constructor
    Reg() : value(0), used(false){};
};

class Dmem
{
public:
    int32_t address;
    int32_t data;
    // Constructor
    Dmem() : address(0), data(0){};
};

// Register names
std::string registerNames[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
                               "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3",
                               "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4",
                               "t5", "t6"};

void printOptions()
{
    std::cout << "============================================================" << std::endl;
    std::cout << "Options:" << std::endl
              << "    - 'r' runs the entire program in one go till it hits a breakpoint or exits." << std::endl
              << "    - 's' runs the next instruction and then stops and waits for next command." << std::endl
              << "    - 'x0' to 'x31' return the contents of the register from the register file (x0 must always stay 0)." << std::endl
              << "    - '0x12345678' returns the contents from the address 0x12345678 in the data memory. " << std::endl
              << "       This should work for all 32 bit addresses, the value shown above is an example." << std::endl
              << "    - 'pc' returns the value of the PC" << std::endl;
    std::cout << "============================================================" << std::endl;
}
// Converts signed binary to decimal
int binToDec(int32_t n)
{
    int32_t temp = n;
    int32_t dec = 0;
    int32_t base = 1;

    while (temp)
    {
        int32_t last = temp % 10;
        temp = temp / 10;
        dec += last * base;
        base *= 2;
    }
    dec = (dec + 128) % 256 - 128;
    return dec;
}

// Finds two's complement of binary input
int32_t twosComplement(std::string str)
{
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == '1')
            str[i] = '0';
        else if (str[i] == '0')
            str[i] = '1';
    }
    int32_t temp = 0;
    temp = stol(str, nullptr, 2);
    temp *= -1;
    temp -= 1;
    return temp;
}
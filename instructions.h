#include <string>

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
    std::string instruction;
    std::string opcode;
    std::string rd;
    std::string func3;
    std::string rs1;
    std::string immed;
    std::string R_immed; // first seven bits of immed
    std::string rs2;
    std::string StoreImmed; // first 6 bits of immed and 5 bits from rd
    std::string UImmed;     // 21 bit immed for lui + auipc
    std::string bImmed;     // branch immed
    std::string UJImmed;    // jal instruction immed

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
        UImmed = "";
        bImmed = "";
        UJImmed = "";
    }
};

// keep track of reg values
class reg
{
public:
    long value;
    bool used;
    // Constructor
    reg() : value(0), used(false){};
};

class dmem
{
public:
    long address;
    long data;
    // Constructor
    dmem() : address(0), data(0){};
};
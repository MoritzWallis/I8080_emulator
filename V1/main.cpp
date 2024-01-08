#include <iostream>
#include <cstring>


using std::cout;
using std::endl;
using std::cin;
class I8080;
class InstructionSet;


typedef unsigned char byte;
typedef unsigned short word;


union BusLatch{
    byte low;
    word full;

    BusLatch(){
        full = 0;
    }

    void set(word value){
        full = value;
    }

    void set(byte value){
        low = value;
    }

    word getFull() const {
        return full;
    }

    byte getLow() const {
        return low;
    }
};

class Memory {
public:
    byte *mem;
    BusLatch* memLatch;
    BusLatch* addressLatch;
    BusLatch* dataLatch;

    Memory(BusLatch* addressLatch, BusLatch* dataLatch, int size = 65536){
        mem = new byte[size];
        this->addressLatch = addressLatch;
        this->dataLatch = dataLatch;
    }
    ~Memory(){
        delete[] mem;
    }
    void read(){
        dataLatch->set(mem[addressLatch->getFull()]);
    }
    void write(){
        mem[addressLatch->getFull()] = dataLatch->getLow();
    }
};


#define number_of_bytes_for_registers 15
// Why is this Big Endian when the i8080 isn't? Hmm...
class RegisterArray{
public:
    /*
        000 for Register B
        001 for register C
        010 for register 0
        011 for register E
        100 for register H
        101 for register L
        110 for memory ref. M --> default HL
        111 for register A
     */
    byte *registers;
    char registerNames[number_of_bytes_for_registers][4] = {"B", "C", "D", "E", "H", "L", "W", "Z", "F", "A", "SP", "", "PC", "PSW", "IR"};


    RegisterArray(){
        registers = new byte[number_of_bytes_for_registers];
        memset(registers, 0, number_of_bytes_for_registers);
    }
    ~RegisterArray(){
        delete[] registers;
    }

    // enum Register {B, C, D, E, H, L, W, Z, F, A, SP, PC=12, PSW, IR}; // F = "condition bits"
    word decodeAndReadRegister(byte coded_register){
        switch(coded_register){
            case 0b000:
                return registers[0]; // B
            case 0b001:
                return registers[1]; // C
            case 0b010:
                return registers[2]; // D
            case 0b011:
                return registers[3]; // E
            case 0b100:
                return registers[4]; // H
            case 0b101:
                return registers[5]; // L
            case 0b110:
                return registers[6]; // Memory reference, HL (pointer)
            case 0b111:
                return registers[9]; // ACCUMULATOR
            default:
                throw "Invalid register code";
        }
    }

    /*
     * Register pair 'RP' fields:
    00=BC   (B:C as 16 bit register)
    01=DE   (D:E as 16 bit register)
    10=HL   (H:L as 16 bit register)
    11=SP   (Stack pointer, refers to PSW (FLAGS:A) for PUSH/POP)
     */
    word decodeAndReadRegisterPair(byte pair){
        switch(pair){
            case 0b00:
                return (registers[1] << 8) | registers[2]; // BC
            case 0b01:
                return (registers[3] << 8) | registers[4]; // DE
            case 0b10:
                return (registers[5] << 8) | registers[6]; // HL
            case 0b11:
                return (registers[11] << 8) | registers[10]; // SP
            default:
                throw "Invalid register pair code";
        }
    }

    word getRegister(byte reg){
        if(reg < 0 || reg > 13){
            throw "Addressing invalid register"; // TODO: better error handling
        }

        switch(reg){
            case 13:
                reg -= 5;
            case 10:
            case 12:
                return (registers[reg + 1] << 8) | registers[reg];
            default:
                return registers[reg];
        }
    }

    void setRegister(byte reg, word value){
        /*
        if(reg < 0 || reg > 15){
            throw "Addressing invalid register"; // TODO: better error handling
        }
        */

        switch(reg){
            case 13:
                reg -= 5;
            case 10:
            case 12:
                registers[reg + 1] = (value >> 8) & 0xFF;
                registers[reg] = value & 0xFF;
                break;
            default:
                registers[reg] = value & 0xFF;
        }
    }

    void printRegisters() {
        for(int i = 0; i < 14; i++){
            if(i == 11){
                continue;
            }
            cout << registerNames[i] << ": " << getRegister(i) << endl;
        }
    }
};


class InstructionSet {
public:
    I8080* cpu;
    typedef void (InstructionSet::*Instruction)(byte operand1, byte operand2);
    Instruction all_instructions[256];
    int number_of_operands[256];
    Instruction current_instruction;

    InstructionSet(I8080* cpu){
        this->cpu = cpu;
        //-init_instruction_array();
    }


    /*  LXI
     *
     */
    void decode_and_execute(const unsigned char opcode){
        switch(opcode){
            case 0x0: // 0x00 	NOP	1
                break;
            case 0x1: // 0x01 	LXI B,D16	3		B <- byte 3, C <- byte 2

                break;
            case 0x2: // 0x02 	STAX B	1		(BC) <- A
                break;
            case 0x3: // 0x03 	INX B	1		BC <- BC+1
                break;
            case 0x4: // 0x04 	INR B	1	Z, S, P, AC	B <- B+1
                break;
            case 0x5: // 0x05 	DCR B	1	Z, S, P, AC	B <- B-1
                break;
            case 0x6: // 0x06 	MVI B, D8	2		B <- byte 2
                break;
            case 0x7: // 0x07 	RLC	1	CY	A = A << 1; bit 0 = prev bit 7; CY = prev bit 7
                break;
            case 0x8: // 0x08 	-
                break;
            case 0x9: // 0x09 	DAD B	1	CY	HL = HL + BC
                break;
            case 0xa: // 0x0a 	LDAX B	1		A <- (BC)
                break;
            case 0xb: // 0x0b 	DCX B	1		BC = BC-1
                break;
            case 0xc: // 0x0c 	INR C	1	Z, S, P, AC	C <- C+1
                break;
            case 0xd: // 0x0d 	DCR C	1	Z, S, P, AC	C <-C-1
                break;
            case 0xe: // 0x0e 	MVI C,D8	2		C <- byte 2
                break;
            case 0xf: // 0x0f 	RRC	1	CY	A = A >> 1; bit 7 = prev bit 0; CY = prev bit 0
                break;
            case 0x10: // 0x10 	-
                break;
            case 0x11: // 0x11 	LXI D,D16	3		D <- byte 3, E <- byte 2
                break;
            case 0x12: // 0x12 	STAX D	1		(DE) <- A
                break;
            case 0x13: // 0x13 	INX D	1		DE <- DE + 1
                break;
            case 0x14: // 0x14 	INR D	1	Z, S, P, AC	D <- D+1
                break;
            case 0x15: // 0x15 	DCR D	1	Z, S, P, AC	D <- D-1
                break;
            case 0x16: // 0x16 	MVI D, D8	2		D <- byte 2
                break;
            case 0x17: // 0x17 	RAL	1	CY	A = A << 1; bit 0 = prev CY; CY = prev bit 7
                break;
            case 0x18: // 0x18 	-
                break;
            case 0x19: // 0x19 	DAD D	1	CY	HL = HL + DE
                break;
            case 0x1a: // 0x1a 	LDAX D	1		A <- (DE)
                break;
            case 0x1b: // 0x1b 	DCX D	1		DE = DE-1
                break;
            case 0x1c: // 0x1c 	INR E	1	Z, S, P, AC	E <-E+1
                break;
            case 0x1d: // 0x1d 	DCR E	1	Z, S, P, AC	E <- E-1
                break;
            case 0x1e: // 0x1e 	MVI E,D8	2		E <- byte 2
                break;
            case 0x1f: // 0x1f 	RAR	1	CY	A = A >> 1; bit 7 = prev bit 7; CY = prev bit 0
                break;
            case 0x20: // 0x20 	RIM	1		special
                break;
            case 0x21: // 0x21 	LXI H,D16	3		H <- byte 3, L <- byte 2
                break;
            case 0x22: // 0x22 	SHLD adr	3		(adr) <-L; (adr+1)<-H
                break;
            case 0x23: // 0x23 	INX H	1		HL <- HL + 1
                break;
            case 0x24: // 0x24 	INR H	1	Z, S, P, AC	H <- H+1
                break;
            case 0x25: // 0x25 	DCR H	1	Z, S, P, AC	H <- H-1
                break;
            case 0x26: // 0x26 	MVI H,D8	2		L <- byte 2
                break;
            case 0x27: // 0x27 	DAA	1		special
                break;
            case 0x28: // 0x28 	-
                break;
            case 0x29: // 0x29 	DAD H	1	CY	HL = HL + HI
                break;
            case 0x2a: // 0x2a 	LHLD adr	3		L <- (adr); H<-(adr+1)
                break;
            case 0x2b: // 0x2b 	DCX H	1		HL = HL-1
                break;
            case 0x2c: // 0x2c 	INR L	1	Z, S, P, AC	L <- L+1
                break;
            case 0x2d: // 0x2d 	DCR L	1	Z, S, P, AC	L <- L-1
                break;
            case 0x2e: // 0x2e 	MVI L, D8	2		L <- byte 2
                break;
            case 0x2f: // 0x2f 	CMA	1		A <- !A
                break;
            case 0x30: // 0x30 	SIM	1		special
                break;
            case 0x31: // 0x31 	LXI SP, D16	3		SP.hi <- byte 3, SP.lo <- byte 2
                break;
            case 0x32: // 0x32 	STA adr	3		(adr) <- A
                break;
            case 0x33: // 0x33 	INX SP	1		SP = SP + 1
                break;
            case 0x34: // 0x34 	INR M	1	Z, S, P, AC	(HL) <- (HL)+1
                break;
            case 0x35: // 0x35 	DCR M	1	Z, S, P, AC	(HL) <- (HL)-1
                break;
            case 0x36: // 0x36 	MVI M,D8	2		(HL) <- byte 2
                break;
            case 0x37: // 0x37 	STC	1	CY	CY = 1
                break;
            case 0x38: // 0x38 	-
                break;
            case 0x39: // 0x39 	DAD SP	1	CY	HL = HL + SP
                break;
            case 0x3a: // 0x3a 	LDA adr	3		A <- (adr)
                break;
            case 0x3b: // 0x3b 	DCX SP	1		SP = SP-1
                break;
            case 0x3c: // 0x3c 	INR A	1	Z, S, P, AC	A <- A+1
                break;
            case 0x3d: // 0x3d 	DCR A	1	Z, S, P, AC	A <- A-1
                break;
            case 0x3e: // 0x3e 	MVI A,D8	2		A <- byte 2
                break;
            case 0x3f: // 0x3f 	CMC	1	CY	CY=!CY
                break;
            case 0x40: // 0x40 	MOV B,B	1		B <- B
                break;
            case 0x41: // 0x41 	MOV B,C	1		B <- C
                break;
            case 0x42: // 0x42 	MOV B,D	1		B <- D
                break;
            case 0x43: // 0x43 	MOV B,E	1		B <- E
                break;
            case 0x44: // 0x44 	MOV B,H	1		B <- H
                break;
            case 0x45: // 0x45 	MOV B,L	1		B <- L
                break;
            case 0x46: // 0x46 	MOV B,M	1		B <- (HL)
                break;
            case 0x47: // 0x47 	MOV B,A	1		B <- A
                break;
            case 0x48: // 0x48 	MOV C,B	1		C <- B
                break;
            case 0x49: // 0x49 	MOV C,C	1		C <- C
                break;
            case 0x4a: // 0x4a 	MOV C,D	1		C <- D
                break;
            case 0x4b: // 0x4b 	MOV C,E	1		C <- E
                break;
            case 0x4c: // 0x4c 	MOV C,H	1		C <- H
                break;
            case 0x4d: // 0x4d 	MOV C,L	1		C <- L
                break;
            case 0x4e: // 0x4e 	MOV C,M	1		C <- (HL)
                break;
            case 0x4f: // 0x4f 	MOV C,A	1		C <- A
                break;
            case 0x50: // 0x50 	MOV D,B	1		D <- B
                break;
            case 0x51: // 0x51 	MOV D,C	1		D <- C
                break;
            case 0x52: // 0x52 	MOV D,D	1		D <- D
                break;
            case 0x53: // 0x53 	MOV D,E	1		D <- E
                break;
            case 0x54: // 0x54 	MOV D,H	1		D <- H
                break;
            case 0x55: // 0x55 	MOV D,L	1		D <- L
                break;
            case 0x56: // 0x56 	MOV D,M	1		D <- (HL)
                break;
            case 0x57: // 0x57 	MOV D,A	1		D <- A
                break;
            case 0x58: // 0x58 	MOV E,B	1		E <- B
                break;
            case 0x59: // 0x59 	MOV E,C	1		E <- C
                break;
            case 0x5a: // 0x5a 	MOV E,D	1		E <- D
                break;
            case 0x5b: // 0x5b 	MOV E,E	1		E <- E
                break;
            case 0x5c: // 0x5c 	MOV E,H	1		E <- H
                break;
            case 0x5d: // 0x5d 	MOV E,L	1		E <- L
                break;
            case 0x5e: // 0x5e 	MOV E,M	1		E <- (HL)
                break;
            case 0x5f: // 0x5f 	MOV E,A	1		E <- A
                break;
            case 0x60: // 0x60 	MOV H,B	1		H <- B
                break;
            case 0x61: // 0x61 	MOV H,C	1		H <- C
                break;
            case 0x62: // 0x62 	MOV H,D	1		H <- D
                break;
            case 0x63: // 0x63 	MOV H,E	1		H <- E
                break;
            case 0x64: // 0x64 	MOV H,H	1		H <- H
                break;
            case 0x65: // 0x65 	MOV H,L	1		H <- L
                break;
            case 0x66: // 0x66 	MOV H,M	1		H <- (HL)
                break;
            case 0x67: // 0x67 	MOV H,A	1		H <- A
                break;
            case 0x68: // 0x68 	MOV L,B	1		L <- B
                break;
            case 0x69: // 0x69 	MOV L,C	1		L <- C
                break;
            case 0x6a: // 0x6a 	MOV L,D	1		L <- D
                break;
            case 0x6b: // 0x6b 	MOV L,E	1		L <- E
                break;
            case 0x6c: // 0x6c 	MOV L,H	1		L <- H
                break;
            case 0x6d: // 0x6d 	MOV L,L	1		L <- L
                break;
            case 0x6e: // 0x6e 	MOV L,M	1		L <- (HL)
                break;
            case 0x6f: // 0x6f 	MOV L,A	1		L <- A
                break;
            case 0x70: // 0x70 	MOV M,B	1		(HL) <- B
                break;
            case 0x71: // 0x71 	MOV M,C	1		(HL) <- C
                break;
            case 0x72: // 0x72 	MOV M,D	1		(HL) <- D
                break;
            case 0x73: // 0x73 	MOV M,E	1		(HL) <- E
                break;
            case 0x74: // 0x74 	MOV M,H	1		(HL) <- H
                break;
            case 0x75: // 0x75 	MOV M,L	1		(HL) <- L
                break;
            case 0x76: // 0x76 	HLT	1		special
                break;
            case 0x77: // 0x77 	MOV M,A	1		(HL) <- C
                break;
            case 0x78: // 0x78 	MOV A,B	1		A <- B
                break;
            case 0x79: // 0x79 	MOV A,C	1		A <- C
                break;
            case 0x7a: // 0x7a 	MOV A,D	1		A <- D
                break;
            case 0x7b: // 0x7b 	MOV A,E	1		A <- E
                break;
            case 0x7c: // 0x7c 	MOV A,H	1		A <- H
                break;
            case 0x7d: // 0x7d 	MOV A,L	1		A <- L
                break;
            case 0x7e: // 0x7e 	MOV A,M	1		A <- (HL)
                break;
            case 0x7f: // 0x7f 	MOV A,A	1		A <- A
                break;
            case 0x80: // 0x80 	ADD B	1	Z, S, P, CY, AC	A <- A + B
                break;
            case 0x81: // 0x81 	ADD C	1	Z, S, P, CY, AC	A <- A + C
                break;
            case 0x82: // 0x82 	ADD D	1	Z, S, P, CY, AC	A <- A + D
                break;
            case 0x83: // 0x83 	ADD E	1	Z, S, P, CY, AC	A <- A + E
                break;
            case 0x84: // 0x84 	ADD H	1	Z, S, P, CY, AC	A <- A + H
                break;
            case 0x85: // 0x85 	ADD L	1	Z, S, P, CY, AC	A <- A + L
                break;
            case 0x86: // 0x86 	ADD M	1	Z, S, P, CY, AC	A <- A + (HL)
                break;
            case 0x87: // 0x87 	ADD A	1	Z, S, P, CY, AC	A <- A + A
                break;
            case 0x88: // 0x88 	ADC B	1	Z, S, P, CY, AC	A <- A + B + CY
                break;
            case 0x89: // 0x89 	ADC C	1	Z, S, P, CY, AC	A <- A + C + CY
                break;
            case 0x8a: // 0x8a 	ADC D	1	Z, S, P, CY, AC	A <- A + D + CY
                break;
            case 0x8b: // 0x8b 	ADC E	1	Z, S, P, CY, AC	A <- A + E + CY
                break;
            case 0x8c: // 0x8c 	ADC H	1	Z, S, P, CY, AC	A <- A + H + CY
                break;
            case 0x8d: // 0x8d 	ADC L	1	Z, S, P, CY, AC	A <- A + L + CY
                break;
            case 0x8e: // 0x8e 	ADC M	1	Z, S, P, CY, AC	A <- A + (HL) + CY
                break;
            case 0x8f: // 0x8f 	ADC A	1	Z, S, P, CY, AC	A <- A + A + CY
                break;
            case 0x90: // 0x90 	SUB B	1	Z, S, P, CY, AC	A <- A - B
                break;
            case 0x91: // 0x91 	SUB C	1	Z, S, P, CY, AC	A <- A - C
                break;
            case 0x92: // 0x92 	SUB D	1	Z, S, P, CY, AC	A <- A + D
                break;
            case 0x93: // 0x93 	SUB E	1	Z, S, P, CY, AC	A <- A - E
                break;
            case 0x94: // 0x94 	SUB H	1	Z, S, P, CY, AC	A <- A + H
                break;
            case 0x95: // 0x95 	SUB L	1	Z, S, P, CY, AC	A <- A - L
                break;
            case 0x96: // 0x96 	SUB M	1	Z, S, P, CY, AC	A <- A + (HL)
                break;
            case 0x97: // 0x97 	SUB A	1	Z, S, P, CY, AC	A <- A - A
                break;
            case 0x98: // 0x98 	SBB B	1	Z, S, P, CY, AC	A <- A - B - CY
                break;
            case 0x99: // 0x99 	SBB C	1	Z, S, P, CY, AC	A <- A - C - CY
                break;
            case 0x9a: // 0x9a 	SBB D	1	Z, S, P, CY, AC	A <- A - D - CY
                break;
            case 0x9b: // 0x9b 	SBB E	1	Z, S, P, CY, AC	A <- A - E - CY
                break;
            case 0x9c: // 0x9c 	SBB H	1	Z, S, P, CY, AC	A <- A - H - CY
                break;
            case 0x9d: // 0x9d 	SBB L	1	Z, S, P, CY, AC	A <- A - L - CY
                break;
            case 0x9e: // 0x9e 	SBB M	1	Z, S, P, CY, AC	A <- A - (HL) - CY
                break;
            case 0x9f: // 0x9f 	SBB A	1	Z, S, P, CY, AC	A <- A - A - CY
                break;
            case 0xa0: // 0xa0 	ANA B	1	Z, S, P, CY, AC	A <- A & B
                break;
            case 0xa1: // 0xa1 	ANA C	1	Z, S, P, CY, AC	A <- A & C
                break;
            case 0xa2: // 0xa2 	ANA D	1	Z, S, P, CY, AC	A <- A & D
                break;
            case 0xa3: // 0xa3 	ANA E	1	Z, S, P, CY, AC	A <- A & E
                break;
            case 0xa4: // 0xa4 	ANA H	1	Z, S, P, CY, AC	A <- A & H
                break;
            case 0xa5: // 0xa5 	ANA L	1	Z, S, P, CY, AC	A <- A & L
                break;
            case 0xa6: // 0xa6 	ANA M	1	Z, S, P, CY, AC	A <- A & (HL)
                break;
            case 0xa7: // 0xa7 	ANA A	1	Z, S, P, CY, AC	A <- A & A
                break;
            case 0xa8: // 0xa8 	XRA B	1	Z, S, P, CY, AC	A <- A ^ B
                break;
            case 0xa9: // 0xa9 	XRA C	1	Z, S, P, CY, AC	A <- A ^ C
                break;
            case 0xaa: // 0xaa 	XRA D	1	Z, S, P, CY, AC	A <- A ^ D
                break;
            case 0xab: // 0xab 	XRA E	1	Z, S, P, CY, AC	A <- A ^ E
                break;
            case 0xac: // 0xac 	XRA H	1	Z, S, P, CY, AC	A <- A ^ H
                break;
            case 0xad: // 0xad 	XRA L	1	Z, S, P, CY, AC	A <- A ^ L
                break;
            case 0xae: // 0xae 	XRA M	1	Z, S, P, CY, AC	A <- A ^ (HL)
                break;
            case 0xaf: // 0xaf 	XRA A	1	Z, S, P, CY, AC	A <- A ^ A
                break;
            case 0xb0: // 0xb0 	ORA B	1	Z, S, P, CY, AC	A <- A | B
                break;
            case 0xb1: // 0xb1 	ORA C	1	Z, S, P, CY, AC	A <- A | C
                break;
            case 0xb2: // 0xb2 	ORA D	1	Z, S, P, CY, AC	A <- A | D
                break;
            case 0xb3: // 0xb3 	ORA E	1	Z, S, P, CY, AC	A <- A | E
                break;
            case 0xb4: // 0xb4 	ORA H	1	Z, S, P, CY, AC	A <- A | H
                break;
            case 0xb5: // 0xb5 	ORA L	1	Z, S, P, CY, AC	A <- A | L
                break;
            case 0xb6: // 0xb6 	ORA M	1	Z, S, P, CY, AC	A <- A | (HL)
                break;
            case 0xb7: // 0xb7 	ORA A	1	Z, S, P, CY, AC	A <- A | A
                break;
            case 0xb8: // 0xb8 	CMP B	1	Z, S, P, CY, AC	A - B
                break;
            case 0xb9: // 0xb9 	CMP C	1	Z, S, P, CY, AC	A - C
                break;
            case 0xba: // 0xba 	CMP D	1	Z, S, P, CY, AC	A - D
                break;
            case 0xbb: // 0xbb 	CMP E	1	Z, S, P, CY, AC	A - E
                break;
            case 0xbc: // 0xbc 	CMP H	1	Z, S, P, CY, AC	A - H
                break;
            case 0xbd: // 0xbd 	CMP L	1	Z, S, P, CY, AC	A - L
                break;
            case 0xbe: // 0xbe 	CMP M	1	Z, S, P, CY, AC	A - (HL)
                break;
            case 0xbf: // 0xbf 	CMP A	1	Z, S, P, CY, AC	A - A
                break;
            case 0xc0: // 0xc0 	RNZ	1		if NZ, RET
                break;
            case 0xc1: // 0xc1 	POP B	1		C <- (sp); B <- (sp+1); sp <- sp+2
                break;
            case 0xc2: // 0xc2 	JNZ adr	3		if NZ, PC <- adr
                break;
            case 0xc3: // 0xc3 	JMP adr	3		PC <= adr
                break;
            case 0xc4: // 0xc4 	CNZ adr	3		if NZ, CALL adr
                break;
            case 0xc5: // 0xc5 	PUSH B	1		(sp-2)<-C; (sp-1)<-B; sp <- sp - 2
                break;
            case 0xc6: // 0xc6 	ADI D8	2	Z, S, P, CY, AC	A <- A + byte
                break;
            case 0xc7: // 0xc7 	RST 0	1		CALL $0
                break;
            case 0xc8: // 0xc8 	RZ	1		if Z, RET
                break;
            case 0xc9: // 0xc9 	RET	1		PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
                break;
            case 0xca: // 0xca 	JZ adr	3		if Z, PC <- adr
                break;
            case 0xcb: // 0xcb 	-
                break;
            case 0xcc: // 0xcc 	CZ adr	3		if Z, CALL adr
                break;
            case 0xcd: // 0xcd 	CALL adr	3		(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=adr
                break;
            case 0xce: // 0xce 	ACI D8	2	Z, S, P, CY, AC	A <- A + data + CY
                break;
            case 0xcf: // 0xcf 	RST 1	1		CALL $8
                break;
            case 0xd0: // 0xd0 	RNC	1		if NCY, RET
                break;
            case 0xd1: // 0xd1 	POP D	1		E <- (sp); D <- (sp+1); sp <- sp+2
                break;
            case 0xd2: // 0xd2 	JNC adr	3		if NCY, PC<-adr
                break;
            case 0xd3: // 0xd3 	OUT D8	2		special
                break;
            case 0xd4: // 0xd4 	CNC adr	3		if NCY, CALL adr
                break;
            case 0xd5: // 0xd5 	PUSH D	1		(sp-2)<-E; (sp-1)<-D; sp <- sp - 2
                break;
            case 0xd6: // 0xd6 	SUI D8	2	Z, S, P, CY, AC	A <- A - data
                break;
            case 0xd7: // 0xd7 	RST 2	1		CALL $10
                break;
            case 0xd8: // 0xd8 	RC	1		if CY, RET
                break;
            case 0xd9: // 0xd9 	-
                break;
            case 0xda: // 0xda 	JC adr	3		if CY, PC<-adr
                break;
            case 0xdb: // 0xdb 	IN D8	2		special
                break;
            case 0xdc: // 0xdc 	CC adr	3		if CY, CALL adr
                break;
            case 0xdd: // 0xdd 	-
                break;
            case 0xde: // 0xde 	SBI D8	2	Z, S, P, CY, AC	A <- A - data - CY
                break;
            case 0xdf: // 0xdf 	RST 3	1		CALL $18
                break;
            case 0xe0: // 0xe0 	RPO	1		if PO, RET
                break;
            case 0xe1: // 0xe1 	POP H	1		L <- (sp); H <- (sp+1); sp <- sp+2
                break;
            case 0xe2: // 0xe2 	JPO adr	3		if PO, PC <- adr
                break;
            case 0xe3: // 0xe3 	XTHL	1		L <-> (SP); H <-> (SP+1)
                break;
            case 0xe4: // 0xe4 	CPO adr	3		if PO, CALL adr
                break;
            case 0xe5: // 0xe5 	PUSH H	1		(sp-2)<-L; (sp-1)<-H; sp <- sp - 2
                break;
            case 0xe6: // 0xe6 	ANI D8	2	Z, S, P, CY, AC	A <- A & data
                break;
            case 0xe7: // 0xe7 	RST 4	1		CALL $20
                break;
            case 0xe8: // 0xe8 	RPE	1		if PE, RET
                break;
            case 0xe9: // 0xe9 	PCHL	1		PC.hi <- H; PC.lo <- L
                break;
            case 0xea: // 0xea 	JPE adr	3		if PE, PC <- adr
                break;
            case 0xeb: // 0xeb 	XCHG	1		H <-> D; L <-> E
                break;
            case 0xec: // 0xec 	CPE adr	3		if PE, CALL adr
                break;
            case 0xed: // 0xed 	-
                break;
            case 0xee: // 0xee 	XRI D8	2	Z, S, P, CY, AC	A <- A ^ data
                break;
            case 0xef: // 0xef 	RST 5	1		CALL $28
                break;
            case 0xf0: // 0xf0 	RP	1		if P, RET
                break;
            case 0xf1: // 0xf1 	POP PSW	1		flags <- (sp); A <- (sp+1); sp <- sp+2
                break;
            case 0xf2: // 0xf2 	JP adr	3		if P=1 PC <- adr
                break;
            case 0xf3: // 0xf3 	DI	1		special
                break;
            case 0xf4: // 0xf4 	CP adr	3		if P, PC <- adr
                break;
            case 0xf5: // 0xf5 	PUSH PSW	1		(sp-2)<-flags; (sp-1)<-A; sp <- sp - 2
                break;
            case 0xf6: // 0xf6 	ORI D8	2	Z, S, P, CY, AC	A <- A | data
                break;
            case 0xf7: // 0xf7 	RST 6	1		CALL $30
                break;
            case 0xf8: // 0xf8 	RM	1		if M, RET
                break;
            case 0xf9: // 0xf9 	SPHL	1		SP=HL
                break;
            case 0xfa: // 0xfa 	JM adr	3		if M, PC <- adr
                break;
            case 0xfb: // 0xfb 	EI	1		special
                break;
            case 0xfc: // 0xfc 	CM adr	3		if M, CALL adr
                break;
            case 0xfd: // 0xfd 	-
                break;
            case 0xfe: // 0xfe 	CPI D8	2	Z, S, P, CY, AC	A - data
                break;
            case 0xff: // 0xff 	RST 7	1		CALL $38
                break;


        }
    }

    Instruction generateInstruction(byte opcode){

        // MOV
        if (opcode > 0x3f && opcode < 0x80 ){
            //-return get_specific_MOV(opcode);
        }

        // ADD
        if(opcode > 0x7f && opcode < 0x88){
            //-return get_specific_ADD(opcode);
        }

        // ADC
        if(opcode > 0x87 && opcode < 0x90){
            //-return get_specific_ADC(opcode);
        }

        // SUB
        if(opcode > 0x8f && opcode < 0x98){
            //-return get_specific_SUB(opcode);
        }

        // SBB
        if(opcode > 0x97 && opcode < 0xa0){
            //-return get_specific_SBB(opcode);
        }

        // ANA
        if(opcode > 0x9f && opcode < 0xa8) {
            //-return get_specific_ANA(opcode);
        }

        // XRA
        if(opcode > 0xa7 && opcode < 0xb0) {
            //-return get_specific_XRA(opcode);
        }

        // ORA
        if(opcode > 0xaf && opcode < 0xb8) {
            //-return get_specific_ORA(opcode);
        }


        // All other instructions
        switch(opcode){
            default:
                return 0;

        }
    }

    int get_number_of_operands(byte opcode){
        return number_of_operands[opcode];
    }

};

class I8080{
public:
    enum Register {B, C, D, E, H, L, W, Z, F, A, SP, PC=12, PSW, IR}; // F = "condition bits"
    RegisterArray* registers;
    Memory *ram;
    BusLatch addressLatch;
    BusLatch dataLatch;
    InstructionSet* instructionSet;

    I8080(){
        registers = new RegisterArray();
        ram = new Memory(&addressLatch, &dataLatch);
        instructionSet = new InstructionSet(this);

        while(true){
            cycle();
        }
    }
    ~I8080(){
        delete registers;
        delete ram;
        delete instructionSet;
    }

    void cycle(){
        fetch();
        decode();
        //-instructionSet->execute();
    }

    // fetch the next instruction and increment PC
    void fetch(){
        addressLatch.set(registers->getRegister(PC));
        ram->read();
        registers->setRegister(IR, dataLatch.getLow());
        registers->setRegister(PC, registers->getRegister(PC) + 1); //TODO: INC PC
    }

    // get Instruction and load potential operands into WZ
    void decode(){
        //-instructionSet->decode_instr(registers->getRegister(IR));
        int n_operands = instructionSet->get_number_of_operands(registers->getRegister(IR));

        if(n_operands == 1){
            addressLatch.set(registers->getRegister(Z));
            ram->read();
            registers->setRegister(IR, dataLatch.getLow());
            registers->setRegister(PC, registers->getRegister(PC) + 1);
        }

        if(n_operands == 2){
            addressLatch.set(registers->getRegister(W));
            ram->read();
            registers->setRegister(IR, dataLatch.getLow());
            registers->setRegister(PC, registers->getRegister(PC) + 1);
        }
    }
};

enum Register {B, C, D, E, H, L, W, Z, F, A, SP, PC=12, PSW, IR}; // F = "condition bits"

int main(){
    //I8080 cpu;
    RegisterArray reg;
    reg.setRegister(C, 0xbb);
    reg.setRegister(B, 0xaa);
    return reg.decodeAndReadRegisterPair(0x00);
}

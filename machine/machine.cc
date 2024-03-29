/*! \file machine.cc
//  \brief Routines to simulate the execution of user programs.
//  DO NOT CHANGE -- part of the machine emulation
//

 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------

*/

#include "machine/machine.h"
#include "drivers/drvConsole.h"
#include "drivers/drvDisk.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "machine/interrupt.h"

/*! Textual names of the exceptions that can be generated by user program
 execution, for debugging purpose.

*/
static char *exceptionNames[] = {
    (char *) "no exception", (char *) "syscall",
    (char *) "page fault",   (char *) "page read only",
    (char *) "bus error",    (char *) "address error",
    (char *) "overflow",     (char *) "illegal instruction"};
#define EXCEPTION_NUMBER 7   //!< Size of exceptionNames, used for sanity checks

//----------------------------------------------------------------------
// CheckEndian
/*! 	Check to be sure that the host really uses the format it says it
//	does, for storing the bytes of an integer.  Stop on error.
*/
//----------------------------------------------------------------------
static void
CheckEndian() {
  // Declare an array of bytes and fills it with 1 2 3 4
  union checkit {
    int8_t charword[4];
    uint32_t intword;
  } check;
  check.charword[0] = 1;
  check.charword[1] = 2;
  check.charword[2] = 3;
  check.charword[3] = 4;
  // Check the byte ordering according to the expected host endianess
  if (check.intword == 0x01020304)
    host_endianess = IS_BIG_ENDIAN;
  else if (check.intword == 0x04030201)
    host_endianess = IS_LITTLE_ENDIAN;
}

//----------------------------------------------------------------------
// Machine::Machine
/*! 	Constructor. Initialize the RISCV machine.
//
//	\param debug if true, drop into the debugger after each user instruction
//		is executed.
*/
//----------------------------------------------------------------------
Machine::Machine(bool debug) {
  int i;

  // Set initial values for the integer and floating point registers
  for (i = 0; i < NUM_INT_REGS; i++)
    int_registers[i] = 0;
  for (i = 0; i < NUM_FP_REGS; i++)
    float_registers[i] = 0;

  // Allocate the main memory of the machine and fills it up with zeroes
  int memSize = g_cfg->NumPhysPages * g_cfg->PageSize;
  mainMemory = new int8_t[memSize];
  for (i = 0; i < memSize; i++)
    mainMemory[i] = 0;

  // Check the endianess of the host machine
  CheckEndian();

  // Sets the debug mode of the machine according to the debug flag
  singleStep = debug;

  // Create the machine sub-components
  this->mmu = new MMU();
  this->interrupt = new Interrupt();
  this->disk = new Disk(DISK_FILE_NAME, DiskRequestDone);
  this->diskSwap = new Disk(DISK_SWAP_NAME, DiskSwapRequestDone);
  this->console = new Console(NULL, NULL, ConsoleGet, ConsolePut);
  if (g_cfg->ACIA)
    this->acia = new ACIA(this);
  else
    this->acia = NULL;

  // Set the machine status
  status = SYSTEM_MODE;
}

//----------------------------------------------------------------------
// Machine::~Machine
//! 	Destructor. De-allocate the data structures used by the
//      simulated RISCV machine.
//----------------------------------------------------------------------
Machine::~Machine() {
  // Deallocate the machine components
  delete this->mmu;
  delete this->interrupt;
  if (this->acia != NULL)
    delete this->acia;
  delete this->disk;
  delete this->diskSwap;
  delete this->console;
}

//----------------------------------------------------------------------
// Machine::RaiseException
/*! 	Transfer control to the Nachos kernel from user mode, because
//	the user program either invoked a system call, or some exception
//	occurred (such as the address translation failed).
//
//	\param which  the cause of the kernel trap (exception number)
//	\param badVaddr the virtual address causing the trap, if appropriate
*/
//----------------------------------------------------------------------
void
Machine::RaiseException(ExceptionType which, int badVAddr) {
  // Sanity check of the exception number
  if (which <= EXCEPTION_NUMBER) {
    DEBUG('m', (char *) "Exception: %s at PC : %x\n", exceptionNames[which],
          this->pc);

    // Call of the exception handler
    badvaddr_reg = badVAddr;
    this->status = SYSTEM_MODE;
    ExceptionHandler(which, badVAddr);   // call the exception handler
    this->status = USER_MODE;            // interrupts are enabled at this point
  } else {
    printf("Nachos internal error: bad exception number %d, exiting\n", which);
    interrupt->Halt(ERROR);
  }
}

//----------------------------------------------------------------------
// Machine::Debugger
/*! 	Primitive debugger.  Note that we can't use
//	gdb to debug user programs, since gdb doesn't run on top of Nachos.
//
//	This method just allow single-stepping, and printing the
//      contents of memory.
*/
//----------------------------------------------------------------------
void
Machine::Debugger() {
  char buf[MAXSTRLEN];
  int num;

  // Print the list of pending interrupts
  interrupt->DumpState();

  // Dump the CPU state (essentially prints the CPU general resgisters)
  DumpState();

  // Print the current clock tick
  printf("At cycle %" PRId64 "\n", g_stats->getTotalTicks());
  fflush(stdout);

  // Gets of command of the basic debugger on stdin
  fgets(buf, MAXSTRLEN, stdin);
  if (sscanf(buf, "%d", &num) == 1) {
    runUntilTime = num;
  } else {
    runUntilTime = 0;
    switch (*buf) {
    case '\n':
      break;
    case 'c':
      singleStep = false;
      break;
    case '?':
      printf("Machine commands:\n");
      printf("    <return>  execute one instruction\n");
      printf("    <number>  run until the given clock cycle number\n");
      printf("    c         run until completion\n");
      printf("    ?         print help message\n");
      break;
    }
  }
}

//----------------------------------------------------------------------
// Machine::DumpState
/*! 	Print the user program's CPU state.  We might print the contents
//	of memory, but that seemed like overkill. Floating point registers
//      are not printed for the same reason.
*/
//----------------------------------------------------------------------
void
Machine::DumpState() {
  int i;
  // Print of the general CPU registers
  printf("Machine registers:\n");
  printf("\tPC:\t0x%" PRIx64 "\n", pc);
  for (i = 0; i < NUM_INT_REGS; i++) {
    switch (i) {
    case STACK_REG:
      printf("\tSP(%d):\t0x%" PRIx64 "\n", i, int_registers[i]);
      break;
    case RETADDR_REG:
      printf("\tRA(%d):\t0x%" PRIx64 "\n", i, int_registers[i]);
      break;
    default:
      printf("\t%d:\t0x%" PRIx64 "\n", i, int_registers[i]);
      break;
    }
  }
  printf("Float registers:\n");
  for (i = 0; i < NUM_FP_REGS; i++) {
    printf("\t%d:\t0x%" PRIx64 "\n", i, float_registers[i]);
  }
}

//----------------------------------------------------------------------
// Machine::ReadRegister/WriteRegister
//!   	Fetch or write the contents of CPU registers
//----------------------------------------------------------------------
// read the contents of an integer register
int64_t
Machine::ReadIntRegister(int num) {
  ASSERT((num >= 0) && (num < NUM_INT_REGS));
  return int_registers[num];
}

// write the contents of an integer register
void
Machine::WriteIntRegister(int num, int64_t value) {
  ASSERT((num >= 0) && (num < NUM_INT_REGS));
  int_registers[num] = value;
}

// read the contents of a FP register
int64_t
Machine::ReadFPRegister(int num) {
  ASSERT((num >= 0) && (num < NUM_FP_REGS));
  return float_registers[num];
}

// store a value into a FP register
void
Machine::WriteFPRegister(int num, int64_t value) {
  ASSERT((num >= 0) && (num < NUM_FP_REGS));
  float_registers[num] = value;
}

//----------------------------------------------------------------------
// Machine::Run
/*! 	Make the RISCV machine start the execution of a user program.
//	Called by the kernel when the program starts up; never returns.
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
*/
//----------------------------------------------------------------------
void
Machine::Run() {
  Instruction instr;
  cycle = 0;

  // We initialize shiftmask
  uint64_t value = 0xffffffff;
  value = (value << 32) + value;
  for (int i = 0; i < 64; i++) {
    shiftMask[i] = value;
    value = value >> 1;
  }

  // Execution time of every executed instruction (for statistics)
  int tps;

  // We are now in user mode
  this->status = USER_MODE;

  // Machine main loop : execute instructions one at a time
  for (;;) {
    tps = OneInstruction(&instr);

    // machine mode is not set accordingly in case of page faults
    // triggered by the instruction... Have to fix that
    this->status = USER_MODE;

    // Advance simulated time and check if there are any pending
    // interrupts to be called.
    interrupt->OneTick(tps);

    // Call the debugger is required
    if (singleStep && (runUntilTime <= g_stats->getTotalTicks()))
      Debugger();
  }
}

//----------------------------------------------------------------------
// int Machine::OneInstruction
/*!	Execute one instruction from a user-level program
//
// 	If there is any kind of exception or interrupt, we invoke the
//	exception handler, and when it returns, we return to Run(), which
//	will re-invoke us in a loop.  This allows us to
//	re-start the instruction execution from the beginning, in
//	case any of our state has changed.  On a syscall,
// 	the OS software must increment the PC so execution begins
// 	at the instruction immediately after the syscall.
//
//	This routine is re-entrant, in that it can be called multiple
//	times concurrently -- one for each thread executing user code.
//	We get re-entrancy by never caching any data -- we always re-start the
//	simulation from scratch each time we are called (or after trapping
//	back to the Nachos kernel on an exception or interrupt), and we always
//	store all data back to the machine registers and memory before
//	leaving.  This allows the Nachos kernel to control our behavior
//	by controlling the contents of memory, the translation table,
//	and the register set.
//
//  \param instr Instruction to be executed
//  \return Execution time of the instruction in cycles
*/
//----------------------------------------------------------------------

int
Machine::OneInstruction(Instruction *instr) {
  int execution_time;   // execution time of the instruction
  if (!mmu->ReadMem(pc, 4, &(instr->value)))
    return 0;   // exception occurred
  instr->Decode();

  // Constant execution time for user instructions (see stats.h)
  execution_time = USER_TICK;

  // Update statistics
  g_current_thread->GetProcessOwner()->stat->incrNumInstruction();

  // Print its textual representation if debug flag 'm' is set
  if (DebugIsEnabled('m')) {
    printf("%s: \t[PC: 0x%" PRIx64 "] \t%s\n", g_current_thread->GetName(), pc,
           instr->printDecodedInstrRISCV(pc).c_str());
    // DumpState();
    // printf("[Process : %s] : [Cycle: %d] -- [PC: %x] -- [Binary Instruction:
    // %x] -- [Opcode: %x] -- [Total Time: %lu]\n", g_current_thread->GetName(),
    // (int)cycle, (int64_t)pc, (uint64_t) instr->value, instr->opcode,
    // g_stats->getTotalTicks()); 	printf("\t(Instruction details):
    // %s\n\n",
    // instr->printDecodedInstrRISCV().c_str());
  }

  pc = pc + 4;

  uint64_t unsignedReg1 = 0;
  uint64_t unsignedReg2 = 0;

  // int128_t longResult;
  __int128 longResult;

  int32_t localDataa, localDatab;
  int64_t localLongResult;
  uint32_t localDataaUnsigned, localDatabUnsigned;
  int32_t localResult;

  float localFloat;

  uint64_t value;

  // Execute the instruction

  // Look at the opCode field to perform the right action

  switch (instr->opcode) {

  case RISCV_LUI:
    int_registers[instr->rd] = instr->imm31_12;
    break;

  case RISCV_AUIPC:
    int_registers[instr->rd] = pc - 4 + instr->imm31_12;
    break;

  case RISCV_JAL:
    int_registers[instr->rd] = pc;
    pc = pc - 4 + instr->imm21_1_signed;
    break;

  case RISCV_JALR:
    localResult = pc;
    pc = (int_registers[instr->rs1] + instr->imm12_I_signed) & 0xfffffffe;
    int_registers[instr->rd] = localResult;
    break;
  //************************************************************************
  // Treatment for: BRANCH INSTRUCTIONS
  case RISCV_BR:
    switch (instr->funct3) {
    case RISCV_BR_BEQ:
      if (int_registers[instr->rs1] == int_registers[instr->rs2]) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    case RISCV_BR_BNE:
      if (int_registers[instr->rs1] != int_registers[instr->rs2]) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    case RISCV_BR_BLT:
      if (int_registers[instr->rs1] < int_registers[instr->rs2]) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    case RISCV_BR_BGE:
      if (int_registers[instr->rs1] >= int_registers[instr->rs2]) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    case RISCV_BR_BLTU:
      unsignedReg1 = (uint64_t) int_registers[instr->rs1];
      unsignedReg2 = (uint64_t) int_registers[instr->rs2];

      if (unsignedReg1 < unsignedReg2) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    case RISCV_BR_BGEU:
      unsignedReg1 = (uint64_t) int_registers[instr->rs1];
      unsignedReg2 = (uint64_t) int_registers[instr->rs2];

      if (unsignedReg1 >= unsignedReg2) {
        pc = pc + (instr->imm13_signed) - 4;
      }
      break;

    default:
      printf("In BR switch case, this should never happen... Instr was %x\n",
             (int) instr->value);
      exit(ERROR);
      break;
    }
    break;

  //************************************************************************
  // Treatment for: LOAD INSTRUCTIONS
  case RISCV_LD:
    switch (instr->funct3) {

    case RISCV_LD_LB: {
      int8_t val8;
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 1,
              (uint64_t *) &val8)) {
        printf("RISCV_LD_LB = FAILURE\n");
        return 0;
      }
      int_registers[instr->rd] = val8;
    } break;

    case RISCV_LD_LH: {
      int16_t val16;
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 2,
              (uint64_t *) &val16)) {
        printf("RISCV_LD_LH = FAILURE\n");
        return 0;
      }
      int_registers[instr->rd] = val16;
    } break;

    case RISCV_LD_LW: {
      int32_t val32;
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 4,
              (uint64_t *) &val32)) {
        printf("RISCV_LD_LW = FAILURE\n");
        return 0;
      }
      int_registers[instr->rd] = val32;
    } break;

    case RISCV_LD_LD:
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 8,
              (uint64_t *) &int_registers[instr->rd])) {
        printf("RISCV_LD_LD = FAILURE\n");
        return 0;
      }
      break;

    case RISCV_LD_LBU: {
      unsigned char v;
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 1,
              (uint64_t *) &v)) {
        printf("RISCV_LD_LBU = FAILURE\n");
        return 0;
      } else {
        int_registers[instr->rd] = v;
      }
    } break;

    case RISCV_LD_LHU: {
      unsigned char v;
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 2,
              (uint64_t *) &v)) {
        printf("RISCV_LD_LHU = FAILURE\n");
        return 0;
      } else {
        int_registers[instr->rd] = v;
      }
    } break;

    case RISCV_LD_LWU:
      if (!mmu->ReadMem(
              (uint64_t) (int_registers[instr->rs1] + instr->imm12_I_signed), 4,
              (uint64_t *) &int_registers[instr->rd])) {
        printf("RISCV_LD_LWU = FAILURE\n");
        return 0;
      }
      break;

    default:
      printf("In LD switch case, this should never happen... Instr was %x\n",
             (int) instr->value);
      exit(ERROR);
      break;
    }
    break;

  //************************************************************************
  // Treatment for: STORE INSTRUCTIONS
  case RISCV_ST:
    switch (instr->funct3) {
    case RISCV_ST_STB:

      if (!mmu->WriteMem((unsigned long long) (int_registers[instr->rs1] +
                                               instr->imm12_S_signed),
                         1, int_registers[instr->rs2])) {
        printf("RISCV_ST_STB = FAILURE\n");
        return 0;
      }
      break;

    case RISCV_ST_STH:
      if (!mmu->WriteMem((unsigned long long) (int_registers[instr->rs1] +
                                               instr->imm12_S_signed),
                         2, int_registers[instr->rs2])) {
        printf("RISCV_ST_STH = FAILURE\n");
        return 0;
      }
      break;

    case RISCV_ST_STW:
      if (!mmu->WriteMem((unsigned long long) (int_registers[instr->rs1] +
                                               instr->imm12_S_signed),
                         4, int_registers[instr->rs2])) {
        printf("RISCV_ST_STW = FAILURE\n");
        return 0;
      }
      break;

    case RISCV_ST_STD:
      if (!mmu->WriteMem((unsigned long long) (int_registers[instr->rs1] +
                                               instr->imm12_S_signed),
                         8, int_registers[instr->rs2])) {
        printf("RISCV_ST_STD = FAILURE\n");
        return 0;
      }
      // g_machine->Debugger();
      break;

    default:
      printf("In ST switch case, this should never happen... Instr was %x\n",
             (int) instr->value);
      exit(ERROR);
      break;
    }
    break;

  //************************************************************************
  // Treatment for: OPI INSTRUCTIONS
  case RISCV_OPI:
    switch (instr->funct3) {
    case RISCV_OPI_ADDI:
      int_registers[instr->rd] =
          int_registers[instr->rs1] + instr->imm12_I_signed;
      break;

    case RISCV_OPI_SLTI:
      int_registers[instr->rd] =
          (int_registers[instr->rs1] < instr->imm12_I_signed) ? 1 : 0;
      break;

    case RISCV_OPI_SLTIU:
      unsignedReg1 |= int_registers[instr->rs1] & 0xffffffff;
      int_registers[instr->rd] = (unsignedReg1 < instr->imm12_I) ? 1 : 0;
      break;

    case RISCV_OPI_XORI:
      int_registers[instr->rd] =
          int_registers[instr->rs1] ^ instr->imm12_I_signed;
      break;

    case RISCV_OPI_ORI:
      int_registers[instr->rd] =
          int_registers[instr->rs1] | instr->imm12_I_signed;
      break;

    case RISCV_OPI_ANDI:
      int_registers[instr->rd] =
          int_registers[instr->rs1] & instr->imm12_I_signed;
      break;

    case RISCV_OPI_SLLI:
      int_registers[instr->rd] = int_registers[instr->rs1] << instr->shamt;
      break;

    case RISCV_OPI_SRI:
      if (instr->funct7_smaller == RISCV_OPI_SRI_SRLI) {
        int_registers[instr->rd] = (int_registers[instr->rs1] >> instr->shamt) &
                                   shiftMask[instr->shamt];
      } else   // SRAI
        int_registers[instr->rd] = int_registers[instr->rs1] >> instr->shamt;
      break;

    default:
      printf("In OPI switch case, this should never happen... Instr was %x\n",
             (int) instr->value);
      exit(ERROR);
      break;
    }
    break;

  //************************************************************************
  // Treatment for: OPIW INSTRUCTIONS
  case RISCV_OPIW:
    localDataa = int_registers[instr->rs1];
    localDatab = instr->imm12_I_signed;
    switch (instr->funct3) {
    case RISCV_OPIW_ADDIW:
      localResult = localDataa + localDatab;
      int_registers[instr->rd] = localResult;
      break;
    case RISCV_OPIW_SLLIW:
      localResult = localDataa << instr->rs2;
      int_registers[instr->rd] = localResult;
      break;
    case RISCV_OPIW_SRW:
      if (instr->funct7 == RISCV_OPIW_SRW_SRLIW)
        localResult = (localDataa >> instr->rs2) & shiftMask[32 + instr->rs2];
      else   // SRAI
        localResult = localDataa >> instr->rs2;

      int_registers[instr->rd] = localResult;
      break;
    default:
      printf("In OPI switch case, this should never happen... Instr was %x\n",
             (int) instr->value);
      exit(ERROR);
      break;
    }
    break;

  //************************************************************************
  // Treatment for: OP INSTRUCTIONS
  case RISCV_OP:
    if (instr->funct7 == 1) {
      // Switch case for multiplication operations (in standard extension RV32M)
      switch (instr->funct3) {
      case RISCV_OP_M_MUL:
        longResult = int_registers[instr->rs1] * int_registers[instr->rs2];
        int_registers[instr->rd] = longResult & 0xffffffffffffffff;
        // int_registers[instr->rd]    = longResult.slc<64>(0);
        break;
      case RISCV_OP_M_MULH:
        longResult = int_registers[instr->rs1] * int_registers[instr->rs2];
        int_registers[instr->rd] = (longResult >> 64) & 0xffffffffffffffff;
        // int_registers[instr->rd]    = longResult.slc<64>(64);
        break;
      case RISCV_OP_M_MULHSU:
        unsignedReg2 = int_registers[instr->rs2];
        longResult = int_registers[instr->rs1] * unsignedReg2;
        int_registers[instr->rd] = (longResult >> 64) & 0xffffffffffffffff;
        // int_registers[instr->rd]      = longResult.slc<64>(64);
        break;
      case RISCV_OP_M_MULHU:
        unsignedReg1 = int_registers[instr->rs1];
        unsignedReg2 = int_registers[instr->rs2];
        longResult = unsignedReg1 * unsignedReg2;
        int_registers[instr->rd] = (longResult >> 64) & 0xffffffffffffffff;
        // int_registers[instr->rd]      = longResult.slc<64>(64);
        break;
      case RISCV_OP_M_DIV:
        int_registers[instr->rd] =
            (int_registers[instr->rs1] / int_registers[instr->rs2]);
        break;
      case RISCV_OP_M_DIVU:
        unsignedReg1 = int_registers[instr->rs1];
        unsignedReg2 = int_registers[instr->rs2];
        int_registers[instr->rd] = unsignedReg1 / unsignedReg2;
        break;
      case RISCV_OP_M_REM:
        int_registers[instr->rd] =
            (int_registers[instr->rs1] % int_registers[instr->rs2]);
        break;
      case RISCV_OP_M_REMU:
        unsignedReg1 = int_registers[instr->rs1];
        unsignedReg2 = int_registers[instr->rs2];
        int_registers[instr->rd] = unsignedReg1 % unsignedReg2;
        break;
      }

    } else {

      // Switch case for base OP operation
      switch (instr->funct3) {
      case RISCV_OP_ADD:
        if (instr->funct7 == RISCV_OP_ADD_ADD)
          int_registers[instr->rd] =
              int_registers[instr->rs1] + int_registers[instr->rs2];
        else
          int_registers[instr->rd] =
              int_registers[instr->rs1] - int_registers[instr->rs2];
        break;
      case RISCV_OP_SLL:
        int_registers[instr->rd] = int_registers[instr->rs1]
                                   << (int_registers[instr->rs2] & 0x3f);
        break;
      case RISCV_OP_SLT:
        int_registers[instr->rd] =
            (int_registers[instr->rs1] < int_registers[instr->rs2]) ? 1 : 0;
        break;
      case RISCV_OP_SLTU:
        unsignedReg1 = (uint64_t) int_registers[instr->rs1];
        unsignedReg2 = (uint64_t) int_registers[instr->rs2];
        // unsignedReg1.set_slc(0, int_registers[instr->rs1].slc<64>(0));
        // unsignedReg2.set_slc(0, int_registers[instr->rs2].slc<64>(0));

        int_registers[instr->rd] = (unsignedReg1 < unsignedReg2) ? 1 : 0;
        break;
      case RISCV_OP_XOR:
        int_registers[instr->rd] =
            int_registers[instr->rs1] ^ int_registers[instr->rs2];
        break;
      case RISCV_OP_SR:
        if (instr->funct7 == RISCV_OP_SR_SRL) {
          int shiftAmount = int_registers[instr->rs2];
          int_registers[instr->rd] = (int_registers[instr->rs1] >>
                                      (int_registers[instr->rs2] & 0x3f)) &
                                     shiftMask[(shiftAmount & 0x3f)];
        } else   // SRA
          int_registers[instr->rd] =
              int_registers[instr->rs1] >> (int_registers[instr->rs2] & 0x3f);
        break;
      case RISCV_OP_OR:
        int_registers[instr->rd] =
            int_registers[instr->rs1] | int_registers[instr->rs2];
        break;
      case RISCV_OP_AND:
        int_registers[instr->rd] =
            int_registers[instr->rs1] & int_registers[instr->rs2];
        break;
      default:
        printf("In OP switch case, this should never happen... Instr was %x\n",
               (int) instr->value);
        exit(ERROR);
        break;
      }
    }
    break;

  //************************************************************************
  // Treatment for: OPW INSTRUCTIONS
  case RISCV_OPW:
    if (instr->funct7 == 1) {
      localDataa = int_registers[instr->rs1] & 0xffffffff;
      localDatab = int_registers[instr->rs2] & 0xffffffff;
      localDataaUnsigned = int_registers[instr->rs1] & 0xffffffff;
      localDatabUnsigned = int_registers[instr->rs2] & 0xffffffff;

      // Switch case for multiplication operations (in standard extension RV32M)
      switch (instr->funct3) {
      case RISCV_OPW_M_MULW:
        localLongResult = localDataa * localDatab;
        int_registers[instr->rd] = localLongResult & 0xffffffff;
        break;

      case RISCV_OPW_M_DIVW:
        int_registers[instr->rd] = (localDataa / localDatab);
        break;

      case RISCV_OPW_M_DIVUW:
        int_registers[instr->rd] = localDataaUnsigned / localDatabUnsigned;
        break;

      case RISCV_OPW_M_REMW:
        int_registers[instr->rd] = (localDataa % localDatab);
        break;

      case RISCV_OPW_M_REMUW:
        int_registers[instr->rd] = localDataaUnsigned % localDatabUnsigned;
        break;
      }

    } else {
      localDataa = int_registers[instr->rs1] & 0xffffffff;
      localDatab = int_registers[instr->rs2] & 0xffffffff;

      // Switch case for base OP operation

      switch (instr->funct3) {
      case RISCV_OPW_ADDSUBW:
        if (instr->funct7 == RISCV_OPW_ADDSUBW_ADDW) {
          localResult = localDataa + localDatab;
          int_registers[instr->rd] = localResult;
        } else {   // SUBW
          localResult = localDataa - localDatab;
          int_registers[instr->rd] = localResult;
        }
        break;
      case RISCV_OPW_SLLW:
        localResult = localDataa << (localDatab & 0x1f);
        int_registers[instr->rd] = localResult;
        break;
      case RISCV_OPW_SRW:
        if (instr->funct7 == RISCV_OPW_SRW_SRLW) {
          localResult = (localDataa >> (localDatab /* & 0x1f*/)) &
                        shiftMask[32 + localDatab /* & 0x1f*/];
          int_registers[instr->rd] = localResult;
        } else {   // SRAW
          localResult = localDataa >> (localDatab /* & 0x1f*/);
          int_registers[instr->rd] = localResult;
        }
        break;
      default:
        printf("In OPW switch case, this should never happen... Instr was %x\n",
               (int) instr->value);
        exit(ERROR);
        break;
      }
    }
    break;
    // END of OP treatment

  //************************************************************************
  // Treatment for: SYSTEM INSTRUCTIONS
  case RISCV_SYSTEM:
    if (SYSCALL_EXCEPTION <= 33 && SYSCALL_EXCEPTION >= 0) {
      g_machine->RaiseException(SYSCALL_EXCEPTION, pc);
    } else {
      fprintf(stderr, "Unresolved system call %d\n", SYSCALL_EXCEPTION);
    }
    break;

  //************************************************************************
  // Treatment for: floating point operations
  case RISCV_FLW:
    if (!mmu->ReadMem(int_registers[instr->rs1] + instr->imm12_I_signed, 4,
                      &value))
      return 0;
    float_registers[instr->rd] = value;
    break;

  case RISCV_FSW:

    if (!mmu->WriteMem(
            (uint32_t) (int_registers[instr->rs1] + instr->imm12_S_signed), 4,
            float_registers[instr->rs2]))
      return 0;

    break;

  case RISCV_FMADD:
    float_registers[instr->rd] =
        float_registers[instr->rs1] * float_registers[instr->rs2] +
        float_registers[instr->rs3];
    break;

  case RISCV_FMSUB:
    float_registers[instr->rd] =
        float_registers[instr->rs1] * float_registers[instr->rs2] -
        float_registers[instr->rs3];
    break;

  case RISCV_FNMSUB:
    float_registers[instr->rd] =
        -float_registers[instr->rs1] * float_registers[instr->rs2] +
        float_registers[instr->rs3];
    break;

  case RISCV_FNMADD:
    float_registers[instr->rd] =
        -float_registers[instr->rs1] * float_registers[instr->rs2] -
        float_registers[instr->rs3];
    break;

  case RISCV_FP:
    switch (instr->funct7) {
    case RISCV_FP_ADD:
      float_registers[instr->rd] =
          float_registers[instr->rs1] + float_registers[instr->rs2];
      break;

    case RISCV_FP_SUB:
      float_registers[instr->rd] =
          float_registers[instr->rs1] - float_registers[instr->rs2];
      break;

    case RISCV_FP_MUL:
      float_registers[instr->rd] =
          float_registers[instr->rs1] * float_registers[instr->rs2];
      break;

    case RISCV_FP_DIV:
      float_registers[instr->rd] =
          float_registers[instr->rs1] / float_registers[instr->rs2];
      break;

    case RISCV_FP_SQRT:
      float_registers[instr->rd] = sqrt(float_registers[instr->rs1]);
      break;

    case RISCV_FP_FSGN:
      localFloat = fabs(float_registers[instr->rs1]);
      if (instr->funct3 == RISCV_FP_FSGN_J) {
        if (float_registers[instr->rs2] < 0) {
          float_registers[instr->rd] = -localFloat;
        } else {
          float_registers[instr->rd] = localFloat;
        }
      } else if (instr->funct3 == RISCV_FP_FSGN_JN) {
        if (float_registers[instr->rs2] < 0) {
          float_registers[instr->rd] = localFloat;
        } else {
          float_registers[instr->rd] = -localFloat;
        }
      } else {   // JX
        if ((float_registers[instr->rs2] < 0 &&
             float_registers[instr->rs1] >= 0) ||
            (float_registers[instr->rs2] >= 0 &&
             float_registers[instr->rs1] < 0)) {
          float_registers[instr->rd] = -localFloat;
        } else {
          float_registers[instr->rd] = localFloat;
        }
      }
      break;

    case RISCV_FP_MINMAX:
      if (instr->funct3 == RISCV_FP_MINMAX_MIN)
        float_registers[instr->rd] =
            MIN(float_registers[instr->rs1], float_registers[instr->rs2]);
      else
        float_registers[instr->rd] =
            MAX(float_registers[instr->rs1], float_registers[instr->rs2]);
      break;
    case RISCV_FP_FCVTW:

      if (instr->rs2 == RISCV_FP_FCVTW_W) {
        int_registers[instr->rd] = float_registers[instr->rs1];
      } else {
        int_registers[instr->rd] = (unsigned int) float_registers[instr->rs1];
      }
      break;

    case RISCV_FP_FMVXFCLASS:
      if (instr->funct3 == RISCV_FP_FMVXFCLASS_FMVX) {
        int_registers[instr->rd] = float_registers[instr->rs1];
      } else {
        fprintf(stderr,
                "Fclass instruction is not handled in riscv simulator\n");
        exit(-1);
      }
      break;

    case RISCV_FP_FCMP:
      if (instr->funct3 == RISCV_FP_FCMP_FEQ)
        int_registers[instr->rd] =
            float_registers[instr->rs1] == float_registers[instr->rs2];
      else if (instr->funct3 == RISCV_FP_FCMP_FLT)
        int_registers[instr->rd] =
            float_registers[instr->rs1] < float_registers[instr->rs2];
      else
        int_registers[instr->rd] =
            float_registers[instr->rs1] <= float_registers[instr->rs2];
      break;

    case RISCV_FP_FCVTS:
      if (instr->rs2 == RISCV_FP_FCVTS_W) {
        float_registers[instr->rd] = int_registers[instr->rs1];
      } else {
        float_registers[instr->rd] = (unsigned int) int_registers[instr->rs1];
      }
      break;

    case RISCV_FP_FMVW:
      float_registers[instr->rd] = int_registers[instr->rs1];
      break;

      /*case RISCV_FP_FCVTDS:
    printf("*** Test nouvelle instr\n");
    float_registers[instr->rd] = float_registers[instr->rs1];
    break;

  case RISCV_FP_FMVXD:
    printf("*** Test nouvelle instr\n");
    int_registers[instr->rd] = float_registers[instr->rs1];
    break;
  case RISCV_FP_FMVDX:
    printf("*** Test nouvelle instr\n");
    float_registers[instr->rd] = int_registers[instr->rs1];
    break;*/

    default:
      printf("Unrecognized instruction, exiting.\n");
      // printf("In FP part of switch opcode, instr 0x%x func7 0x%x is not "
      //        "handled yet OPCode : 0x%x, PC : 0x%lx)  cycle is %d\n\n",
      //        instr->value, instr->funct7, instr->opcode, pc - 4, (int)
      //        cycle);
      exit(-1);
      break;
    }
    break;

  default:
    printf("In default part of switch opcode, instr %x is not handled yet "
           "(OPCode : %x, PC : %" PRIx64 ")  cycle is %d\n\n",
           instr->opcode, instr->opcode, pc - 4, (int) cycle);
    exit(-1);
    break;
  }

  int_registers[0] = 0;
  n_inst = n_inst + 1;
  cycle++;

  // Now we have successfully executed the instruction.
  return execution_time;
}

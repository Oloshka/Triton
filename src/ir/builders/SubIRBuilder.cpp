#include <iostream>
#include <sstream>
#include <stdexcept>

#include "SubIRBuilder.h"
#include "Registers.h"
#include "SMT2Lib.h"
#include "SymbolicElement.h"


SubIRBuilder::SubIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


void SubIRBuilder::regImm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint64_t          reg     = this->operands[0].getValue();
  uint64_t          imm     = this->operands[1].getValue();

  uint64_t          symReg  = ap.getRegSymbolicID(reg);
  uint32_t          regSize = this->operands[0].getSize();

  /* Create the SMT semantic */
  /* OP_1 */
  if (symReg != UNSET)
    op1 << smt2lib::extract(regSize, "#" + std::to_string(symReg));
  else
    op1 << smt2lib::bv(ap.getRegisterValue(reg), regSize * REG_SIZE);

  /* OP_2 */
  op2 << smt2lib::bv(imm, regSize * REG_SIZE);

  /* Finale expr */
  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(expr, reg);

  /* Apply the taint */
  ap.aluSpreadTaintRegImm(se, reg);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  inst.addElement(EflagsBuilder::af(se, ap, regSize, op1, op2));
  inst.addElement(EflagsBuilder::cfSub(se, ap, op1, op2));
  inst.addElement(EflagsBuilder::ofSub(se, ap, regSize, op1, op2));
  inst.addElement(EflagsBuilder::pf(se, ap));
  inst.addElement(EflagsBuilder::sf(se, ap, regSize));
  inst.addElement(EflagsBuilder::zf(se, ap, regSize));
}


void SubIRBuilder::regReg(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint64_t          reg1     = this->operands[0].getValue();
  uint64_t          reg2     = this->operands[1].getValue();

  uint64_t          symReg1  = ap.getRegSymbolicID(reg1);
  uint64_t          symReg2  = ap.getRegSymbolicID(reg2);
  uint32_t          regSize1 = this->operands[0].getSize();
  uint32_t          regSize2 = this->operands[1].getSize();


  /* Create the SMT semantic */
  // OP_1
  if (symReg1 != UNSET)
    op1 << smt2lib::extract(regSize1, "#" + std::to_string(symReg1));
  else
    op1 << smt2lib::bv(ap.getRegisterValue(reg1), regSize1 * REG_SIZE);

  // OP_2
  if (symReg2 != UNSET)
    op2 << smt2lib::extract(regSize2, "#" + std::to_string(symReg2));
  else
    op2 << smt2lib::bv(ap.getRegisterValue(reg2), regSize1 * REG_SIZE);

  // Final expr
  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(expr, reg1);

  /* Apply the taint */
  ap.aluSpreadTaintRegReg(se, reg1, reg2);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  inst.addElement(EflagsBuilder::af(se, ap, regSize1, op1, op2));
  inst.addElement(EflagsBuilder::cfSub(se, ap, op1, op2));
  inst.addElement(EflagsBuilder::ofSub(se, ap, regSize1, op1, op2));
  inst.addElement(EflagsBuilder::pf(se, ap));
  inst.addElement(EflagsBuilder::sf(se, ap, regSize1));
  inst.addElement(EflagsBuilder::zf(se, ap, regSize1));
}


void SubIRBuilder::regMem(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          readSize = this->operands[1].getSize();
  uint64_t          mem      = this->operands[1].getValue();
  uint64_t          reg      = this->operands[0].getValue();

  uint64_t          symReg   = ap.getRegSymbolicID(reg);
  uint64_t          symMem   = ap.getMemSymbolicID(mem);
  uint32_t          regSize  = this->operands[0].getSize();

  /* Create the SMT semantic */
  // OP_1
  if (symReg != UNSET)
    op1 << smt2lib::extract(regSize, "#" + std::to_string(symReg));
  else
    op1 << smt2lib::bv(ap.getRegisterValue(reg), readSize * REG_SIZE);

  // OP_2
  if (symMem != UNSET)
    op2 << "#" << std::dec << symMem;
  else
    op2 << smt2lib::bv(ap.getMemValue(mem, readSize), readSize * REG_SIZE);

  // Final expr
  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(expr, reg);

  /* Apply the taint */
  ap.aluSpreadTaintRegMem(se, reg, mem, readSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  inst.addElement(EflagsBuilder::af(se, ap, regSize, op1, op2));
  inst.addElement(EflagsBuilder::cfSub(se, ap, op1, op2));
  inst.addElement(EflagsBuilder::ofSub(se, ap, regSize, op1, op2));
  inst.addElement(EflagsBuilder::pf(se, ap));
  inst.addElement(EflagsBuilder::sf(se, ap, regSize));
  inst.addElement(EflagsBuilder::zf(se, ap, regSize));
}


void SubIRBuilder::memImm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          writeSize = this->operands[0].getSize();
  uint64_t          mem       = this->operands[0].getValue();
  uint64_t          imm       = this->operands[1].getValue();

  uint64_t          symMem    = ap.getMemSymbolicID(mem);

  /* Create the SMT semantic */
  /* OP_1 */
  if (symMem != UNSET)
    op1 << "#" << std::dec << symMem;
  else
    op1 << smt2lib::bv(ap.getMemValue(mem, writeSize), writeSize * REG_SIZE);

  /* OP_2 */
  op2 << smt2lib::bv(imm, writeSize * REG_SIZE);

  /* Final expr */
  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createMemSE(expr, mem);

  /* Apply the taint */
  ap.aluSpreadTaintMemImm(se, mem, writeSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  inst.addElement(EflagsBuilder::af(se, ap, writeSize, op1, op2));
  inst.addElement(EflagsBuilder::cfSub(se, ap, op1, op2));
  inst.addElement(EflagsBuilder::ofSub(se, ap, writeSize, op1, op2));
  inst.addElement(EflagsBuilder::pf(se, ap));
  inst.addElement(EflagsBuilder::sf(se, ap, writeSize));
  inst.addElement(EflagsBuilder::zf(se, ap, writeSize));
}


void SubIRBuilder::memReg(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          writeSize = this->operands[0].getSize();
  uint64_t          mem       = this->operands[0].getValue();
  uint64_t          reg       = this->operands[1].getValue();
  uint32_t          regSize   = this->operands[1].getSize();

  uint64_t          symReg    = ap.getRegSymbolicID(reg);
  uint64_t          symMem    = ap.getMemSymbolicID(mem);

  /* Create the SMT semantic */
  // OP_1
  if (symMem != UNSET)
    op1 << "#" << std::dec << symMem;
  else
    op1 << smt2lib::bv(ap.getMemValue(mem, writeSize), writeSize * REG_SIZE);

  // OP_2
  if (symReg != UNSET)
    op2 << smt2lib::extract(regSize, "#" + std::to_string(symReg));
  else
    op2 << smt2lib::bv(ap.getRegisterValue(reg), writeSize * REG_SIZE);

  // Final expr
  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createMemSE(expr, mem);

  /* Apply the taint */
  ap.aluSpreadTaintMemReg(se, mem, reg, writeSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  inst.addElement(EflagsBuilder::af(se, ap, writeSize, op1, op2));
  inst.addElement(EflagsBuilder::cfSub(se, ap, op1, op2));
  inst.addElement(EflagsBuilder::ofSub(se, ap, writeSize, op1, op2));
  inst.addElement(EflagsBuilder::pf(se, ap));
  inst.addElement(EflagsBuilder::sf(se, ap, writeSize));
  inst.addElement(EflagsBuilder::zf(se, ap, writeSize));
}


Inst *SubIRBuilder::process(AnalysisProcessor &ap) const {
  this->checkSetup();

  Inst *inst = new Inst(ap.getThreadID(), this->address, this->disas);

  try {
    this->templateMethod(ap, *inst, this->operands, "SUB");
    ap.incNumberOfExpressions(inst->numberOfElements()); /* Used for statistics */
    inst->addElement(ControlFlow::rip(ap, this->nextAddress));
  }
  catch (std::exception &e) {
    delete inst;
    throw;
  }

  return inst;
}


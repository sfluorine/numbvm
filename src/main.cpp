#include <iostream>

#include "vm.h"

static void push_index(std::vector<uint8_t>& program, uint64_t index) {
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (index >> (i * 8) & 0xFF);
        program.push_back(byte);
    }
}

int main()
{
    NumbVm vm;
    std::vector<uint8_t> program;

    auto x = vm.put_i64(69);
    program.push_back(INSTRUCTION_TO_BYTE(Instruction::I64Const));
    push_index(program, x);
    auto y = vm.put_i64(420);
    program.push_back(INSTRUCTION_TO_BYTE(Instruction::I64Const));
    push_index(program, y);
    auto z = vm.put_f64(69.420);
    program.push_back(INSTRUCTION_TO_BYTE(Instruction::F64Const));
    push_index(program, z);
    program.push_back(INSTRUCTION_TO_BYTE(Instruction::Halt));

    vm.set_program(std::move(program));
    vm.execute();

    vm.dump_stack();
}

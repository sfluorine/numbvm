#include <iostream>

#include "vm.h"

static void push_index(std::vector<uint8_t>& program, uint64_t index)
{
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (index >> (i * 8) & 0xFF);
        program.push_back(byte);
    }
}

int main()
{
    NumbVm vm;
    std::vector<uint8_t> program;

    program.push_back(INSTRUCTION_TO_BYTE(Instruction::F64Const));
    push_index(program, vm.put_f64(69.420));

    program.push_back(INSTRUCTION_TO_BYTE(Instruction::SetGlobal));
    push_index(program, vm.put_global(Value()));

    program.push_back(INSTRUCTION_TO_BYTE(Instruction::GetGlobal));
    push_index(program, 0);

    vm.set_program(std::move(program));
    vm.execute();

    vm.dump_stack();
}

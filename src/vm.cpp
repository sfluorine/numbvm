#include "vm.h"

#include <format>
#include <iostream>

NumbVm::NumbVm()
    : m_halt(true)
    , m_pc(-1)
    , m_sp(-1)
{
    m_stack.reserve(2048);
}

uint64_t NumbVm::put_i64(int64_t i64)
{
    return m_i64_pool.put(i64);
}

uint64_t NumbVm::put_f64(double f64)
{
    return m_f64_pool.put(f64);
}

uint64_t NumbVm::put_global(Value value)
{
    return m_global.put(std::move(value));
}

void NumbVm::set_program(std::vector<uint8_t> program)
{
    m_program = std::move(program);
}

void NumbVm::execute()
{
    if (m_program.size() > 0) {
        m_halt = false;
    }

    while (!m_halt) {
        catch_trap(eval());
    }
}

void NumbVm::dump_stack()
{
    for (auto i = 0; i <= m_sp; i++) {
        std::cout << "sp: " << i << " -> ";
        switch (m_stack[i].type()) {
        case Value::Type::Undefined:
            std::cout << "undefined" << '\n';
            break;
        case Value::Type::I64:
            std::cout << m_stack[i].as_i64() << '\n';
            break;
        case Value::Type::F64:
            std::cout << m_stack[i].as_f64() << '\n';
            break;
        case Value::Type::Object:
            std::cout << m_stack[i].as_object() << '\n';
        }
    }
}

NumbTrap NumbVm::eval()
{
    // TODO: implement this.
    switch (BYTE_TO_INSTRUCTION(fetch())) {
    case Instruction::Halt: {
        m_halt = true;
        break;
    }
    case Instruction::I64Const: {
        uint64_t index = 0;

        for (int i = 0; i < 8; i++) {
            index |= static_cast<uint64_t>(fetch()) << (i * 8);
        }

        auto* constant = m_i64_pool.get(index);
        if (constant == nullptr) {
            return NumbTrap::InvalidOperand;
        }

        return push_i64(*constant);
        break;
    }
    case Instruction::F64Const: {
        uint64_t index = 0;

        for (int i = 0; i < 8; i++) {
            index |= static_cast<uint64_t>(fetch()) << (i * 8);
        }

        auto* constant = m_f64_pool.get(index);
        if (constant == nullptr) {
            return NumbTrap::InvalidOperand;
        }

        return push_f64(*constant);
        break;
    }
    case Instruction::Pop: {
        return pop();
        break;
    }
    case Instruction::SetGlobal: {
        uint64_t index = 0;

        for (int i = 0; i < 8; i++) {
            index |= static_cast<uint64_t>(fetch()) << (i * 8);
        }

        auto* global = m_global.get(index);
        if (global == nullptr) {
            return NumbTrap::GlobalNotFound;
        }

        if (m_sp < 0) {
            return NumbTrap::StackUnderflow;
        }

        *global = m_stack[m_sp];
        return pop();

        break;
    }
    case Instruction::GetGlobal: {
        uint64_t index = 0;

        for (int i = 0; i < 8; i++) {
            index |= static_cast<uint64_t>(fetch()) << (i * 8);
        }

        auto* global = m_global.get(index);
        if (global == nullptr) {
            return NumbTrap::GlobalNotFound;
        }

        if (m_sp >= static_cast<int64_t>(m_stack.capacity())) {
            return NumbTrap::StackOverflow;
        }

        m_stack[++m_sp] = *global;
        break;
    }
    }
    return NumbTrap::OK;
}

NumbTrap NumbVm::push_i64(int64_t i64)
{
    if (m_sp >= static_cast<int64_t>(m_stack.capacity())) {
        return NumbTrap::StackOverflow;
    }

    m_stack[++m_sp] = Value(i64);
    return NumbTrap::OK;
}

NumbTrap NumbVm::push_f64(double f64)
{
    if (m_sp >= static_cast<int64_t>(m_stack.capacity())) {
        return NumbTrap::StackOverflow;
    }

    m_stack[++m_sp] = Value(f64);
    return NumbTrap::OK;
}

NumbTrap NumbVm::pop()
{
    if (m_sp < 0) {
        return NumbTrap::StackUnderflow;
    }

    m_sp--;
    return NumbTrap::OK;
}

void NumbVm::catch_trap(NumbTrap trap)
{
    switch (trap) {
    case NumbTrap::StackOverflow: {
        emit_trap("stack overflow at sp: {}", m_sp);
        m_halt = true;
        break;
    }
    case NumbTrap::StackUnderflow: {
        emit_trap("stack underflow at sp: {}", m_sp);
        m_halt = true;
        break;
    }
    case NumbTrap::InvalidOperand: {
        emit_trap("invalid operand at pc: {}", m_pc);
        m_halt = true;
        break;
    }
    case NumbTrap::GlobalNotFound: {
        emit_trap("global not found", m_pc);
        m_halt = true;
        break;
    }
    case NumbTrap::OK:
        break;
    }
}

uint8_t NumbVm::fetch()
{
    return m_program[++m_pc];
}

template<typename... Args>
void NumbVm::emit_trap(char const* fmt, Args&&... args)
{
    std::cerr << std::format("[VM TRAP]: pc: {}\n  -> {}\n", m_pc, std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

template<typename T>
class Pool {
public:
    Pool()
        : m_index(0)
    {
    }

    template<typename... Args>
    uint64_t put(Args&&... args)
    {
        m_pool.push_back(T(std::forward<Args>(args)...));
        return m_index++;
    }

    T* get(uint64_t index)
    {
        if (index > m_index) {
            return nullptr;
        }

        return &m_pool[index];
    }

private:
    uint64_t m_index;
    std::vector<T> m_pool;
};

enum class Instruction : uint8_t {
    Halt,
    I64Const,
    F64Const,
    Pop,
    SetGlobal,
    GetGlobal,
};

enum class NumbTrap : uint8_t {
    StackOverflow,
    StackUnderflow,
    InvalidOperand,
    GlobalNotFound,
    OK,
};

#define INSTRUCTION_TO_BYTE(ins) (static_cast<uint8_t>(ins))
#define BYTE_TO_INSTRUCTION(byte) (static_cast<Instruction>(byte))

class Object;

struct Undefined {
};

class Value {
public:
    enum class Type {
        Undefined,
        I64,
        F64,
        Object,
    };

    Value()
        : m_type(Type::Undefined)
        , m_data(Undefined {})
    {
    }

    Value(int64_t i64)
        : m_type(Type::I64)
        , m_data(i64)
    {
    }

    Value(double f64)
        : m_type(Type::F64)
        , m_data(f64)
    {
    }

    Value(Object* object)
        : m_type(Type::Object)
        , m_data(object)
    {
    }

    Type type() const { return m_type; }

    int64_t as_i64() const { return std::get<int64_t>(m_data); }

    double as_f64() const { return std::get<double>(m_data); }

    Object* as_object() const { return std::get<Object*>(m_data); }

private:
    Type m_type;
    std::variant<Undefined, int64_t, double, Object*> m_data;
};

struct Object {
    bool marked { false };
    std::map<std::string, Value> fields;
};

class NumbVm {
public:
    NumbVm();

    uint64_t put_i64(int64_t);

    uint64_t put_f64(double);

    uint64_t put_global(Value value);

    void set_program(std::vector<uint8_t>);

    void execute();

    void dump_stack();

private:
    NumbTrap eval();

    NumbTrap push_i64(int64_t);

    NumbTrap push_f64(double);

    NumbTrap pop();

    void catch_trap(NumbTrap);

    uint8_t fetch();

    template<typename... Args>
    void emit_trap(char const*, Args&&...);

private:
    bool m_halt;

    int64_t m_pc; // program counter
    int64_t m_sp; // stack pointer

    std::vector<Value> m_stack;
    std::vector<uint8_t> m_program;

    Pool<Value> m_global;
    Pool<int64_t> m_i64_pool;
    Pool<double> m_f64_pool;
};

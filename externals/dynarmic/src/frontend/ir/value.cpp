/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include "common/assert.h"
#include "frontend/ir/microinstruction.h"
#include "frontend/ir/value.h"

namespace Dynarmic {
namespace IR {

Value::Value(Inst* value) : type(Type::Opaque) {
    inner.inst = value;
}

Value::Value(Arm::Reg value) : type(Type::RegRef) {
    inner.imm_regref = value;
}

Value::Value(Arm::ExtReg value) : type(Type::ExtRegRef) {
    inner.imm_extregref = value;
}

Value::Value(bool value) : type(Type::U1) {
    inner.imm_u1 = value;
}

Value::Value(u8 value) : type(Type::U8) {
    inner.imm_u8 = value;
}

Value::Value(u16 value) : type(Type::U16) {
    inner.imm_u16 = value;
}

Value::Value(u32 value) : type(Type::U32) {
    inner.imm_u32 = value;
}

Value::Value(u64 value) : type(Type::U64) {
    inner.imm_u64 = value;
}

Value::Value(std::array<u8, 8> value) : type(Type::CoprocInfo) {
    inner.imm_coproc = value;
}

bool Value::IsImmediate() const {
    if (type == Type::Opaque)
        return inner.inst->GetOpcode() == Opcode::Identity ? inner.inst->GetArg(0).IsImmediate() : false;
    return true;
}

bool Value::IsEmpty() const {
    return type == Type::Void;
}

Type Value::GetType() const {
    if (type == Type::Opaque) {
        if (inner.inst->GetOpcode() == Opcode::Identity) {
            return inner.inst->GetArg(0).GetType();
        } else {
            return inner.inst->GetType();
        }
    }
    return type;
}

Arm::Reg Value::GetRegRef() const {
    ASSERT(type == Type::RegRef);
    return inner.imm_regref;
}

Arm::ExtReg Value::GetExtRegRef() const {
    ASSERT(type == Type::ExtRegRef);
    return inner.imm_extregref;
}

Inst* Value::GetInst() const {
    ASSERT(type == Type::Opaque);
    return inner.inst;
}

bool Value::GetU1() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetU1();
    ASSERT(type == Type::U1);
    return inner.imm_u1;
}

u8 Value::GetU8() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetU8();
    ASSERT(type == Type::U8);
    return inner.imm_u8;
}

u16 Value::GetU16() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetU16();
    ASSERT(type == Type::U16);
    return inner.imm_u16;
}

u32 Value::GetU32() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetU32();
    ASSERT(type == Type::U32);
    return inner.imm_u32;
}

u64 Value::GetU64() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetU64();
    ASSERT(type == Type::U64);
    return inner.imm_u64;
}

std::array<u8, 8> Value::GetCoprocInfo() const {
    if (type == Type::Opaque && inner.inst->GetOpcode() == Opcode::Identity)
        return inner.inst->GetArg(0).GetCoprocInfo();
    ASSERT(type == Type::CoprocInfo);
    return inner.imm_coproc;
}

} // namespace IR
} // namespace Dynarmic

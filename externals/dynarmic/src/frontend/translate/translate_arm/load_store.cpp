/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include "translate_arm.h"

namespace Dynarmic {
namespace Arm {

bool ArmTranslatorVisitor::arm_LDRBT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_LDRHT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_LDRSBT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_LDRSHT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_LDRT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_STRBT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_STRHT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

bool ArmTranslatorVisitor::arm_STRT() {
    ASSERT_MSG(false, "System instructions unimplemented");
}

static IR::Value GetAddress(IR::IREmitter& ir, bool P, bool U, bool W, Reg n, IR::Value offset) {
    const bool index = P;
    const bool add = U;
    const bool wback = !P || W;

    const auto offset_addr = add ? ir.Add(ir.GetRegister(n), offset) : ir.Sub(ir.GetRegister(n), offset);
    const auto address = index ? offset_addr : ir.GetRegister(n);

    if (wback) {
        ir.SetRegister(n, offset_addr);
    }

    return address;
}

bool ArmTranslatorVisitor::arm_LDR_lit(Cond cond, bool U, Reg t, Imm12 imm12) {
    const bool add = U;

    // LDR <Rt>, [PC, #+/-<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm12) : (base - imm12);
        const auto data = ir.ReadMemory32(ir.Imm32(address));

        if (t == Reg::PC) {
            ir.LoadWritePC(data);
            ir.SetTerm(IR::Term::ReturnToDispatch{});
            return false;
        }

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDR_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm12 imm12) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if ((!P || W) && n == t)
        return UnpredictableInstruction();

    const u32 imm32 = imm12;

    // LDR <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDR <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ReadMemory32(address);

        if (t == Reg::PC) {
            ir.LoadWritePC(data);
            if (!P && W && n == Reg::R13)
                ir.SetTerm(IR::Term::PopRSBHint{});
            else
                ir.SetTerm(IR::Term::ReturnToDispatch{});
            return false;
        }

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDR_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm5 imm5, ShiftType shift, Reg m) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (m == Reg::PC)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // LDR <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDR <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = EmitImmShift(ir.GetRegister(m), shift, imm5, ir.GetCFlag()).result;
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ReadMemory32(address);

        if (t == Reg::PC) {
            ir.LoadWritePC(data);
            ir.SetTerm(IR::Term::ReturnToDispatch{});
            return false;
        }

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRB_lit(Cond cond, bool U, Reg t, Imm12 imm12) {
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = imm12;
    const bool add = U;

    // LDRB <Rt>, [PC, #+/-<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm32) : (base - imm32);
        const auto data = ir.ZeroExtendByteToWord(ir.ReadMemory8(ir.Imm32(address)));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm12 imm12) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if ((!P || W) && n == t)
        return UnpredictableInstruction();
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = imm12;

    // LDRB <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDRB <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ZeroExtendByteToWord(ir.ReadMemory8(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm5 imm5, ShiftType shift, Reg m) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // LDRB <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDRB <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = EmitImmShift(ir.GetRegister(m), shift, imm5, ir.GetCFlag()).result;
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ZeroExtendByteToWord(ir.ReadMemory8(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRD_lit(Cond cond, bool U, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (RegNumber(t) % 2 == 1)
        return UnpredictableInstruction();
    if (t+1 == Reg::PC)
        return UnpredictableInstruction();

    const Reg t2 = t+1;
    const u32 imm32 = (imm8a << 4) | imm8b;
    const bool add = U;

    // LDRD <Rt>, <Rt2>, [PC, #+/-<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm32) : (base - imm32);
        const auto data_a = ir.ReadMemory32(ir.Imm32(address));
        const auto data_b = ir.ReadMemory32(ir.Imm32(address + 4));

        ir.SetRegister(t, data_a);
        ir.SetRegister(t2, data_b);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRD_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    if (RegNumber(t) % 2 == 1)
        return UnpredictableInstruction();
    if (!P && W)
        return UnpredictableInstruction();
    if ((!P || W) && (n == t || n == t+1))
        return UnpredictableInstruction();
    if (t+1 == Reg::PC)
        return UnpredictableInstruction();

    const Reg t2 = t+1;
    const u32 imm32 = (imm8a << 4) | imm8b;

    // LDRD <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDRD <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address_a = GetAddress(ir, P, U, W, n, offset);
        const auto address_b = ir.Add(address_a, ir.Imm32(4));
        const auto data_a = ir.ReadMemory32(address_a);
        const auto data_b = ir.ReadMemory32(address_b);

        ir.SetRegister(t, data_a);
        ir.SetRegister(t2, data_b);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRD_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    if (RegNumber(t) % 2 == 1)
        return UnpredictableInstruction();
    if (!P && W)
        return UnpredictableInstruction();
    if (t+1 == Reg::PC || m == Reg::PC || m == t || m == t+1)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t || n == t+1))
        return UnpredictableInstruction();

    const Reg t2 = t+1;

    // LDRD <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDRD <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address_a = GetAddress(ir, P, U, W, n, offset);
        const auto address_b = ir.Add(address_a, ir.Imm32(4));
        const auto data_a = ir.ReadMemory32(address_a);
        const auto data_b = ir.ReadMemory32(address_b);

        ir.SetRegister(t, data_a);
        ir.SetRegister(t2, data_b);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRH_lit(Cond cond, bool P, bool U, bool W, Reg t, Imm4 imm8a, Imm4 imm8b) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (P == W)
        return UnpredictableInstruction();
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;
    const bool add = U;

    // LDRH <Rt>, [PC, #-/+<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm32) : (base - imm32);
        const auto data = ir.ZeroExtendHalfToWord(ir.ReadMemory16(ir.Imm32(address)));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if ((!P || W) && n == t)
        return UnpredictableInstruction();
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;

    // LDRH <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDRH <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ZeroExtendHalfToWord(ir.ReadMemory16(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // LDRH <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDRH <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.ZeroExtendHalfToWord(ir.ReadMemory16(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSB_lit(Cond cond, bool U, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;
    const bool add = U;

    // LDRSB <Rt>, [PC, #+/-<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm32) : (base - imm32);
        const auto data = ir.SignExtendByteToWord(ir.ReadMemory8(ir.Imm32(address)));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if ((!P || W) && n == t)
        return UnpredictableInstruction();
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;

    // LDRSB <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDRSB <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.SignExtendByteToWord(ir.ReadMemory8(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // LDRSB <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDRSB <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.SignExtendByteToWord(ir.ReadMemory8(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSH_lit(Cond cond, bool U, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;
    const bool add = U;

    // LDRSH <Rt>, [PC, #-/+<imm>]
    if (ConditionPassed(cond)) {
        const u32 base = ir.AlignPC(4);
        const u32 address = add ? (base + imm32) : (base - imm32);
        const auto data = ir.SignExtendHalfToWord(ir.ReadMemory16(ir.Imm32(address)));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (n == Reg::PC)
        return UnpredictableInstruction();
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if ((!P || W) && n == t)
        return UnpredictableInstruction();
    if (t == Reg::PC)
        return UnpredictableInstruction();

    const u32 imm32 = (imm8a << 4) | imm8b;

    // LDRSH <Rt>, [<Rn>, #+/-<imm>]{!}
    // LDRSH <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.SignExtendHalfToWord(ir.ReadMemory16(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDRSH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    ASSERT_MSG(!(!P && W), "T form of instruction unimplemented");
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if ((!P || W) && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // LDRSH <Rt>, [<Rn>, #+/-<Rm>]{!}
    // LDRSH <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        const auto data = ir.SignExtendHalfToWord(ir.ReadMemory16(address));

        ir.SetRegister(t, data);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STR_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm12 imm12) {
    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // STR <Rt>, [<Rn>, #+/-<imm>]{!}
    // STR <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm12);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory32(address, ir.GetRegister(t));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STR_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm5 imm5, ShiftType shift, Reg m) {
    if (m == Reg::PC)
        return UnpredictableInstruction();

    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // STR <Rt>, [<Rn>, #+/-<Rm>]{!}
    // STR <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = EmitImmShift(ir.GetRegister(m), shift, imm5, ir.GetCFlag()).result;
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory32(address, ir.GetRegister(t));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm12 imm12) {
    if (t == Reg::PC)
        return UnpredictableInstruction();

    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // STRB <Rt>, [<Rn>, #+/-<imm>]{!}
    // STRB <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm12);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory8(address, ir.LeastSignificantByte(ir.GetRegister(t)));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm5 imm5, ShiftType shift, Reg m) {
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();

    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // STRB <Rt>, [<Rn>, #+/-<Rm>]{!}
    // STRB <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = EmitImmShift(ir.GetRegister(m), shift, imm5, ir.GetCFlag()).result;
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory8(address, ir.LeastSignificantByte(ir.GetRegister(t)));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRD_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (size_t(t) % 2 != 0)
        return UnpredictableInstruction();
    if (!P && W)
        return UnpredictableInstruction();

    const u32 imm32 = imm8a << 4 | imm8b;
    const Reg t2 = t + 1;

    if (W && (n == Reg::PC || n == t || n == t2))
        return UnpredictableInstruction();
    if (t2 == Reg::PC)
        return UnpredictableInstruction();

    // STRD <Rt>, [<Rn>, #+/-<imm>]{!}
    // STRD <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address_a = GetAddress(ir, P, U, W, n, offset);
        const auto address_b = ir.Add(address_a, ir.Imm32(4));
        const auto value_a = ir.GetRegister(t);
        const auto value_b = ir.GetRegister(t2);
        ir.WriteMemory32(address_a, value_a);
        ir.WriteMemory32(address_b, value_b);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRD_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    if (size_t(t) % 2 != 0)
        return UnpredictableInstruction();
    if (!P && W)
        return UnpredictableInstruction();

    const Reg t2 = t + 1;

    if (t2 == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if (W && (n == Reg::PC || n == t || n == t2))
        return UnpredictableInstruction();

    // STRD <Rt>, [<Rn>, #+/-<Rm>]{!}
    // STRD <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address_a = GetAddress(ir, P, U, W, n, offset);
        const auto address_b = ir.Add(address_a, ir.Imm32(4));
        const auto value_a = ir.GetRegister(t);
        const auto value_b = ir.GetRegister(t2);
        ir.WriteMemory32(address_a, value_a);
        ir.WriteMemory32(address_b, value_b);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Imm4 imm8a, Imm4 imm8b) {
    if (t == Reg::PC)
        return UnpredictableInstruction();
    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    const u32 imm32 = imm8a << 4 | imm8b;

    // STRH <Rt>, [<Rn>, #+/-<imm>]{!}
    // STRH <Rt>, [<Rn>], #+/-<imm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.Imm32(imm32);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory16(address, ir.LeastSignificantHalf(ir.GetRegister(t)));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STRH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg t, Reg m) {
    if (t == Reg::PC || m == Reg::PC)
        return UnpredictableInstruction();
    if (W && (n == Reg::PC || n == t))
        return UnpredictableInstruction();

    // STRH <Rt>, [<Rn>, #+/-<Rm>]{!}
    // STRH <Rt>, [<Rn>], #+/-<Rm>
    if (ConditionPassed(cond)) {
        const auto offset = ir.GetRegister(m);
        const auto address = GetAddress(ir, P, U, W, n, offset);
        ir.WriteMemory16(address, ir.LeastSignificantHalf(ir.GetRegister(t)));
    }
    return true;
}

static bool LDMHelper(IR::IREmitter& ir, bool W, Reg n, RegList list, IR::Value start_address, IR::Value writeback_address) {
    auto address = start_address;
    for (size_t i = 0; i <= 14; i++) {
        if (Common::Bit(i, list)) {
            ir.SetRegister(static_cast<Reg>(i), ir.ReadMemory32(address));
            address = ir.Add(address, ir.Imm32(4));
        }
    }
    if (W && !Common::Bit(RegNumber(n), list)) {
        ir.SetRegister(n, writeback_address);
    }
    if (Common::Bit<15>(list)) {
        ir.LoadWritePC(ir.ReadMemory32(address));
        if (n == Reg::R13)
            ir.SetTerm(IR::Term::PopRSBHint{});
        else
            ir.SetTerm(IR::Term::ReturnToDispatch{});
        return false;
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDM(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // LDM <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.GetRegister(n);
        auto writeback_address = ir.Add(start_address, ir.Imm32(u32(Common::BitCount(list) * 4)));
        return LDMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDMDA(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // LDMDA <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Sub(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list) - 4)));
        auto writeback_address = ir.Sub(start_address, ir.Imm32(4));
        return LDMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDMDB(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // LDMDB <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Sub(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list))));
        auto writeback_address = start_address;
        return LDMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDMIB(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // LDMIB <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Add(ir.GetRegister(n), ir.Imm32(4));
        auto writeback_address = ir.Add(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list))));
        return LDMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_LDM_usr() {
    return InterpretThisInstruction();
}

bool ArmTranslatorVisitor::arm_LDM_eret() {
    return InterpretThisInstruction();
}

static bool STMHelper(IR::IREmitter& ir, bool W, Reg n, RegList list, IR::Value start_address, IR::Value writeback_address) {
    auto address = start_address;
    for (size_t i = 0; i <= 14; i++) {
        if (Common::Bit(i, list)) {
            ir.WriteMemory32(address, ir.GetRegister(static_cast<Reg>(i)));
            address = ir.Add(address, ir.Imm32(4));
        }
    }
    if (W) {
        ir.SetRegister(n, writeback_address);
    }
    if (Common::Bit<15>(list)) {
        ir.WriteMemory32(address, ir.Imm32(ir.PC()));
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STM(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // STM <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.GetRegister(n);
        auto writeback_address = ir.Add(start_address, ir.Imm32(u32(Common::BitCount(list) * 4)));
        return STMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STMDA(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // STMDA <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Sub(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list) - 4)));
        auto writeback_address = ir.Sub(start_address, ir.Imm32(4));
        return STMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STMDB(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // STMDB <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Sub(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list))));
        auto writeback_address = start_address;
        return STMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STMIB(Cond cond, bool W, Reg n, RegList list) {
    if (n == Reg::PC || Common::BitCount(list) < 1)
        return UnpredictableInstruction();
    // STMIB <Rn>{!}, <reg_list>
    if (ConditionPassed(cond)) {
        auto start_address = ir.Add(ir.GetRegister(n), ir.Imm32(4));
        auto writeback_address = ir.Add(ir.GetRegister(n), ir.Imm32(u32(4 * Common::BitCount(list))));
        return STMHelper(ir, W, n, list, start_address, writeback_address);
    }
    return true;
}

bool ArmTranslatorVisitor::arm_STM_usr() {
    return InterpretThisInstruction();
}

} // namespace Arm
} // namespace Dynarmic

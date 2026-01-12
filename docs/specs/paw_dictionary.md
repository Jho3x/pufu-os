# Paw Language Dictionary (v1.0)

**Source of Truth**: `z_pufu_4/docs/paw_reference.md`, `z_pufu_1/docs/mvp_plan.md`

## 1. Overview
Paw (Pufu Assembly Wrapper) is the low-level assembly language executed by the Pufu VM. It maps directly to C-structs (`PufuInstruction`).

## 2. Opcodes (Numerical)
Based on `mvp_plan.md`, the VM uses numerical opcodes for speed.

| Opcode | Mnemonic | Arguments           | Description |
| :--- | :--- | :--- | :--- |
| 0x01   | `mov`      | `reg1 val`          | Move value/reg into `reg1`. |
| 0x02   | `add`      | `reg1 val`          | `reg1 += val` |
| 0x03   | `sub`      | `reg1 val`          | `reg1 -= val` |
| 0x04   | `cmp`      | `reg1 val`          | Compare `reg1` vs `val`. Sets internal flags. |
| 0x05   | `jmp`      | `label`             | Unconditional jump. |
| 0x06   | `beq`      | `label`             | Branch if Equal (Flag == 0). |
| 0x07   | `bne`      | `label`             | Branch if Not Equal. |
| 0xFF   | `syscall`  | `(name) arg`        | Execute Kernel Function. |

*Note: `reg` can be `r0`..`r9`.*

## 3. Syscalls
Core Kernel interactions.
- `(write) "string"`: Print to Console.
- `(sleep) ms`: Yield execution.
- `(spawn) "path"`: Launch process.
- `(kill) "name"`: Terminate process.
- `(exit)`: Terminate self.
- `(trinity_set_string) "key value"`: Trinity UI property set.
- `(trinity_create_node) "name type"`: Create UI Node.

## 4. Syntax format
```asm
label_name:
    opcode arg1 arg2
    syscall (name) "arg"
```
The Loader parses this text into binary `PufuInstruction` structs.

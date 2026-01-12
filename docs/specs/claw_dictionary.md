# Claw Language Dictionary (v2.0)

**Source of Truth**: `z_pufu_0/docs/claw_architecture.md`, `z_pufu_0/docs/claw_syntax.md`, `z_pufu_0/power_binary.pufu`

## 1. Overview
Claw (the "Mind") is Pufu's logic layer, based on **Binary Cellular Automata**.
It treats "Code" and "Data" as a single unified Structure of Cells.

## 2. The Double Valve Architecture
Every node is a **Cell** defined by two 4-bit valves (Nibbles):
`ID [VALVE 1: CONTEXT] [VALVE 2: OPERATION] [INPUT 1] [INPUT 2]`

### 2.1 Valve 1: Context (Opcode)
Determines the role of the cell in the system.
| Hex | Mnemonic | System Context Role |
| :--- | :--- | :--- |
| **0** | **ZER** | **Structure/Null**: Defines topology (Brane, Pipe, Storage). |
| **1** | **AND** | **Link**: Reference/Pointer to another cell. |
| **2** | **BIE** | **Alloc**: Reserve memory/scope. |
| **3** | **TIE** | **Free**: Release memory. |
| **5** | **STA** | **Store**: Persist value (Memory/Disk). |
| **6** | **XOR** | **Jump**: Control flow/Loop. |
| **7** | **HEP** | **Label/Fork**: Define name or parallel path. |
| **8** | **OCT** | **Wait**: Sync/Delay. |
| **9** | **JOE** | **Load**: Read from memory. |
| **A** | **RAI** | **Exec**: execute Valve 2 Logic. |
| **B** | **NOD** | **Debug**: Trace/Log. |
| **C** | **DOZ** | **In**: Input (Sensor/Key). |
| **D** | **AXE** | **Out**: Output (Screen/Actuator). |
| **E** | **NAD** | **Signal**: Interrupt/Event. |
| **F** | **ONE** | **Identity**: Existence marker. |

### 2.2 Valve 2: Operation (Logic)
Determines the boolean transformation if `Val1 == RAI` (Exec).
| Hex | Mnemonic | Logic Gate (A, B) |
| :--- | :--- | :--- |
| **0** | **ZER** | **False**: Always 0. |
| **1** | **AND** | **And**: A & B. |
| **2** | **BIE** | **A & !B**. |
| **3** | **PHI** | **A**: Buffer A. |
| **4** | **QUD** | **!A & B**. |
| **5** | **STA** | **B**: Buffer B. |
| **6** | **XOR** | **Xor**: A != B. |
| **7** | **HEP** | **Or**: A \| B. |
| **8** | **OCT** | **Nor**: !(A \| B). |
| **9** | **JOE** | **Xnor**: A == B. |
| **A** | **RAI** | **Not B**: !B. |
| **B** | **NOD** | **A \| !B**. |
| **C** | **DOZ** | **Not A**: !A. |
| **D** | **AXE** | **Impl**: !A \| B. |
| **E** | **NAD** | **Nand**: !(A & B). |
| **F** | **ONE** | **True**: Always 1. |

## 3. Structural Concepts
*   **Brane**: A container defined by a ZER chain (`Name -> Scope -> Logic`).
*   **Pipe (Parallel)**: A ZER structure that enables parallel execution of the stratum above.
*   **Helicoid**: A chain of passive cells attached to a main cell, acting as DNA/Metadata (used for Strings, Files).

## 4. Syntax Example (Multiplier)
```claw
_claw::init
# Logic: p00 = u0 AND l0
# RAI (Exec Logic) + AND (Gate)
p00 RAI AND u0 l0

# Struct: Output 0
# AXE (Output Port) + TIE (Identity/Wire)
out0 AXE TIE z0 0
_claw::end
```

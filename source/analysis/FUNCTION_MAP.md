# Recovered game function map

Addresses are offsets in the 67,113-byte decrypted PKLITE program image. Names
are assigned from live behavior, call relationships, and referenced display
strings; the stripped EXE did not preserve original identifiers.

| Address | Recovered purpose | Confidence |
|---:|---|---|
| `0x0292` | program-level setup and repeated game session | High |
| `0x02BF` | main status/dashboard renderer | High |
| `0x0C68` | random-event selection | Medium |
| `0x0CD5` | debt warning selector | High |
| `0x0D3A` | riot/crowd threshold message | High |
| `0x0DD2` | main command menu renderer | High |
| `0x0F43` | main command dispatcher | Exact |
| `0x1520` | bank deposit/withdraw workflow | Exact |
| `0x16FA` | buy workflow | Exact |
| `0x23F3` | drive/location workflow | Exact |
| `0x3144` | hospital workflow | Exact |
| `0x3238` | loan-shark workflow | Exact |
| `0x3542` | riot entry workflow | Exact |
| `0x3761` | sell workflow | Exact |
| `0x3F99` | schools overview | Exact |
| `0x53A0` | bounded random-number helper | High |
| `0x72B4` | text-color helper | High |
| `0x7433` | formatted text output | High |
| `0x76B8` | single-key input | High |
| `0x770A` | cursor positioning | High |

## Exact main dispatcher

The jump table at `0x0FE0` covers ASCII `b` through `v`:

| Key | Target | Action |
|---|---:|---|
| `B` | `0x0F9C` → `0x16FA` | Buy |
| `C` | `0x0FA4` → `0x3F99` | Schools |
| `D` | `0x0FA9` → `0x23F3` | Drive |
| `H` | `0x0FAE` → `0x3144` | Hospital |
| `L` | `0x0FB3` → `0x3238` | Loan Shark |
| `Q` | `0x0FB8` | Quit |
| `R` | `0x0FC8` → `0x3542` | Riot |
| `S` | `0x0FCD` → `0x3761` | Sell |
| `V` | `0x0FD2` → `0x1520` | Visit Bank |

Every other key in the table routes to the default redraw path at `0x0FD7`.

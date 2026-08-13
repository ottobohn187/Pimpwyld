# Reverse-engineering and behavioral comparison report

## Input identification

- Archive: `pwyld20(1).zip`
- Original executable: `PWYLD20.EXE`, 39,443 bytes
- Executable SHA-256: `594db7ab8fbd531357c6e4f87ccea0d80c738ad8069186975ca99e976ecb8e23`
- Documentation SHA-256: `75f3490b409a166aaf129d636b680dbe9d206af560a9268b23c2c5c913b76859`
- Format: 16-bit DOS MZ executable, PKLITE compressed
- Compiler signature after unpacking: `Borland C++ - Copyright 1991 Borland Intl.`

PKLITE encrypted the literal stream. Decompression with the extra-compression
XOR mode yielded 67,113 bytes of decoded program image and exposed the game's
data and display strings. The packed footer reports original entry point
`0080:1071`.

## Static comparison matrix

| Feature | Evidence in EXE | Reconstructed C | Result |
|---|---|---|---|
| Starting cash | Documentation says `$500` | `$500` | Match |
| Starting bank | Documentation says about half of cash | `$250` | Match |
| Starting weapon | `.22 Liberator` literal and documentation | one `.22 Liberator` | Match |
| Commodities | 10 recovered names | same 10 names | Match |
| Weapons | 10 recovered names | same 10 names | Match |
| Locations | 18 schools plus Administration Building | same 19 names | Match |
| Market actions | BUY/SELL strings and quantity errors | buy/sell with cash and hold checks | Structural match |
| Bank | deposit/withdraw strings and errors | deposit/withdraw with balance checks | Structural match |
| Loan shark | receive/pay strings, debt warnings | receive/pay and debt tracking | Structural match |
| Travel | DRIVE list and destination prompt | location list and travel | Structural match |
| Riot/combat | riot threshold, charge, enemies, win text | riot gate and combat | Approximation |
| Random prices | price display and market logic visible | seeded price variation | Approximation |
| Exact ANSI UI | direct DOS screen/color calls | portable terminal output | Different by design |
| Sexual school scene | present in recovered literals | omitted | Intentionally excluded |

## Dynamic validation performed

The original packed executable was run live in a DOSBox WASM emulator. Its
screen was captured after skipping the intro and after opening the Schools,
Buy, Bank, Hospital, Loan Shark, Drive, and Riot paths. Keystrokes were sent to
the actual DOS process rather than inferred from static strings.

Live validation confirmed:

- the main dispatcher keys are `B C D H L Q R S V`
- starting state is cash 500, bank 250, health 100, max hold 10, lays 0,
  condoms 1, debt 0, status 1, and guns 1
- the starting school and market are randomized
- depositing 100 changes cash 500 to 400 and bank 250 to 350
- schools cost 25 to visit; the Administration Building costs 200
- a live drive to Trinity advanced Day 1 to Day 2 and cash 500 to 475
- the Schools screen's exact control, academics, riot, and security table

The reconstructed program was compiled with strict warnings and its
deterministic self-test passed. The test covers verified initial state,
inventory sale arithmetic, and bank transfers.

A byte-for-byte or frame-for-frame equivalence claim would be false. The EXE has
no symbols, is segmented 16-bit code, and its original source is absent. Exact
equivalence still requires DOS-emulator traces for each branch and further
manual function labeling. This release therefore distinguishes every directly
confirmed feature from reconstructed behavior.

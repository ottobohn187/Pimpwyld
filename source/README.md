# Pimp Wyld 3.0 — Big Ten Edition

This is a playable, expanded 3.0 edition built from the clean-room C
reconstruction of `PWYLD20.EXE`. The original was a PKLITE-packed 16-bit DOS
executable built with Borland C++ 1991. This is readable portable C, not the
original lost source text.

## Build and run

```sh
make
./pwyld20
```

For deterministic play:

```sh
./pwyld20 --seed 1
```

Run automated checks with `make test`.

The native Windows build recreates the original 80-column DOS-style dashboard,
ANSI colors, boxed status panels, two-row command menu, single-key commands,
and lettered school screens. It remains a normal Windows console application;
DOSBox is not required. The launcher uses a 110-column window for the expanded
event artwork while the dashboard itself remains a compact 78 columns.

## Restored gameplay systems

- all ten drugs can be bought and sold, including Crystal, with clear receipts
  and full-hold/insufficient-cash messages
- condoms do not incorrectly consume drug-hold space
- schools at 96% riot or higher develop a 4x-7x panic-price market
- travel can trigger hostile attacks, armed muggings, trenchcoat upgrades, or
  an original-style anarchist offering a bundle of AK-47s
- owned weapon types contribute distinct amounts of power during combat

## Version 3.0 campaign

- all 18 current Big Ten universities replace the old K-12 locations
- 60-day campaign with a final Impact Score and reputation rank
- self-explanatory command labels such as `(B)uy`, `(S)ell`, and `(R)iot`
- real latitude/longitude travel distances scaled to fares from $5 to $25
- high-security campuses have cheaper markets but more dangerous opposition;
  low-security campuses are stronger sell markets
- riot actions organize unrest, increase future random-fight risk, and lead to
  a forward-facing ANSI/JRPG combat screen
- riot combat uses a three-person crew (YOU, ACE, and NYX); every living
  fighter receives a turn and has an individual numeric HP bar
- combat actions include Attack, Gun, Defend, Rally, and Escape; if YOU fall,
  the riot is lost even if another crew member is still standing
- a lay requires and consumes one condom; all companion encounters are adults
- lays add to impact and can trigger a jealous adult boyfriend encounter
- a neon, xterm-256 half-block ANSI opening scene plus detailed alley scenes
  for the AK dealer and random trenchcoat upgrade
- `(T)est` opens a nine-scene, no-consequence event gallery covering the AK
  deal, trenchcoat, market panic, riot buildup and battle, street attack,
  mugging, successful lay, and boyfriend attack; it preserves game state and
  the random-number sequence
- the AK encounter now uses a rainy-alley ANSI scene with the dealer on the
  left and a neon-edged weapon case displaying the offered AK and sidearms

## Fidelity

Recovered directly from the executable:

- starting cash ($500), bank balance ($250), health (100), maximum hold (10),
  one condom, one status point, one gun, and the `.22 Liberator`
- ten commodities and ten weapons
- the original nineteen-location structure, re-themed in 3.0 as 18 Big Ten
  campuses and Big Ten Headquarters
- bank, loan, market, travel, hospital, riot, combat, status, health, hold,
  security, control, academics, and random-mugging concepts
- the main command vocabulary and much of the original display wording
- exact school academics, riot, and security table shown by the live EXE
- the original flat travel prices (replaced by distance fares in 3.0)

Reconstructed rather than proven exact:

- exact endpoints of price ranges and random-event probabilities
- combat arithmetic, loan ceiling, interest, and victory thresholds
- the exact Borland `rand()` sequence and screen coordinates/colors

The original contains sexual material involving school-age characters. That
scene is deliberately not reproduced in this portable reconstruction. It is
identified in the binary evidence report without recreating its content.

The original EXE was also run live in a DOSBox WASM emulator. See
`REVERSE_ENGINEERING.md`, `analysis/FUNCTION_MAP.md`, and `evidence/` for the
hashes, unpacking details, recovered dispatcher, live captures, and comparison
matrix.

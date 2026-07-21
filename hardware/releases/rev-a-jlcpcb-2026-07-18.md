# Rev A JLCPCB Manufacturing Release

Status: **In production as of 2026-07-21.**

This record identifies the first Rev A fabrication and assembly run and the exact local upload artifacts retained with the project.

## Order Record

The JLCPCB order page displayed the following information:

| Item | Recorded value |
| --- | --- |
| Order time | 2026-07-18 00:37:30, as displayed by JLCPCB |
| JLCPCB order | `W2026071807372374` |
| Overall status | In Production |
| PCB product | PCB Prototype |
| PCB order | `Y15-03620135F` |
| PCB quantity | 5 pieces |
| PCB build estimate | 3 days |
| PCB price | $30.98 |
| PCB production file | `Archive_Y15`; confirmed |
| Assembly product | Economic PCBA |
| PCBA order | Displayed beginning `SMT026071763327-362`; suffix not visible in the retained screenshot |
| PCBA quantity | 2 pieces |
| PCBA build estimate | 4 days |
| PCBA price | $84.98 |
| Assembly BOM | `driver-board-jlc-bom.csv` |
| Assembly CPL | `driver-board-jlc-cpl.csv` |
| Merchandise total | $115.96 |
| Shipping | $24.55 |
| Customs duties and taxes | $28.09 |
| Order total | $168.60 |

The order page does not show stackup, copper weight, surface finish, colour, or the other detailed fabrication selections. If those settings are later exported from JLCPCB, add them here without changing the archived upload artifacts.

## Archived Upload Artifacts

| File | SHA-256 |
| --- | --- |
| `driver-board/mfr/Archive.zip` | `2fa4bc9299dedf3daeadd7bcf76799896939166ab728aa14c1a1434a415308a5` |
| `driver-board/mfr/driver-board-jlc-bom.csv` | `09befc26d306caf2595d57af9a16463424a30b2fd342b907b6d6cff0e5063712` |
| `driver-board/mfr/driver-board-jlc-cpl.csv` | `672cb036feffceff9ac96b1f98a63404802aedea3e041ae011d15d0772670f0f` |

`Archive.zip` contains the submitted four-layer Gerbers and plated/non-plated drill files. The BOM and CPL are retained separately because JLCPCB accepts them as separate assembly uploads.

The Git tag `rev-a-jlcpcb-2026-07-18` identifies the repository checkpoint for this manufacturing run.

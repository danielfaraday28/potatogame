### Art briefs for shop assets (single file)

This document specifies every required image in `assets/ui` and `assets/weapons`, with exact output filenames, sizes, palettes, composition notes, and a ready-to-use prompt for an image model. Keep the art consistent with the attached reference (Brotato-like UI): flat, readable, minimal gradients, crisp outlines.

### Global style baseline
- Style: clean, flat UI; minimal or no gradients; no noise. Subtle inner glow allowed but keep it very soft (3–5% opacity).
- Palette (suggested): dark blue-greys for backgrounds; cool light greys for borders; accent green for currency. Use NZ spelling (colour etc.).
- Outline: where applicable, use very dark grey rgb(24,26,34) at 1–2 px.
- Corners: rounded corners on cards, 14 px radius.
- Transparency: PNGs with transparent background outside the intended shape.

---

### UI — card_normal
- Output: assets/ui/card_normal.png
- Canvas: 220×240 px, PNG, transparent background
- Use: default shop card background
- Colours: fill rgb(56,58,72); border rgb(170,174,192) at 1–2 px
- Composition: full-bleed rounded rectangle; no text; keep ~12 px inner safe padding
- Prompt: Create a flat UI card background with rounded corners in a minimalist game UI style. The card is a 220×240 px rounded rectangle with a very dark blue-grey fill (rgb 56,58,72) and a thin cool light-grey border (rgb 170,174,192, 1–2 px). No logos, no text, no patterns. Background outside the shape must be transparent. Export as PNG.
- Checklist: size 220×240; transparent outside; no embedded text/icons.

### UI — card_selected
- Output: assets/ui/card_selected.png
- Canvas: 220×240 px, PNG, transparent
- Use: highlighted/selected card
- Colours: fill rgb(70,74,92); border rgb(200,204,220) at 2 px
- Composition: same geometry as normal; subtle inner glow allowed (<=5% opacity)
- Prompt: Create a selected-state variant of a flat UI card used in a minimalist game shop. 220×240 px rounded rectangle, dark blue-grey fill (rgb 70,74,92) with a brighter cool-grey border (rgb 200,204,220, 2 px). No text or patterns. Transparent background.
- Checklist: size 220×240; transparent; no embedded text.

### UI — card_locked
- Output: assets/ui/card_locked.png
- Canvas: 220×240 px, PNG, transparent
- Use: locked card state
- Colours: fill rgb(96,89,39) (muted olive-grey); border rgb(150,154,170)
- Composition: same as normal; optional faint hatch at 3–4% opacity (very subtle)
- Prompt: Create a locked-state variant of a flat UI card in a minimalist game shop. 220×240 px rounded rectangle with a muted olive-grey fill (rgb 96,89,39) and a thin cool-grey border (rgb 150,154,170). No text or icons. Transparent outside the shape.
- Checklist: size 220×240; transparent; no embedded text/icons.

### UI — coin
- Output: assets/ui/coin.png
- Canvas: 24×24 px, PNG, transparent
- Use: currency indicator
- Colours: base green rgb(96,175,96) or teal rgb(94,168,139); outline rgb(24,26,34)
- Composition: circular token; 1 px safe padding; small flat highlight wedge; no numbers
- Prompt: Create a 24×24 px coin icon in a minimalist game UI style. Round token with green base (rgb ~96,175,96), dark grey outline (rgb 24,26,34), subtle flat highlight. No text. Transparent background.
- Checklist: size 24×24; transparent; readable silhouette with dark outline.

### UI — lock (unlocked)
- Output: assets/ui/lock.png
- Canvas: 24×24 px, PNG, transparent
- Use: lock toggle indicator (unlocked)
- Colours: fill rgb(210,214,226); outline rgb(24,26,34)
- Composition: padlock with open shackle; simple, rounded body; no tiny details
- Prompt: Create a 24×24 px minimalist padlock icon with an open shackle. Light grey fill (rgb ~210,214,226) with a dark outline (rgb 24,26,34). No text or gradients. Transparent background.
- Checklist: size 24×24; transparent; clear "unlocked" silhouette.

### UI — lock_locked (locked)
- Output: assets/ui/lock_locked.png
- Canvas: 24×24 px, PNG, transparent
- Use: lock toggle indicator (locked)
- Colours: same as unlocked; body can be slightly darker to imply active lock
- Composition: closed shackle; simple, bold silhouette
- Prompt: Create a 24×24 px minimalist closed padlock icon. Light grey fill with a dark outline, matching the unlocked icon style. No text. Transparent background.
- Checklist: size 24×24; transparent; matches unlocked style.

### UI — reroll
- Output: assets/ui/reroll.png
- Canvas: 28×28 px, PNG, transparent
- Use: reroll button/icon near price
- Colours: neutral grey fill rgb(170,174,192); outline rgb(24,26,34)
- Composition: circular arrows symbol; compact; 1–2 px outline
- Prompt: Create a 28×28 px minimalist reroll icon with circular arrows. Neutral grey fill (rgb 170,174,192) and dark outline (rgb 24,26,34). No text, clean flat style. Transparent background.
- Checklist: size 28×28; transparent; crisp at small scale.

### Weapon — pistol
- Output: assets/weapons/pistol.png
- Canvas: 48×48 px, PNG, transparent
- Use: weapon icon on card
- Colours: light grey rgb(210,214,226) body; dark outline rgb(24,26,34); minimal internal cut lines
- Composition: simple side-profile pistol, slightly chibi/proportioned for readability; no text
- Prompt: Create a 48×48 px minimalist pistol icon, side profile, light grey fill with a dark outline, readable at small size, flat style, no gradients, transparent background.
- Checklist: size 48×48; transparent; strong silhouette; consistent outline.

### Weapon — smg
- Output: assets/weapons/smg.png
- Canvas: 48×48 px, PNG, transparent
- Use: weapon icon on card
- Colours: same as pistol; slight visual distinction (longer magazine/stock)
- Composition: simple side-profile SMG, chibi-friendly proportions, no tiny detail
- Prompt: Create a 48×48 px minimalist SMG icon, side profile, light grey fill with a dark outline. Keep shapes bold and readable; flat style; transparent background.
- Checklist: size 48×48; transparent; matches pistol style.
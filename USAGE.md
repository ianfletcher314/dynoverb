# Dynoverb - Usage Guide

**Dynamic Reverb Plugin**

Dynoverb is a versatile reverb plugin featuring four distinct reverb algorithms: Algorithmic (Room/Hall/Plate/Chamber), Shimmer, Spring, and Gated. With comprehensive controls including pre-delay, damping, modulation, and built-in ducking, it's designed for everything from subtle ambience to dramatic effects.

---

## Use Cases in Modern Rock Production

### Drum Bus Processing

Reverb on drums adds space, depth, and character.

**Room Reverb for Drums:**
- Type: Algorithmic
- Mode: Room
- Pre-Delay: 10-20 ms
- Decay: 0.8-1.2 s
- Size: 40-50%
- Damping: 50%
- Diffusion: 70%
- Early Reflections: 60%
- Width: 100%
- High Pass: 200 Hz
- Low Pass: 8000 Hz
- Mix: 15-25%

**80s Gated Snare:**
- Type: Gated
- Pre-Delay: 0-10 ms
- Decay: 1.5-2.0 s
- Gate Threshold: -20 dB
- Gate Hold: 150-200 ms
- Gate Release: 50-100 ms
- Gate Shape: 70-80%
- Size: 60%
- Mix: 40-60%

**Big Hall Drums:**
- Type: Algorithmic
- Mode: Hall
- Pre-Delay: 30-50 ms
- Decay: 2.5-4.0 s
- Size: 70%
- Damping: 60%
- Diffusion: 80%
- Width: 120%
- Mix: 20-30%
- Ducking: 30% (keeps punch)

### Guitar Bus / Individual Tracks

Reverb adds dimension to guitars without washing them out.

**Ambient Clean Guitar:**
- Type: Shimmer
- Pitch: Octave Up
- Shimmer Amount: 40%
- Pre-Delay: 50-80 ms
- Decay: 3-5 s
- Size: 60%
- Damping: 40%
- Width: 150%
- Mix: 30-50%

**Classic Spring Guitar:**
- Type: Spring
- Tension: 50%
- Drip: 40%
- Spring Mix: 70%
- Pre-Delay: 0-10 ms
- Decay: 1.5-2.0 s
- Size: 40%
- Width: 80%
- Mix: 25-35%

**Lead Guitar Plate:**
- Type: Algorithmic
- Mode: Plate
- Pre-Delay: 40-60 ms
- Decay: 1.8-2.5 s
- Size: 50%
- Damping: 45%
- Diffusion: 85%
- Mod Rate: 0.5 Hz
- Mod Depth: 20%
- Mix: 20-30%

**Rhythm Guitar Room:**
- Type: Algorithmic
- Mode: Room
- Pre-Delay: 20 ms
- Decay: 0.6-1.0 s
- Size: 35%
- High Pass: 150 Hz
- Low Pass: 6000 Hz
- Mix: 10-20%
- Ducking: 40%

### Bass Guitar

Bass typically needs minimal reverb, but subtle room can add life.

**Subtle Bass Room:**
- Type: Algorithmic
- Mode: Room
- Pre-Delay: 10 ms
- Decay: 0.4-0.6 s
- Size: 30%
- High Pass: 200 Hz
- Low Pass: 3000 Hz
- Mix: 5-10%
- Ducking: 50%

### Vocals

Vocals benefit from carefully chosen reverb to add depth without losing clarity.

**Lead Vocal Plate:**
- Type: Algorithmic
- Mode: Plate
- Pre-Delay: 50-80 ms
- Decay: 1.5-2.5 s
- Size: 50%
- Damping: 50%
- Diffusion: 80%
- High Pass: 200 Hz
- Low Pass: 8000 Hz
- Width: 100%
- Mix: 15-25%
- Ducking: 30%

**Vocal Hall:**
- Type: Algorithmic
- Mode: Hall
- Pre-Delay: 60-100 ms
- Decay: 2.5-4.0 s
- Size: 60%
- Damping: 55%
- Early Reflections: 40%
- Width: 120%
- Mix: 20-30%
- Ducking: 40%

**Ethereal Vocal Shimmer:**
- Type: Shimmer
- Pitch: Mixed
- Shimmer Amount: 50%
- Pre-Delay: 80 ms
- Decay: 4-8 s
- Size: 70%
- Damping: 35%
- Width: 180%
- Mix: 25-40%

**Backing Vocal Chamber:**
- Type: Algorithmic
- Mode: Chamber
- Pre-Delay: 30 ms
- Decay: 1.2-1.8 s
- Size: 45%
- Width: 150%
- Mix: 25-35%

### Mix Bus / Mastering

Reverb on the mix bus should be extremely subtle if used at all.

**Mix Bus Glue Reverb (subtle):**
- Type: Algorithmic
- Mode: Room or Chamber
- Pre-Delay: 20 ms
- Decay: 0.4-0.8 s
- Size: 30%
- High Pass: 300 Hz
- Low Pass: 6000 Hz
- Mix: 3-8%
- Ducking: 50%

---

## Recommended Settings

### Quick Reference Table

| Application | Type | Mode/Settings | Decay | Pre-Delay | Mix |
|------------|------|---------------|-------|-----------|-----|
| Drum Room | Algorithmic | Room | 0.8-1.2s | 15 ms | 20% |
| Gated Snare | Gated | Hold 180ms | 1.8s | 5 ms | 50% |
| Clean Guitar | Shimmer | Octave Up 40% | 4s | 60 ms | 35% |
| Spring Guitar | Spring | Tension 50% | 1.8s | 5 ms | 30% |
| Lead Vocal | Algorithmic | Plate | 2s | 60 ms | 20% |
| Backing Vocal | Algorithmic | Chamber | 1.5s | 30 ms | 30% |
| Ambient | Shimmer | Mixed 60% | 6s | 80 ms | 40% |

### Algorithm Mode Guide

**Algorithmic:**
- **Room**: Small, tight reflections - drums, guitars, natural sound
- **Hall**: Large, spacious - vocals, orchestral, dramatic
- **Plate**: Dense, smooth - vocals, snare, classic studio sound
- **Chamber**: Medium space, character - backing vocals, ensemble

**Shimmer:**
- **Octave Up**: Ethereal, angelic - ambient guitar, pads, special effects
- **Fifth Up**: Less extreme, musical shimmer
- **Octave Down**: Darker, mysterious
- **Fifth Down**: Subtle darkness
- **Mixed**: Combination - complex, evolving textures

**Spring:**
- Classic guitar amp reverb character
- **Tension**: Higher = tighter, more metallic
- **Drip**: The characteristic spring "boing"

**Gated:**
- 80s-style gated reverb
- **Hold**: How long reverb sustains before cut
- **Release**: How fast the gate closes
- **Shape**: Steepness of the gate curve

---

## Signal Flow Tips

### Where to Place Dynoverb

1. **Send/Return (Aux)**: Most flexible - blend reverb with dry signal
2. **Insert (100% wet)**: For parallel processing setups
3. **Insert (with Mix)**: Convenient for single-track reverb

### Pre-Delay Guidelines

- **0-20 ms**: Tight, connected to source - drums, aggressive rock
- **30-50 ms**: Separated but present - most applications
- **60-100 ms**: Clearly separate - vocals, leads
- **100+ ms**: Dramatic separation - special effects

### Using Ducking

Ducking reduces reverb level when input signal is present:
- **0%**: No ducking - reverb always at full level
- **20-40%**: Subtle - reverb ducks gently, maintains clarity
- **50-70%**: Moderate - reverb clearly ducks, keeps source upfront
- **80-100%**: Aggressive - reverb only heard in gaps

### Using Freeze

- **Freeze**: Captures and sustains current reverb tail indefinitely
- Use for drones, transitions, or creative effects
- Combine with Shimmer for evolving pads

---

## Combining with Other Plugins

### Vocal Reverb Chain
1. **VoxProc** - process vocal
2. **Send to Dynoverb** - add space
3. **Ducker** on reverb return (optional) - duck reverb with dry vocal

### Drum Reverb Setup
1. **Bus Glue** on drum bus
2. **Send snare/kit to Dynoverb** - room/hall
3. **Separate Dynoverb** for gated snare effect

### Guitar Ambient Chain
1. **PDLBRD** - basic tone
2. **TapeWarm** - analog warmth
3. **Dynoverb** (Shimmer) - ambient space

### Reverb Blending
- Use multiple Dynoverb instances with different settings
- Short room + long hall = depth + space
- Plate + shimmer = classic + ethereal

---

## Quick Start Guide

**Add plate reverb to vocals in 30 seconds:**

1. Create a send from your vocal to an aux/bus
2. Insert Dynoverb on the aux (or use insert with Mix control)
3. Set **Type** to Algorithmic
4. Set **Mode** to Plate
5. Set **Pre-Delay** to 60 ms
6. Set **Decay** to 2.0 s
7. Set **Size** to 50%
8. Set **Damping** to 50%
9. Set **High Pass** to 200 Hz
10. Set **Mix** to 100% (if on send) or 20% (if on insert)
11. Adjust send level to taste

**Create gated snare reverb in 30 seconds:**

1. Create a send from your snare to an aux/bus
2. Insert Dynoverb on the aux
3. Set **Type** to Gated
4. Set **Pre-Delay** to 5 ms
5. Set **Decay** to 1.8 s
6. Set **Size** to 60%
7. Set **Gate Threshold** to -20 dB
8. Set **Gate Hold** to 180 ms
9. Set **Gate Release** to 80 ms
10. Set **Gate Shape** to 75%
11. Blend to taste - typically 30-50% of dry level

**Create shimmer ambience in 30 seconds:**

1. Insert Dynoverb on your guitar or pad track
2. Set **Type** to Shimmer
3. Set **Pitch** to Octave Up or Mixed
4. Set **Shimmer Amount** to 50%
5. Set **Pre-Delay** to 80 ms
6. Set **Decay** to 5 s
7. Set **Size** to 65%
8. Set **Damping** to 35%
9. Set **Width** to 150%
10. Set **Mix** to 35%
11. Enable **Ducking** at 30% to keep source clear
12. Play sustained notes and enjoy the wash

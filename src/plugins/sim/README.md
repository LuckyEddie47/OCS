# OCS bench test simulator

Runs OCS on a bare Teensy 4.1 with no sensors, switches or network attached, so the website and
command set can be tried on the bench. It is compiled in only when `SIMULATE` is `ON`, which
`build_sim.sh` does for you without touching your `Config.h`.

## What is simulated

| Thing | Behaviour |
|---|---|
| Roof limit switches | Follow the roof motor relays. End to end travel takes 15 s (`:ZRn#`) |
| Roof interlock (mount parked) | Parked at start. Turning on the mount park relay parks the mount 5 s later |
| Mains sense | On (`:ZPn#`) |
| Thermostat sensor | Inside temperature/humidity drift toward outside, heat/cool/dehumidifier relays push them |
| Weather sensors | Fixed safe values: 8 °C, sky −20 °C, wind ~6 kph, dry, 1012 mb, until changed with commands |
| Network | None. The website is served over the Teensy's third USB serial port and `ocs_sim.py serve` turns it into `http://localhost:8080` |
| Clock | `TIME_LOCATION_SOURCE TEENSY`, set from the PC's UTC time when `ocs_sim.py serve` starts |

The dome is not simulated: with no encoder it never moves, but its controls and target fields still work.

## Use

```bash
# build and flash (USB type Triple Serial), using your own config
src/plugins/sim/build_sim.sh --config Config.h --upload

# serve the website at http://localhost:8080 (leave it running)
src/plugins/sim/ocs_sim.py serve

# send commands on the first USB serial port
src/plugins/sim/ocs_sim.py cmd ':Z?#' ':GRN#'
```

The three USB serial ports are: 1 the normal command channel, 2 command channel `SERIAL_D` (as the
OCS4 pinmap defines it in this USB mode), 3 HTTP for `ocs_sim.py serve`.

## Simulator commands

| Command | Does |
|---|---|
| `:ZSn,s#` | Force sense n (1..8) on (`s=1`) or off (`s=0`), `s=A` hands it back to the simulator/pin |
| `:ZIx,v#` | Inside value: `T` temperature °C, `H` humidity %, `P` pressure mb |
| `:ZOx,v#` | Outside value: `T` temperature °C, `S` sky temperature °C, `W` wind kph, `P` pressure mb, `H` humidity %, `R` rain (1 wet, 2 warn, 3 dry), `Q` sky quality |
| `:ZMn#` | Mount parked (1) or unparked (0) |
| `:ZPn#` | Mains power ok (1) or failed (0) |
| `:ZRn#` | Roof travel time, n seconds (2..600) |
| `:ZGn#` | Put the roof at n % open (0..100), e.g. stopped part way |
| `:Z?#` | Status: roof position, direction, mount, inside T and H |

Examples: `:ZOW,40#` makes it windy (unsafe), `:ZOR,1#` makes it rain, `:ZS2,1#` with the roof
closed gives both limit switches on (a sensor fault).

## Things to know

- NV (settings, relay memory) is cached. For about the first minute after start up the cache is
  still being loaded, and a change can take up to ~45 s to reach flash. Restarting sooner loses it.
- A power device that shares its relay with `HEAT_RELAY`, `COOL_RELAY` or `HUMIDITY_RELAY` is also
  switched by the thermostat, at start up and every 5 minutes, which overrides the power panel and relay memory.
- Serial debug output (`DEBUG`) goes to the first USB port together with command replies.

#!/usr/bin/env python3
"""Bench test helper for an OCS built with SIMULATE ON (see README.md).

  ocs_sim.py serve            serve the OCS website at http://localhost:8080
  ocs_sim.py cmd ':Z?#' ...   send commands to OCS and print the replies

A SIMULATE build uses Teensy USB type "Triple Serial". The first port is the normal
OCS command channel (the second is command channel D), the third carries HTTP: this
script forwards each browser request to it, one at a time, and relays the reply
(which ends with a 0x04 byte).
"""

import argparse
import datetime
import glob
import socketserver
import sys
import termios
import threading
import time

import serial

END_OF_RESPONSE = b"\x04"
RESPONSE_TIMEOUT = 15.0


def teensy_port(interface):
    """Finds the Teensy serial port by USB interface: if00 is commands, if04 is HTTP."""
    ports = sorted(glob.glob(f"/dev/serial/by-id/usb-Teensyduino*-{interface}"))
    if not ports:
        sys.exit(f"No Teensy serial port with interface {interface} found, is the board "
                 "connected and built with USB type Triple Serial? Use --port to choose one.")
    return ports[0]


class Bridge(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True

    def __init__(self, address, port):
        super().__init__(address, BridgeHandler)
        self.port = port
        self.serial = None
        self.lock = threading.Lock()

    def connection(self):
        """The HTTP serial port, (re)opened as needed since the board drops off USB when it restarts."""
        if self.serial is None:
            self.serial = serial.Serial(self.port, 115200, timeout=0.2)
        return self.serial

    def disconnect(self):
        if self.serial is not None:
            try:
                self.serial.close()
            except OSError:
                pass
        self.serial = None


class BridgeHandler(socketserver.StreamRequestHandler):
    def handle(self):
        request_line = self.rfile.readline(2048)
        if not request_line:
            return
        # read and drop the rest of the browser's headers, OCS only needs the request line
        while True:
            line = self.rfile.readline(4096)
            if line in (b"\r\n", b"\n", b""):
                break

        with self.server.lock:
            for attempt in (1, 2):
                try:
                    self.forward(request_line)
                    break
                except (OSError, termios.error, serial.SerialException) as e:
                    # the port goes stale when the board restarts, reopen it and try once more
                    self.server.disconnect()
                    if attempt == 2:
                        print(f"OCS not reachable ({e}), is it restarting?", file=sys.stderr)
                        self.wfile.write(b"HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\n\r\n")
                        return

        print(request_line.decode(errors="replace").strip(), file=sys.stderr)

    def forward(self, request_line):
        port = self.server.connection()
        port.reset_input_buffer()
        port.write(request_line.rstrip(b"\r\n") + b"\r\n\r\n")
        port.flush()

        deadline = time.monotonic() + RESPONSE_TIMEOUT
        while time.monotonic() < deadline:
            chunk = port.read(port.in_waiting or 1)
            if not chunk:
                continue
            end = chunk.find(END_OF_RESPONSE)
            if end >= 0:
                self.wfile.write(chunk[:end])
                return
            self.wfile.write(chunk)
        print(f"timeout waiting for reply to {request_line!r}", file=sys.stderr)


def serve(args):
    port = args.port or teensy_port("if04")
    host, _, listen_port = args.listen.rpartition(":")

    # a bench board has no network time, so give it this computer's (UTC) clock
    utc = datetime.datetime.now(datetime.timezone.utc).strftime("%m/%d/%Y,%H:%M:%S")
    with serial.Serial(args.command_port or teensy_port("if00"), 9600, timeout=0.5) as ser:
        print(f"set OCS clock to {utc} UTC: {send_command(ser, f':SU{utc}#') or 'no reply'}")

    with Bridge((host or "127.0.0.1", int(listen_port)), port) as bridge:
        print(f"OCS website (via {port}) at http://{host or '127.0.0.1'}:{listen_port}/ , Ctrl+C to stop")
        try:
            bridge.serve_forever()
        except KeyboardInterrupt:
            pass


def send_command(ser, command):
    ser.reset_input_buffer()
    ser.write(command.encode())
    ser.flush()
    reply = b""
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        chunk = ser.read(ser.in_waiting or 1)
        if chunk:
            reply += chunk
            if reply.endswith(b"#"):
                break
        elif reply:
            break  # numeric replies (0 or 1) have no terminating '#'
    return reply.decode(errors="replace")


def send_commands(args):
    port = args.port or teensy_port("if00")
    with serial.Serial(port, 9600, timeout=0.5) as ser:
        for command in args.commands:
            print(f"{command} -> {send_command(ser, command)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="action", required=True)

    s = sub.add_parser("serve", help="serve the OCS website on this computer")
    s.add_argument("--port", help="HTTP serial port (default: the Teensy's third serial port)")
    s.add_argument("--command-port", help="command serial port used to set the clock (default: the Teensy's first serial port)")
    s.add_argument("--listen", default="127.0.0.1:8080", help="address:port to listen on (default 127.0.0.1:8080)")
    s.set_defaults(func=serve)

    c = sub.add_parser("cmd", help="send OCS commands, e.g. ':ZM0#' ':GRN#'")
    c.add_argument("--port", help="command serial port (default: the Teensy's first serial port)")
    c.add_argument("commands", nargs="+")
    c.set_defaults(func=send_commands)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()

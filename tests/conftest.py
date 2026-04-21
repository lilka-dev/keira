"""
pytest configuration for Keira UART emulator tests.

Usage:
    pytest tests/ --port /dev/ttyACM0
    pytest tests/ --port /dev/ttyUSB0 --baud 115200 --emu-timeout 15
"""

import pytest
from uart_emulator import KeiraEmulator


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--port",
        default="/dev/ttyACM0",
        help="Serial port connected to the Keira device (default: /dev/ttyACM0)",
    )
    parser.addoption(
        "--baud",
        type=int,
        default=115200,
        help="Serial baud rate (default: 115200)",
    )
    parser.addoption(
        "--emu-timeout",
        type=float,
        default=10.0,
        help="Seconds to wait for device responses (default: 10)",
    )


@pytest.fixture(scope="session")
def keira(request: pytest.FixtureRequest) -> KeiraEmulator:
    """Session-scoped fixture: one serial connection for all tests."""
    port = request.config.getoption("--port")
    baud = request.config.getoption("--baud")
    timeout = request.config.getoption("--emu-timeout")
    emu = KeiraEmulator(port=port, baud=baud, timeout=timeout)
    emu.connect()
    yield emu
    emu.close()

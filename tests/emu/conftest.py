import os
import sys

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools", "emu"))
from keira_emu import KeiraEmu  # noqa: E402

IMAGE = os.environ.get("KEIRA_QEMU_IMAGE", "build-lilka_v2_qemu-docker/flash_image.bin")
QEMU = os.environ.get("QEMU", "qemu-system-xtensa")
# Screenshots of every test end up here, CI uploads them as an artifact
OUT_DIR = os.environ.get("KEIRA_EMU_OUT", "test_output/emu")


@pytest.fixture
def emu(request):
    os.makedirs(OUT_DIR, exist_ok=True)
    name = request.node.name
    with open(os.path.join(OUT_DIR, f"{name}.log"), "w") as log, KeiraEmu(IMAGE, QEMU, log) as emu:
        yield emu
        emu.screenshot(os.path.join(OUT_DIR, f"{name}_last.png"))

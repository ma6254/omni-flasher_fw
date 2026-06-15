# SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: CC0-1.0

from typing import Callable

import pytest
from pytest_embedded_idf.dut import IdfDut


@pytest.fixture
def log_minimum_free_heap_size(dut: IdfDut) -> Callable[..., None]:
    def inner() -> None:
        dut.expect(r'Minimum free heap size: \d+ bytes')

    return inner

@pytest.fixture
def log_build_time(dut: IdfDut) -> Callable[..., None]:
    def inner() -> None:
        dut.expect(r'Build time: \S+ \S+')

    return inner

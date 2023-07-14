import math
import libcasm.container as container

def test_TOL():
    assert math.isclose(container.TOL, 1e-5)

def test_KB():
    assert math.isclose(container.KB, 8.6173423E-05) # eV/K

def test_PLANK():
    assert math.isclose(container.PLANCK, 4.135667516E-15) # eV-s

def test_add():
    assert math.isclose(container.add(1.0, 2.0), 3.0)

"""CASM global constants and definitions"""

from ._casmglobal import KB as _KB
from ._casmglobal import PLANCK as _PLANCK
from ._casmglobal import TOL as _TOL
from ._casmglobal import libcasm_global_version

TOL = _TOL
"""Default CASM tolerance"""


KB = _KB
"""Boltzmann Constant

`From CODATA 2014 <https://arxiv.org/pdf/1507.07956.pdf>`_
"""


PLANCK = _PLANCK
"""Planck Constant

`From CODATA 2014 <https://arxiv.org/pdf/1507.07956.pdf>`_
"""

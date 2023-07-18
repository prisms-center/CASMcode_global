"""CASM global constants and definitions"""

from ._casmglobal import TOL as _TOL
TOL = _TOL
"""Default CASM tolerance"""

from ._casmglobal import KB as _KB
KB = _KB
"""Boltzmann Constant

`From CODATA 2014 <https://arxiv.org/pdf/1507.07956.pdf>`_
"""

from ._casmglobal import PLANCK as _PLANCK
PLANCK = _PLANCK
"""Planck Constant

`From CODATA 2014 <https://arxiv.org/pdf/1507.07956.pdf>`_
"""

from ._casmglobal import libcasm_global_version

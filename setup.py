from setuptools import setup
from distutils.core import setup, Extension
from Cython.Build import cythonize


import sys

extensions = [
    Extension(
        name="uWebSockets",
        sources=["python/*.pyx"], # cython sources
        include_dirs=["include", "external/uWebSockets/src"],
        # libraries=["uwserver"], # this is the c++ library name from CMakeLists.txt
        # library_dirs=["cmake-build-debug"],
        extra_compile_args=['-std=c++11'],
        language="c++"
    )

]


setup(
    author="Sherlock",
    name="uWebSockets",  # name of package in 'pip freeze'
    # package_dir = {} # leave this empty to install to dist-packages
    ext_modules=cythonize(extensions)
)
from distutils.core import setup, Extension
from Cython.Build import cythonize



extensions = [
    Extension(
        name="uWebSockets",
        # there has to be a better way to get all uWebSocket sources...
        sources=['python/*.pyx',
                 'external/uWebSockets/src/Epoll.cpp',
                 'external/uWebSockets/src/Extensions.cpp',
                 'external/uWebSockets/src/Group.cpp',
                 'external/uWebSockets/src/HTTPSocket.cpp',
                 'external/uWebSockets/src/Hub.cpp',
                 'external/uWebSockets/src/Networking.cpp',
                 'external/uWebSockets/src/Node.cpp',
                 'external/uWebSockets/src/Room.cpp',
                 'external/uWebSockets/src/Socket.cpp',
                 'external/uWebSockets/src/WebSocket.cpp'],
        include_dirs=["include", "external/uWebSockets/src"],
        # these are the c++ libraries linked in CMakeLists.txt:
        libraries=["crypto", "z", "uv", "ssl", "pthread"],
        extra_compile_args=['-std=c++11'],
        language="c++"
    )

]


setup(
    author="E.Nuger",
    version="0.3",
    description="Python wrap of an implementation of the  uWebSockets v0.14 server",
    url="https://github.com/evgenyslab/webSockets",
    name="uWebSockets",  # name of package in 'pip freeze'
    # package_dir = {} # leave this empty to install to dist-packages
    ext_modules=cythonize(extensions)
)
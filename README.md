# Websockets Server Class

*Project is split from [here](https://github.com/evgenyslab/Sockets/tree/v0.0.0)*

## Prereqs

```bash
sudo apt-get install \
    zlib1g-dev \
    libssl-dev \
    libuv-dev 

```

## Install

```bash
git clone https://github.com/evgenyslab/webSockets.git
cd webSockets
git checkout dev
git submodule update --init --recursive
python3 setup.py install
cd ..
rm -rf webSockets

```

## TODO List:

- [ ] client c++ cleanup
- [ ] server c++ cleanup
- [ ] client python wrap
- [ ] c++ tests
- [ ] python tests
- [ ] example cleanup
- [ ] why does python call to readNonBlocking block? something about mutex?

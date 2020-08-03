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

- [ ] can a call back be registered to read?
- [ ] implement syslog
- [ ] how to implement clean close within uws app? seems like `h.getDefaultGroup<uWS::SERVER>().close();` is 
called within an `onMessage` call back under specific conditions...
- [ ] client c++ cleanup
- [ ] server c++ cleanup
- [ ] c++ tests
- [ ] python tests
- [ ] example cleanup
# import libcpp plug for string:
from libcpp.string cimport string
from libcpp cimport bool

# mirror class header declarations here
cdef extern from "uWServer.h":
    cdef cppclass uWServer:
        uWServer(int) except +

        void config();
        void run();
        void stop();

        void ping();

        void sendStringAsBinary(string);
        void sendStringAsText(string);

        string readBlocking();
        string readNonBlocking();

        bool isConnected();
        bool hasMessages();

cdef extern from "uWClient.h":
    cdef cppclass uWClient:
        uWClient(int) except +

        void config();
        void run();
        void stop();

        void ping();

        void sendStringAsBinary(string);
        void sendStringAsText(string);

        string readBlocking();
        string readNonBlocking();

        bool isConnected();
        bool hasMessages();


cdef class Server:
    # reference c-class
    cdef uWServer *thisptr
    # create python interfaces for each C++ function member.
    # CAN add python processing here if need be!
    def __cinit__(self, port=8890):
        self.thisptr = new uWServer(port)
    def __dealloc__(self):
        del self.thisptr
    def config(self):
        self.thisptr.config()
    def run(self):
        self.thisptr.run()
    def stop(self):
        self.thisptr.stop()
    def ping(self):
        self.thisptr.ping()
    def sendStringAsBinary(self, msg):
        self.thisptr.sendStringAsBinary(msg)
    def readBlocking(self):
        return self.thisptr.readBlocking()
    def readNonBlocking(self):
        return self.thisptr.readNonBlocking()
    def isConnected(self):
        return self.thisptr.isConnected()
    def hasMessages(self):
        return self.thisptr.hasMessages()

cdef class Client:
    # reference c-class
    cdef uWClient *thisptr
    # create python interfaces for each C++ function member.
    # CAN add python processing here if need be!
    def __cinit__(self, port=8890):
        self.thisptr = new uWClient(port)
    def __dealloc__(self):
        del self.thisptr
    def config(self):
        self.thisptr.config()
    def run(self):
        self.thisptr.run()
    def stop(self):
        self.thisptr.stop()
    def ping(self):
        self.thisptr.ping()
    def sendStringAsBinary(self, msg):
        self.thisptr.sendStringAsBinary(msg)
    def readBlocking(self):
        return self.thisptr.readBlocking()
    def readNonBlocking(self):
        return self.thisptr.readNonBlocking()
    def isConnected(self):
        return self.thisptr.isConnected()
    def hasMessages(self):
        return self.thisptr.hasMessages()
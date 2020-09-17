# import libcpp plug for string:
from libcpp.string cimport string
from libcpp cimport bool

# mirror class header declarations here
cdef extern from "uWServer.h":
    cdef cppclass uWServer:
        uWServer(int, int) except +

        void config();
        bool run();
        void stop();
        void setPort(int);

        void ping();

        void sendStringAsBinary(string);
        void sendStringAsText(string);

        string readBlocking();
        string readNonBlocking();
        string readLastNonBlocking();

        bool isConnected();
        bool hasMessages();

cdef extern from "uWClient.h":
    cdef cppclass uWClient:
        uWClient(int, int) except +

        void config();
        bool run();
        void stop();
        void setPort(int);

        void ping();

        void sendStringAsBinary(string);
        void sendStringAsText(string);

        string readBlocking();
        string readNonBlocking();
        string readLastNonBlocking();

        bool isConnected();
        bool hasMessages();


cdef class Server:
    # reference c-class
    cdef uWServer *thisptr
    # create python interfaces for each C++ function member.
    # CAN add python processing here if need be!
    def __cinit__(self, port=8890, messageQueueLength=10):
        self.thisptr = new uWServer(port, messageQueueLength)
    def __dealloc__(self):
        del self.thisptr
    def config(self):
        self.thisptr.config()
    def run(self):
        status = self.thisptr.run()
        if not status:
            print("Server Error: Socket in use!")
        return status
    def stop(self):
        self.thisptr.stop()
    def ping(self):
        self.thisptr.ping()
    def setPort(self, port):
        self.thisptr.setPort(port)
    def sendStringAsBinary(self, msg):
        self.thisptr.sendStringAsBinary(msg)
    def readBlocking(self):
        return self.thisptr.readBlocking()
    def readNonBlocking(self):
        return self.thisptr.readNonBlocking()
    def readLastNonBlocking(self):
        return self.thisptr.readLastNonBlocking()
    def isConnected(self):
        return self.thisptr.isConnected()
    def hasMessages(self):
        return self.thisptr.hasMessages()

cdef class Client:
    # reference c-class
    cdef uWClient *thisptr
    # create python interfaces for each C++ function member.
    # CAN add python processing here if need be!
    def __cinit__(self, port=8890, messageQueueLength=10):
        self.thisptr = new uWClient(port, messageQueueLength)
    def __dealloc__(self):
        del self.thisptr
    def config(self):
        self.thisptr.config()
    def run(self):
        status = self.thisptr.run()
        if not status:
            print("Error in connecting on socket")
        return status
    def stop(self):
        self.thisptr.stop()
    def ping(self):
        self.thisptr.ping()
    def setPort(self, port):
        self.thisptr.setPort(port)
    def sendStringAsBinary(self, msg):
        self.thisptr.sendStringAsBinary(msg)
    def readBlocking(self):
        return self.thisptr.readBlocking()
    def readNonBlocking(self):
        return self.thisptr.readNonBlocking()
    def readLastNonBlocking(self):
        return self.thisptr.readLastNonBlocking()
    def isConnected(self):
        return self.thisptr.isConnected()
    def hasMessages(self):
        return self.thisptr.hasMessages()
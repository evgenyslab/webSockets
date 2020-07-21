# import libcpp plug for string:
from libcpp.string cimport string

# mirror class header declarations here
cdef extern from "uWServer.h":
    cdef cppclass uWServer:
        uWServer(int) except +

        void config();
        void run();
        void stop();

        void sendStringAsBinary(string);
        void sendStringAsText(string);

        string readBlocking();
        string readNonBlocking();


# define python class that will be called thorugh import myClass.PymyClass:
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
    def sendStringAsBinary(self, msg):
        self.thisptr.sendStringAsBinary(msg)
    def sendStringAsText(self, msg):
        self.thisptr.sendStringAsText(msg)
    def readBlocking(self):
        return self.thisptr.readBlocking()
    def readNonBlocking(self):
        return self.thisptr.readBlocking()
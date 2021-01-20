CPROVDIR=.
CXXFLAGS=-ggdb -std=c++11 -fstack-protector -DUNIX -DOS_UNIX -I$(CPROVDIR)/Include -fPIC
LDFLAGS=-L$(CPROVDIR)/Libraries -lpthread -ldl -luuid -lcrypto -lssl
OUTPUT=InitializerTool

all: $(OUTPUT)

InitializerTool: 64BitHashes.o Base64Encoder.o BaseException.o Chronometer.o CommandLine.o ConsoleInputHelperFunctions.o FifoBuffer.o Guid.o HardCodedCryptographicKeys.o InitializerData.o IpcTransactionHelperFunctions.o Main.o Object.o SmartMemoryAllocator.o SocketClient.o Socket.o StructuredBuffer.o StructuredBufferElement.o TerminationSignal.o TlsClient.o TlsNode.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	-rm -rf $(OUTPUT) *.o
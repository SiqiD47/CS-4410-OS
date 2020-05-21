CFLAGS = -lpthread -I rpc
OBJECTS = fs_client.cc fs_server.cc inode_layer.cc

all: $(OBJECTS)
	$(CXX) $(OBJECTS) part1_tester.cc $(CFLAGS) -o part1_tester
	$(CXX) $(OBJECTS) part2_tester.cc $(CFLAGS) -o part2_tester
	$(CXX) $(OBJECTS) part3_tester.cc $(CFLAGS) -o part3_tester

clean:
	rm -f part1_tester part2_tester part3_tester

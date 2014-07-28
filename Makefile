CXXFLAGS=-I/opt/local/include
OBJS=main.o statement.o #expression.o
all: glad

#%.o : src/%.cc
%.o : src/%.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

statement.o: src/statement.cc src/statement_def.h src/statement.h src/expression.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

main.o: src/main.cpp src/ast.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

glad: $(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm -f *.o

CXXFLAGS=-I/opt/local/include
OBJS=main.o statement.o parser.o #expression.o
all: glad tesla_glad

#%.o : src/%.cc
%.o : src/%.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

statement.o: src/statement.cc src/statement_def.h src/statement.h src/expression.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

main.o: src/main.cpp src/ast.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

glad: $(OBJS)
	$(CXX)  -L/opt/local/lib -o $@ $(OBJS) -lboost_system-mt -lboost_filesystem-mt

tesla_glad: $(OBJS) tesla_glad.o
	$(CXX) -L/opt/local/lib -o $@ statement.o parser.o tesla_glad.o -lboost_system-mt -lboost_filesystem-mt

clean:
	rm -f *.o
